#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "http_request.hpp"
#include "http_response.hpp"

#include <atomic>
#include <functional>
#include <map>
#include <string>

namespace http {

using Handler = std::function<HttpResponse(const HttpRequest&)>;
using RequestHook = std::function<void(int status, long long count, const std::string& path)>;

class HttpServer {
public:
    HttpServer(int port, const std::string& doc_root);

    void register_handler(const std::string& path, Handler handler);
    void set_request_hook(RequestHook hook);
    int run(std::atomic<bool>& running);

private:
    void handle_client(int client_fd);
    HttpResponse route(const HttpRequest& req) const;

    int port_;
    std::string doc_root_;
    int listen_fd_ = -1;
    long long requests_ = 0;
    std::map<std::string, Handler> handlers_;
    RequestHook request_hook_;
};

}  // namespace http

#endif  // HTTP_SERVER_HPP
