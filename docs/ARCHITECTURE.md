# ARCHITECTURE.md — Diseño

## Visión general

Hay dos servidores que sirven el mismo frontend de `web/`:

- **C++** (`server/cpp/bin/web_server`): single-threaded sobre sockets TCP
- **Python** (`server/python/serve.py`): `http.server` con hilos, API extendida

Flujo de cada petición (C++):

```
TCP accept → leer request (hasta CRLFCRLF) → parsear → routear → construir response → enviar → close
```

## Módulos C++

| Módulo | Archivos | Responsabilidad |
|---|---|---|
| `HttpServer` | `server/cpp/include/server.hpp` / `server/cpp/src/server.cpp` | Ciclo de vida del socket, aceptar y atender clientes, enrutado |
| `HttpRequest` | `server/cpp/include/http_request.hpp` / `server/cpp/src/http_request.cpp` | Parseo de la línea de petición (método, target, versión, path) |
| `HttpResponse` | `server/cpp/include/http_response.hpp` / `server/cpp/src/http_response.cpp` | Construcción del response HTTP (código, headers, body) |
| `MIME types` | `server/cpp/include/mime_types.hpp` / `server/cpp/src/mime_types.cpp` | Detección de Content-Type por extensión |
| `system_info` | `server/cpp/include/system_info.hpp` / `server/cpp/src/system_info.cpp` | API del sistema: `/api/status` (hostname, IP, uptime, CPU, RAM, disco) |
| `http_common` | `server/cpp/include/http_common.hpp` / `server/cpp/src/http_common.cpp` | Utilidades: códigos de estado, url_decode |

## Servidor Python (`server/python/serve.py`)

Servidor `ThreadingHTTPServer` con un handler que:

- Sirve archivos estáticos de `web/` (con protección de path traversal)
- Expone `/api/status` (lee `/proc/*`) y `/api/pihole` (estado Pi-hole vía
  `sudo -n pihole status` + conteo de `gravity.db`)

El endpoint `/api/pihole` depende de `sudo` sin contraseña (NOPASSWD) para
consultar el estado de Pi-hole.

## Enrutado extensible (C++)

El servidor usa un mapa de *handlers* registrados por ruta:

```cpp
using Handler = std::function<HttpResponse(const HttpRequest&)>;
server.register_handler("/api/status", [](const HttpRequest& req) { ... });
```

Si la ruta no tiene handler registrado, se intenta servir un archivo estático
desde `DOC_ROOT` (`/` mapea a `index.html`). Si no existe se responde `404`.

## Frontend (`web/`)

Frontend escalable e independiente del servidor:

```
web/
├── index.html      # dashboard
├── css/style.css   # estilos
├── js/api.js       # helpers (fetchJSON, formateadores)
├── js/main.js      # lógica del dashboard
├── img/            # iconos
└── data/           # datos estáticos
```

El dashboard consulta `/api/status` y `/api/pihole` (relativos = misma origin)
y refresca cada 10 s. Así funciona igual servido por C++ o Python.

## Ciclo del servidor C++

1. `socket()` crea el descriptor de escucha
2. `setsockopt(SO_REUSEADDR)` permite reusar el puerto
3. `bind()` asocia la dirección `0.0.0.0:PUERTO`
4. `listen()` habilita cola de conexiones
5. Bucle infinito: `accept()` → `handle_client()` → `close()`

## Seguridad / limitaciones conocidas

- C++ single-threaded: una petición a la vez
- No maneja chunked transfer ni body de POST
- Sin TLS (para TLS se usa Pi-Hole en el puerto 443)
