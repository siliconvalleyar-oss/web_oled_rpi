# USAGE.md — Ejecución

## Servidor C++

### Uso básico

```bash
./server/cpp/bin/web_server [PUERTO] [DOC_ROOT]
```

| Argumento | Default | Descripción |
|---|---|---|
| `PUERTO` | `8081` | Puerto TCP de escucha |
| `DOC_ROOT` | `web` | Directorio de archivos estáticos (desde la raíz del repo) |

### Primer plano

```bash
make run PORT=8081
```

### Segundo plano (démón)

```bash
make start PORT=8081
```

### Detener

```bash
make stop
```

Equivale a `pkill -x web_server_v<VERSION>` (el binario se nombra con la
versión del archivo `VERSION`, ej: `web_server_v1.0.6`).

> Usar `pkill -x` (nombre exacto). `pkill -f web_server` también coincide
> con el shell que lo ejecuta y puede matarse a sí mismo.

## Servidor Python

### Uso básico

```bash
python3 serve.py [PUERTO]
```

| Argumento | Default | Descripción |
|---|---|---|
| `PUERTO` | `8000` | Puerto TCP de escucha |

### Primer plano

```bash
make py-run PY_PORT=8000
```

### Segundo plano (démón)

```bash
make py-start PY_PORT=8000
```

### Detener

```bash
make py-stop
```

## Logs

- C++: `logs/server.log`
- Python: `logs/python.log`

```bash
tail -f logs/server.log
tail -f logs/python.log
```

## Notas sobre el firewall

Ambos servidores escuchan en `0.0.0.0`. Si UFW está activo, abrir los puertos:

```bash
sudo ufw allow 8081/tcp
sudo ufw allow 8000/tcp
```
