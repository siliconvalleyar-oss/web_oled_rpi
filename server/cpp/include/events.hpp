#ifndef HTTP_EVENTS_HPP
#define HTTP_EVENTS_HPP

#include <deque>
#include <mutex>
#include <string>
#include <vector>

namespace http {

class EventLog {
public:
    explicit EventLog(size_t cap = 500);

    void push(const std::string& msg);
    std::string backlog() const;
    void add_subscriber(int fd);
    void remove_subscriber(int fd);
    void handle_sse(int fd);

private:
    bool write_fd(int fd, const std::string& s);

    size_t cap_;
    mutable std::mutex mtx_;
    std::deque<std::string> events_;
    std::vector<int> subscribers_;
};

EventLog& events();

}  // namespace http

#endif  // HTTP_EVENTS_HPP
