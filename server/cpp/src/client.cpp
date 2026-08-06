#include "client.hpp"

#include "json_util.hpp"
#include "system_info.hpp"

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>

namespace http {

bool send_status(const std::string& host, int port) {
    std::string body = build_system_json();

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* res = nullptr;
    std::string port_str = std::to_string(port);
    if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res) != 0 || !res) {
        std::fprintf(stderr, "[SEND] No se pudo resolver el host '%s'\n", host.c_str());
        return false;
    }

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        std::fprintf(stderr, "[SEND] socket() fallo: %s\n", std::strerror(errno));
        freeaddrinfo(res);
        return false;
    }

    if (connect(fd, res->ai_addr, res->ai_addrlen) != 0) {
        std::fprintf(stderr, "[SEND] No se pudo conectar a %s:%d (%s)\n",
                     host.c_str(), port, std::strerror(errno));
        ::close(fd);
        freeaddrinfo(res);
        return false;
    }
    freeaddrinfo(res);

    std::ostringstream req;
    req << "POST /api/peer HTTP/1.1\r\n";
    req << "Host: " << host << "\r\n";
    req << "Content-Type: application/json\r\n";
    req << "Content-Length: " << body.size() << "\r\n";
    req << "Connection: close\r\n";
    req << "\r\n";
    req << body;

    std::string data = req.str();
    size_t sent = 0;
    while (sent < data.size()) {
        ssize_t n = send(fd, data.data() + sent, data.size() - sent, 0);
        if (n <= 0) {
            std::fprintf(stderr, "[SEND] Error al enviar: %s\n", std::strerror(errno));
            ::close(fd);
            return false;
        }
        sent += static_cast<size_t>(n);
    }

    std::string resp;
    char buf[4096];
    ssize_t n;
    while ((n = recv(fd, buf, sizeof(buf) - 1, 0)) > 0) {
        buf[n] = '\0';
        resp.append(buf, static_cast<size_t>(n));
    }
    ::close(fd);

    bool ok = resp.compare(0, 12, "HTTP/1.1 200") == 0;

    std::string hostname = json_string(body, "hostname");
    std::string ip = json_string(body, "ip");
    long long temp = json_long(body, "temp_c");
    std::printf("[SEND] Estado de %s (%s) %s%lldC -> %s:%d -> %s\n",
                hostname.c_str(), ip.c_str(),
                (temp >= 0) ? "" : "?",
                (temp >= 0) ? temp : -1,
                host.c_str(), port,
                ok ? "OK" : "FALLO");

    if (!ok) {
        std::fprintf(stderr, "[SEND] Respuesta: %s\n", resp.c_str());
    }
    return ok;
}

}  // namespace http
