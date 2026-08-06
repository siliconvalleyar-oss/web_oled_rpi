# API.md — Endpoints HTTP

Servidores:

- **C++**: `http://rpi2w.local:8081`
- **Python**: `http://rpi2w.local:8000`

Ambos sirven el mismo frontend y el endpoint `/api/status`. Solo el servidor
Python expone `/api/pihole`.

## GET /

Sirve `web/index.html` (dashboard).

```
$ curl http://rpi2w.local:8081/
HTTP/1.1 200 OK
Content-Type: text/html; charset=utf-8
```

## GET /api/status

Información del sistema en JSON. Disponible en ambos servidores.

```
$ curl http://rpi2w.local:8000/api/status
{
  "hostname": "rpi2w",
  "ip": "192.168.1.38",
  "uptime_seconds": 14764.9,
  "loadavg": [0.24, 0.28, 0.27],
  "cores": 4,
  "cpu_model": "Raspberry Pi Zero 2 W Rev 1.0",
  "temp_c": 38,
  "memory": { "total": 497750016, "used": 174530560, "available": 323219456 },
  "disk": { "total": 30878613504, "used": 18723065856, "free": 10841419776 }
}
```

| Campo | Tipo | Descripción |
|---|---|---|
| `hostname` | string | Hostname de la máquina |
| `ip` | string | IP local (primera interfaz no-loopback) |
| `uptime_seconds` | number | Tiempo encendido en segundos |
| `loadavg` | array | Carga promedio 1/5/15 min |
| `cores` | int | Número de núcleos de CPU |
| `cpu_model` | string | Modelo de la CPU |
| `temp_c` | int | Temperatura de la CPU en °C |
| `memory` | object | `total`, `used`, `available` (bytes) |
| `disk` | object | `total`, `used`, `free` (bytes) |

## GET /api/pihole

Estado de Pi-hole. Solo servidor Python.

```
$ curl http://rpi2w.local:8000/api/pihole
{
  "installed": true,
  "blocking_enabled": true,
  "domains_blocked": 99558,
  "updated": "2026-08-05 21:53:38"
}
```

| Campo | Tipo | Descripción |
|---|---|---|
| `installed` | bool | ¿Pi-hole instalado? |
| `blocking_enabled` | bool | ¿Bloqueo de anuncios activo? |
| `domains_blocked` | int | Dominios en la lista de bloqueo (`gravity.db`) |
| `updated` | string | Última actualización de la lista |

## GET /status

Alias JSON del estado del servidor C++ (endpoint legacy).

```
$ curl http://rpi2w.local:8081/status
```

## GET /<archivo>

Sirve cualquier archivo dentro de `web/`.

```
$ curl http://rpi2w.local:8081/css/style.css
$ curl http://rpi2w.local:8081/js/main.js
```

## Códigos de estado

| Código | Descripción |
|---|---|
| `200 OK` | Petición satisfactoria |
| `400 Bad Request` | Request mal formado |
| `403 Forbidden` | Traversal de directorio (Python) |
| `404 Not Found` | Ruta o archivo inexistente |
| `405 Method Not Allowed` | Método distinto de `GET` |

## Métodos soportados

Solo `GET`. Cualquier otro método responde `405` (C++) o `501` (Python).
