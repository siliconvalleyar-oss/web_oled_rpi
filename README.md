# raspberry-src — Servidor Web C++ y Python (Raspberry Pi)

Dashboard web para la Raspberry Pi Zero 2 W con dos implementaciones de
servidor y frontend compartido:

- **C++** (`server/cpp`) — sockets POSIX puros, sin frameworks externos
- **Python** (`server/python`) — servidor estándar con API JSON extendida

Ambos sirven el mismo frontend escalable en `web/` y se acceden desde la red.
Incluye además la librería **SSD1306_OLED_RPI** vendored en `libs/ssd1306/`
para el display OLED 128x32 (I2C).

## Características

- Sockets TCP crudos en C++ (`socket`, `bind`, `listen`, `accept`)
- API JSON del sistema: `/api/status`, `/api/pihole`
- Frontend escalable (HTML/CSS/JS separados) en `web/`
- Dashboard con info del sistema y de Pi-hole, auto-refresh cada 10 s
- Librería OLED SSD1306 vendored (128x32, I2C 0x3C)
- Build simple con `make`

## Estructura del proyecto

```
raspberry-src/
├── Makefile             # orquestación: cpp, run/start/stop, py-*, clean
├── README.md
├── .gitignore
├── server/
│   ├── cpp/             # servidor C++ (sockets)
│   │   ├── Makefile     # all, run, start, stop, restart, clean, cleanall
│   │   ├── src/         # código fuente (.cpp)
│   │   └── include/     # cabeceras (.hpp)
│   └── python/
│       └── serve.py     # servidor Python (puerto 8000) + API
├── web/                 # frontend escalable (doc_root)
│   ├── index.html       # dashboard
│   ├── css/style.css
│   ├── js/api.js        # helpers de red/formato
│   ├── js/main.js       # lógica del dashboard
│   ├── img/             # iconos/logo
│   └── data/            # datos estáticos
├── libs/
│   └── ssd1306/         # librería OLED SSD1306 vendored (include/, src/, Makefile)
├── var/                 # runtime (gitignored)
│   ├── input/           # archivos de entrada para pruebas
│   ├── output/          # resultados de pruebas
│   └── logs/            # logs de ejecución
└── docs/                # documentación en Markdown
```

## Documentación

| Archivo | Contenido |
|---|---|
| [docs/README.md](docs/README.md) | Índice de documentación |
| [docs/BUILD.md](docs/BUILD.md) | Requisitos y compilación |
| [docs/USAGE.md](docs/USAGE.md) | Ejecución (C++ y Python) |
| [docs/DEPLOY.md](docs/DEPLOY.md) | Despliegue en la Pi (systemd) |
| [docs/TESTING.md](docs/TESTING.md) | Pruebas y benchmark |
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | Diseño y módulos |
| [docs/API.md](docs/API.md) | Endpoints HTTP |
| [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) | Problemas frecuentes |

## Arranque rápido

Servidor C++ (puerto 8081):

```bash
make cleanall && make
make start PORT=8081
```

Servidor Python (puerto 8000):

```bash
make py-start PY_PORT=8000
```

Probar desde otro equipo:

```bash
curl http://rpi2w.local:8081/          # dashboard C++
curl http://rpi2w.local:8000/          # dashboard Python
curl http://rpi2w.local:8000/api/status
curl http://rpi2w.local:8000/api/pihole
```
