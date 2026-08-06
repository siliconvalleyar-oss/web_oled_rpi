#include "server.hpp"
#include "system_info.hpp"

#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

int main(int argc, char* argv[]) {
    int port = 8081;
    std::string doc_root = "web";

    if (argc > 1) port = std::atoi(argv[1]);
    if (argc > 2) doc_root = argv[2];

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

    return server.run();
}
