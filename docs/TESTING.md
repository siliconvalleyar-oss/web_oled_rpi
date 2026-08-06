# TESTING.md — Pruebas

## Verificación básica

Los dos servidores deben responder en sus puertos:

- C++: `8081`
- Python: `8000`

```bash
# Página principal
curl -i http://rpi2w.local:8081/

# API del sistema (ambos servidores)
curl -s http://rpi2w.local:8081/api/status
curl -s http://rpi2w.local:8000/api/status

# API de Pi-hole (solo Python)
curl -s http://rpi2w.local:8000/api/pihole

# Archivo inexistente
curl -i http://rpi2w.local:8081/noexiste
```

| Caso | Resultado esperado |
|---|---|
| `GET /` | `200 OK` + dashboard HTML |
| `GET /api/status` | `200 OK` + JSON del sistema |
| `GET /api/pihole` | `200 OK` + JSON de Pi-hole |
| `GET /noexiste` | `404 Not Found` |

## Verificación del frontend

Todos los recursos del dashboard deben responder `200`:

```bash
for f in / /css/style.css /js/api.js /js/main.js /img/favicon.svg /api/status; do
  curl -s -o /dev/null -w "%{http_code}  $f\n" http://rpi2w.local:8000$f
done
```

## Prueba de transferencia (ida y vuelta)

### Preparar un archivo de 1 MB

```bash
head -c 1048576 /dev/urandom > test1mb.bin
md5sum test1mb.bin   # guardar el hash
```

### Envío (subida a la Pi)

```bash
time scp test1mb.bin pi@rpi2w.local:/home/pi/src/web_oled_rpi/var/input/
```

### Recepción (descarga vía servidor C++)

```bash
curl -s -o /tmp/download.bin -w "velocidad: %{speed_download} B/s | tiempo: %{time_total} s\n" \
  http://rpi2w.local:8081/test1mb.bin

md5sum /tmp/download.bin   # debe coincidir con el hash de origen
```

### Resultados de referencia (WiFi, Pi Zero 2 W)

| Dirección | Método | Tiempo (1 MB) | Velocidad |
|---|---|---|---|
| Subida local → Pi | scp/SSH | ~2.7 s | ~0.39 MB/s |
| Descarga Pi → local | HTTP (servidor C++) | ~0.29 s | ~3.65 MB/s |

La descarga HTTP es ~10x más rápida que la subida scp (el cifrado SSH satura la CPU de la Pi Zero).

## Logs

El servidor registra cada petición en `var/logs/server.log`:

```
Servidor C++ escuchando en 0.0.0.0:8081 (doc_root=.)
192.168.1.41 <- GET / HTTP/1.1 -> 200
192.168.1.41 <- GET /status HTTP/1.1 -> 200
```

Ver en vivo:

```bash
tail -f var/logs/server.log
```
