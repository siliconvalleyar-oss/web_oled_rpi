#ifndef JSON_UTIL_HPP
#define JSON_UTIL_HPP

#include <cstdlib>
#include <string>

namespace http {

inline std::string json_string(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\":\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return "";
    pos += needle.size();
    size_t end = json.find('"', pos);
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos);
}

inline long long json_long(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\":";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return -1;
    pos += needle.size();
    return std::atoll(json.c_str() + pos);
}

inline std::string json_truncate(const std::string& in, size_t max) {
    if (in.size() <= max) return in;
    if (max <= 3) return in.substr(0, max);
    return in.substr(0, max - 3) + "...";
}

}  // namespace http

#endif  // JSON_UTIL_HPP
