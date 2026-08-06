#include "server.hpp"

#include "http_common.hpp"
#include "mime_types.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

namespace http {

HttpServer::HttpServer(int port, const std::string& doc_root)
    : port_(port), doc_root_(doc_root) {}

void HttpServer::register_handler(const std::string& path, Handler handler) {
    handlers_[path] = std::move(handler);
}

HttpResponse HttpServer::route(const HttpRequest& req) const {
    auto it = handlers_.find(req.path());
    if (it != handlers_.end()) {
        return it->second(req);
    }

    std::string file_path = doc_root_;
    if (req.path() == "/") {
        file_path += "/index.html";
    } else {
        file_path += req.path();
    }

    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        HttpResponse res;
        res.status(404)
            .content_type("text/html; charset=utf-8")
            .body("<html><body><h1>404 Not Found</h1><p>" + file_path + "</p></body></html>");
        return res;
    }

    std::ostringstream ss;
    ss << file.rdbuf();

    HttpResponse res;
    res.content_type(mime_type_for(file_path)).body(ss.str());
    return res;
}

int HttpServer::run() {
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        std::cerr << "socket() failed: " << std::strerror(errno) << std::endl;
        return 1;
    }

    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(port_));

    if (bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "bind() failed: " << std::strerror(errno) << std::endl;
        ::close(listen_fd_);
        return 1;
    }

    if (listen(listen_fd_, 16) < 0) {
        std::cerr << "listen() failed: " << std::strerror(errno) << std::endl;
        ::close(listen_fd_);
        return 1;
    }

    std::cout << "Servidor C++ escuchando en 0.0.0.0:" << port_
              << " (doc_root=" << doc_root_ << ")" << std::endl;

    while (true) {
        sockaddr_in client{};
        socklen_t client_len = sizeof(client);
        int client_fd = accept(listen_fd_, reinterpret_cast<sockaddr*>(&client), &client_len);
        if (client_fd < 0) {
            std::cerr << "accept() failed: " << std::strerror(errno) << std::endl;
            continue;
        }
        handle_client(client_fd);
    }

    ::close(listen_fd_);
    return 0;
}

void HttpServer::handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    std::string request;

    while (request.find("\r\n\r\n") == std::string::npos) {
        ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) break;
        buffer[n] = '\0';
        request.append(buffer, static_cast<size_t>(n));
    }

    if (!request.empty()) {
        HttpRequest req;
        HttpResponse res;

        if (req.parse(request)) {
            res = route(req);
        } else {
            res.status(400)
                .content_type("text/html; charset=utf-8")
                .body("<html><body><h1>400 Bad Request</h1></body></html>");
        }

        std::string response = res.build();
        send(client_fd, response.data(), response.size(), 0);

        std::cout << req.method() << " " << req.path() << " -> "
                  << response.substr(9, 3) << std::endl;
    }

    ::close(client_fd);
}

}  // namespace http
