#include "system_info.hpp"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

namespace http {

namespace {

std::string read_first_line(const std::string& path) {
    std::ifstream file(path);
    std::string line;
    if (file) std::getline(file, line);
    return line;
}

long long parse_kb(const std::string& meminfo, const std::string& key) {
    size_t pos = meminfo.find(key + ":");
    if (pos == std::string::npos) return 0;
    pos += key.size() + 1;
    return std::atoll(meminfo.c_str() + pos) * 1024;
}

std::string local_ip() {
    ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) != 0) return "";
    std::string result;
    for (ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;
        if (std::string(ifa->ifa_name) == "lo") continue;
        char buf[INET_ADDRSTRLEN] = {0};
        sockaddr_in* sa = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
        inet_ntop(AF_INET, &sa->sin_addr, buf, sizeof(buf));
        result = buf;
        break;
    }
    freeifaddrs(ifaddr);
    return result;
}

std::string cpu_model() {
    std::ifstream file("/proc/cpuinfo");
    std::string line;
    while (std::getline(file, line)) {
        if (line.compare(0, 5, "Model") == 0) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string model = line.substr(pos + 1);
                model.erase(0, model.find_first_not_of(" \t"));
                return model;
            }
        }
    }
    return "unknown";
}

}  // namespace

std::string build_system_json() {
    std::ostringstream js;

    struct sysinfo si{};
    sysinfo(&si);

    std::string meminfo;
    {
        std::ifstream f("/proc/meminfo");
        std::ostringstream ss;
        ss << f.rdbuf();
        meminfo = ss.str();
    }

    long long mem_total = parse_kb(meminfo, "MemTotal");
    long long mem_avail = parse_kb(meminfo, "MemAvailable");

    struct statvfs sv{};
    statvfs("/", &sv);
    unsigned long long disk_total = static_cast<unsigned long long>(sv.f_blocks) * sv.f_frsize;
    unsigned long long disk_free = static_cast<unsigned long long>(sv.f_bavail) * sv.f_frsize;
    unsigned long long disk_used = disk_total - static_cast<unsigned long long>(sv.f_bfree) * sv.f_frsize;

    std::string temp_raw = read_first_line("/sys/class/thermal/thermal_zone0/temp");
    long long temp_c = -1;
    if (!temp_raw.empty()) temp_c = std::atoll(temp_raw.c_str()) / 1000;

    char hostname_buf[256] = {0};
    gethostname(hostname_buf, sizeof(hostname_buf));

    double load[3] = {0, 0, 0};
    getloadavg(load, 3);

    std::string ip = local_ip();
    std::string host = hostname_buf;

    js << "{";
    js << "\"hostname\":\"" << host << "\",";
    js << "\"ip\":\"" << ip << "\",";
    js << "\"uptime_seconds\":" << si.uptime << ",";
    js << "\"loadavg\":[" << load[0] << "," << load[1] << "," << load[2] << "],";
    js << "\"cores\":" << sysconf(_SC_NPROCESSORS_ONLN) << ",";
    js << "\"cpu_model\":\"" << cpu_model() << "\",";
    js << "\"temp_c\":" << temp_c << ",";
    js << "\"memory\":{";
    js << "\"total\":" << mem_total << ",";
    js << "\"used\":" << (mem_total - mem_avail) << ",";
    js << "\"available\":" << mem_avail;
    js << "},";
    js << "\"disk\":{";
    js << "\"total\":" << disk_total << ",";
    js << "\"used\":" << disk_used << ",";
    js << "\"free\":" << disk_free;
    js << "}";
    js << "}";

    return js.str();
}

}  // namespace http
