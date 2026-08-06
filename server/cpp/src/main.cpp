#include "server.hpp"
#include "system_info.hpp"
#include "client.hpp"
#include "json_util.hpp"
#include "oled_status.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <csetjmp>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

namespace {

std::atomic<bool> g_running(true);
OledStatus g_oled;
std::chrono::steady_clock::time_point g_last_oled = std::chrono::steady_clock::now();

void signal_handler(int) {
    g_running = false;
}

jmp_buf g_oled_jmp;
volatile sig_atomic_t g_oled_timed_out = 0;

void oled_watchdog_handler(int) {
    g_oled_timed_out = 1;
    std::longjmp(g_oled_jmp, 1);
}

void oled_arm_watchdog() {
    struct sigaction sa{};
    sa.sa_handler = oled_watchdog_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, nullptr);
}

bool oled_init_guarded() {
    oled_arm_watchdog();
    if (setjmp(g_oled_jmp) != 0) {
        std::fprintf(stderr, "[OLED] Init agotado (bus I2C atascado). Continuando sin display.\n");
        g_oled.disable();
        return false;
    }
    alarm(5);
    bool ok = g_oled.init();
    alarm(0);
    return ok;
}

void oled_show_guarded(const std::string& l1, const std::string& l2,
                       const std::string& l3, const std::string& l4) {
    if (!g_oled.ready()) return;
    oled_arm_watchdog();
    if (setjmp(g_oled_jmp) != 0) {
        std::fprintf(stderr, "[OLED] Actualizacion agotada (bus I2C atascado). Display desactivado.\n");
        g_oled.disable();
        return;
    }
    alarm(2);
    g_oled.show(l1, l2, l3, l4);
    alarm(0);
}

void oled_throttled(const std::string& l1, const std::string& l2,
                    const std::string& l3, const std::string& l4) {
    auto now = std::chrono::steady_clock::now();
    if (now - g_last_oled < std::chrono::milliseconds(250)) return;
    g_last_oled = now;
    oled_show_guarded(l1, l2, l3, l4);
}

}  // namespace

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    if (argc > 1 && std::string(argv[1]) == "send") {
        std::string host = (argc > 2) ? argv[2] : "rpi2w.local";
        int port = (argc > 3) ? std::atoi(argv[3]) : 8081;
        return http::send_status(host, port) ? 0 : 1;
    }

    int port = 8081;
    std::string doc_root = "web";

    if (argc > 1) port = std::atoi(argv[1]);
    if (argc > 2) doc_root = argv[2];

    std::string peer_json = "{}";

    oled_init_guarded();
    oled_show_guarded("RPi Web Server", "Iniciando...", "Modo: recibe", "");

    http::HttpServer server(port, doc_root);

    server.register_handler("/api/status", [](const http::HttpRequest&) {
        http::HttpResponse res;
        res.content_type("application/json").body(http::build_system_json());
        return res;
    });

    server.register_handler("/api/peer", [&peer_json](const http::HttpRequest& req) {
        if (req.method() == "POST" && !req.body().empty()) {
            peer_json = req.body();
        }
        http::HttpResponse res;
        res.content_type("application/json").body(peer_json);
        return res;
    });

    server.register_handler("/status", [](const http::HttpRequest&) {
        std::ostringstream js;
        std::string hostname = "unknown";
        std::ifstream hf("/etc/hostname");
        if (hf) std::getline(hf, hostname);

        js << "{\n";
        js << "  \"server\": \"cpp-socket-server\",\n";
        js << "  \"hostname\": \"" << hostname << "\",\n";
        js << "  \"uptime_seconds\": " << time(nullptr) << "\n";
        js << "}\n";

        http::HttpResponse res;
        res.content_type("application/json").body(js.str());
        return res;
    });

    server.set_request_hook([port, &peer_json](int status, long long count, const std::string& path) {
        std::string line4 = path;
        std::string peer_host = http::json_string(peer_json, "hostname");
        if (!peer_host.empty()) {
            line4 = "PC: " + peer_host + " " + http::json_string(peer_json, "ip");
            long long peer_temp = http::json_long(peer_json, "temp_c");
            if (peer_temp >= 0) line4 += " " + std::to_string(peer_temp) + "C";
        }
        oled_throttled(
            "RPi Web Server",
            "Escuchando :" + std::to_string(port),
            "Req: " + std::to_string(count) + "  " + std::to_string(status),
            http::json_truncate(line4, 20));
    });

    std::cout << "Inicializando OLED..." << std::endl;
    oled_show_guarded("RPi Web Server", "Escuchando", "0.0.0.0:" + std::to_string(port), "doc: " + doc_root);

    int rc = server.run(g_running);

    std::cout << "\nDeteniendo servidor..." << std::endl;
    oled_show_guarded("RPi Web Server", "Apagando...", "", "");
    g_oled.shutdown();

    std::cout << "[OK] Bye!" << std::endl;
    return rc;
}
