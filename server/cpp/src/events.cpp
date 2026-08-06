#include "events.hpp"

#include <cerrno>
#include <cstdio>
#include <ctime>
#include <poll.h>
#include <sstream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace http {

namespace {

std::string json_escape(const std::string& in) {
    std::ostringstream out;
    for (char c : in) {
        switch (c) {
            case '"':  out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\n': out << "\\n";  break;
            case '\r': out << "\\r";  break;
            case '\t': out << "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out << buf;
                } else {
                    out << c;
                }
        }
    }
    return out.str();
}

std::string now_hms() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_r(&t, &tm);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
    return buf;
}

}  // namespace

EventLog& events() {
    static EventLog log;
    return log;
}

EventLog::EventLog(size_t cap) : cap_(cap) {}

void EventLog::push(const std::string& msg) {
    std::lock_guard<std::mutex> lk(mtx_);
    std::string json = "{\"t\":\"" + now_hms() + "\",\"msg\":\"" + json_escape(msg) + "\"}";
    events_.push_back(json);
    if (events_.size() > cap_) events_.pop_front();

    std::string frame = "data: " + json + "\n\n";
    for (auto it = subscribers_.begin(); it != subscribers_.end();) {
        if (write_fd(*it, frame)) {
            ++it;
        } else {
            it = subscribers_.erase(it);
        }
    }
}

std::string EventLog::backlog() const {
    std::lock_guard<std::mutex> lk(mtx_);
    std::string out;
    for (const auto& e : events_) out += "data: " + e + "\n\n";
    return out;
}

void EventLog::add_subscriber(int fd) {
    std::lock_guard<std::mutex> lk(mtx_);
    subscribers_.push_back(fd);
}

void EventLog::remove_subscriber(int fd) {
    std::lock_guard<std::mutex> lk(mtx_);
    for (auto it = subscribers_.begin(); it != subscribers_.end(); ++it) {
        if (*it == fd) {
            subscribers_.erase(it);
            break;
        }
    }
}

bool EventLog::write_fd(int fd, const std::string& s) {
    ssize_t n = ::send(fd, s.data(), s.size(), MSG_NOSIGNAL);
    return n == static_cast<ssize_t>(s.size());
}

void EventLog::handle_sse(int fd) {
    const std::string headers =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/event-stream\r\n"
        "Cache-Control: no-cache\r\n"
        "Connection: keep-alive\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n";
    ::send(fd, headers.data(), headers.size(), MSG_NOSIGNAL);

    const std::string initial = "retry: 2000\n\n" + backlog();
    ::send(fd, initial.data(), initial.size(), MSG_NOSIGNAL);

    add_subscriber(fd);

    std::thread([this, fd]() {
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        while (true) {
            struct pollfd pfd;
            pfd.fd = fd;
            pfd.events = POLLIN;
            pfd.revents = 0;
            int pr = ::poll(&pfd, 1, 30000);
            if (pr > 0) break;
            if (pr < 0 && errno != EINTR) break;
            {
                std::lock_guard<std::mutex> lk(mtx_);
                ::send(fd, ": ping\n\n", 8, MSG_NOSIGNAL);
            }
        }

        remove_subscriber(fd);
        ::close(fd);
    }).detach();
}

}  // namespace http
