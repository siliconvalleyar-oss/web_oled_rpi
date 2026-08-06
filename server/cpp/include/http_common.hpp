#ifndef HTTP_COMMON_HPP
#define HTTP_COMMON_HPP

#include <string>

namespace http {

constexpr int BUFFER_SIZE = 8192;

std::string status_text(int code);
std::string url_decode(const std::string& in);

}  // namespace http

#endif  // HTTP_COMMON_HPP
