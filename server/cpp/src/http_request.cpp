#include "http_request.hpp"

#include "http_common.hpp"

#include <cctype>
#include <cstdlib>
#include <sstream>

namespace http {

bool HttpRequest::parse(const std::string& raw) {
    size_t header_end = raw.find("\r\n\r\n");
    std::string head = (header_end == std::string::npos) ? raw : raw.substr(0, header_end);

    std::istringstream iss(head);
    if (!(iss >> method_ >> target_ >> version_)) {
        return false;
    }

    path_ = target_;
    size_t qpos = path_.find('?');
    if (qpos != std::string::npos) {
        path_ = path_.substr(0, qpos);
    }
    path_ = url_decode(path_);

    std::string line;
    std::getline(iss, line);
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string key = line.substr(0, colon);
        for (char& c : key) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        std::string val = line.substr(colon + 1);
        size_t start = val.find_first_not_of(" \t");
        if (start != std::string::npos) val = val.substr(start);
        headers_[key] = val;
    }

    if (header_end != std::string::npos) {
        int cl = content_length();
        if (cl > 0) {
            body_ = raw.substr(header_end + 4, static_cast<size_t>(cl));
        }
    }

    valid_ = true;
    return true;
}

int HttpRequest::content_length() const {
    auto it = headers_.find("content-length");
    if (it == headers_.end()) return 0;
    return std::atoi(it->second.c_str());
}

}  // namespace http
