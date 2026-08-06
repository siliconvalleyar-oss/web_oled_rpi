# Documentación del proyecto

Índice de la documentación del servidor web C++/Python para Raspberry Pi.

Guía principal: [README.md](../README.md)

## Índice

| Documento | Contenido |
|---|---|
| [BUILD.md](BUILD.md) | Requisitos, compilación del servidor C++ y flags |
| [USAGE.md](USAGE.md) | Ejecución del servidor C++ y Python |
| [DEPLOY.md](DEPLOY.md) | Despliegue en la Pi como servicio systemd |
| [TESTING.md](TESTING.md) | Pruebas y benchmark de transferencia de 1 MB |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Diseño de módulos y flujo de peticiones |
| [API.md](API.md) | Endpoints HTTP de ambos servidores |
| [TROUBLESHOOTING.md](TROUBLESHOOTING.md) | Problemas frecuentes y soluciones |
| [LEARNINGS.md](LEARNINGS.md) | Reglas de versionado (tags), credenciales git y push |

## Frontend (`web/`)

El frontend es escalable y no depende del servidor: HTML, CSS y JS separados.

```
web/
├── index.html      # dashboard (sistema + Pi-hole)
├── css/style.css   # estilos (tema oscuro, responsive)
├── js/api.js       # helpers: fetchJSON, formateadores
├── js/main.js      # lógica: carga /api/status y /api/pihole
├── img/            # iconos y logo
└── data/           # datos estáticos
```

El dashboard consume `GET /api/status` y `GET /api/pihole` (misma origin),
disponibles en ambos servidores, y refresca cada 10 segundos.
