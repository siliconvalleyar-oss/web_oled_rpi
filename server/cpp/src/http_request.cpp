#include "http_request.hpp"

#include "http_common.hpp"

#include <sstream>

namespace http {

bool HttpRequest::parse(const std::string& raw) {
    std::istringstream iss(raw);
    if (!(iss >> method_ >> target_ >> version_)) {
        return false;
    }

    path_ = target_;
    size_t qpos = path_.find('?');
    if (qpos != std::string::npos) {
        path_ = path_.substr(0, qpos);
    }
    path_ = url_decode(path_);

    valid_ = true;
    return true;
}

}  // namespace http
