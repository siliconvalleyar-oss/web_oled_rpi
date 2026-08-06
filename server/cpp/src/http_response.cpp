#include "http_response.hpp"

#include "http_common.hpp"

#include <sstream>

namespace http {

HttpResponse& HttpResponse::status(int code) {
    status_ = code;
    return *this;
}

HttpResponse& HttpResponse::content_type(const std::string& type) {
    content_type_ = type;
    return *this;
}

HttpResponse& HttpResponse::body(const std::string& content) {
    body_ = content;
    return *this;
}

std::string HttpResponse::build() const {
    std::ostringstream res;
    res << "HTTP/1.1 " << status_ << " " << status_text(status_) << "\r\n";
    res << "Content-Type: " << content_type_ << "\r\n";
    res << "Content-Length: " << body_.size() << "\r\n";
    res << "Connection: close\r\n";
    res << "\r\n";
    res << body_;
    return res.str();
}

}  // namespace http
