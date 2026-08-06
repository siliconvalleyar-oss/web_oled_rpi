#include "server.hpp"
#include "system_info.hpp"
#include "oled_status.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

namespace {

std::atomic<bool> g_running(true);
OledStatus g_oled;
std::chrono::steady_clock::time_point g_last_oled = std::chrono::steady_clock::now();

void signal_handler(int) {
    g_running = false;
}

void oled_throttled(const std::string& l1, const std::string& l2,
                    const std::string& l3, const std::string& l4) {
    auto now = std::chrono::steady_clock::now();
    if (now - g_last_oled < std::chrono::milliseconds(250)) return;
    g_last_oled = now;
    g_oled.show(l1, l2, l3, l4);
}

}  // namespace

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    int port = 8081;
    std::string doc_root = "web";

    if (argc > 1) port = std::atoi(argv[1]);
    if (argc > 2) doc_root = argv[2];

    g_oled.init();
    g_oled.show("RPi Web Server", "Iniciando...", "v1.0.1", "");

    http::HttpServer server(port, doc_root);

    server.register_handler("/api/status", [](const http::HttpRequest&) {
        http::HttpResponse res;
        res.content_type("application/json").body(http::build_system_json());
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

    server.set_request_hook([port](int status, long long count, const std::string& path) {
        oled_throttled(
            "RPi Web Server",
            "Escuchando :" + std::to_string(port),
            "Req: " + std::to_string(count) + "  " + std::to_string(status),
            path);
    });

    std::cout << "Inicializando OLED..." << std::endl;
    g_oled.show("RPi Web Server", "Escuchando", "0.0.0.0:" + std::to_string(port), "doc: " + doc_root);

    int rc = server.run(g_running);

    std::cout << "\nDeteniendo servidor..." << std::endl;
    g_oled.show("RPi Web Server", "Apagando...");
    g_oled.shutdown();

    std::cout << "[OK] Bye!" << std::endl;
    return rc;
}
