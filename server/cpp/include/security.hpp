#ifndef SECURITY_HPP
#define SECURITY_HPP

#include <string>

namespace http {

std::string sha256_hex(const std::string& data);

}  // namespace http

#endif  // SECURITY_HPP
