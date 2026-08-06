#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>

namespace http {

class HttpResponse {
public:
    HttpResponse& status(int code);
    HttpResponse& content_type(const std::string& type);
    HttpResponse& body(const std::string& content);

    std::string build() const;
    int status_code() const { return status_; }

private:
    int status_ = 200;
    std::string content_type_ = "text/html; charset=utf-8";
    std::string body_;
};

}  // namespace http

#endif  // HTTP_RESPONSE_HPP
