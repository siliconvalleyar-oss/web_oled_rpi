#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <map>
#include <string>

namespace http {

class HttpRequest {
public:
    bool parse(const std::string& raw);

    const std::string& method() const { return method_; }
    const std::string& target() const { return target_; }
    const std::string& version() const { return version_; }
    const std::string& path() const { return path_; }
    bool valid() const { return valid_; }

private:
    std::string method_;
    std::string target_;
    std::string version_;
    std::string path_;
    std::map<std::string, std::string> headers_;
    bool valid_ = false;
};

}  // namespace http

#endif  // HTTP_REQUEST_HPP
