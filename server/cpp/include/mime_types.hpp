#ifndef MIME_TYPES_HPP
#define MIME_TYPES_HPP

#include <string>

namespace http {

std::string mime_type_for(const std::string& path);

}  // namespace http

#endif  // MIME_TYPES_HPP
