#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <string>

namespace http {

bool send_status(const std::string& host, int port);

}  // namespace http

#endif  // HTTP_CLIENT_HPP
