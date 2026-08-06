# DEPLOY.md — Despliegue en la Raspberry Pi

## Opción 1: Primer plano

Servidor C++:

```bash
cd /home/pi/src/web_oled_rpi
make run PORT=8081
```

Servidor Python:

```bash
make py-run PY_PORT=8000
```

## Opción 2: Segundo plano (manual)

```bash
make start PORT=8081       # C++
make py-start PY_PORT=8000 # Python
```

Detener:

```bash
make stop                  # C++
make py-stop               # Python
```

## Opción 3: Servicio systemd (recomendado)

### Servidor C++

Crear `/etc/systemd/system/web-server.service`:

```ini
[Unit]
Description=Servidor Web C++ (sockets)
After=network.target

[Service]
WorkingDirectory=/home/pi/src/web_oled_rpi
ExecStart=/home/pi/src/web_oled_rpi/server/cpp/bin/web_server 8081 web
Restart=on-failure
RestartSec=3
User=pi
StandardOutput=append:/home/pi/src/web_oled_rpi/var/logs/server.log
StandardError=append:/home/pi/src/web_oled_rpi/var/logs/server.log

[Install]
WantedBy=multi-user.target
```

### Servidor Python

Crear `/etc/systemd/system/web-server-py.service`:

```ini
[Unit]
Description=Servidor Web Python (dashboard + API)
After=network.target

[Service]
WorkingDirectory=/home/pi/src/web_oled_rpi
ExecStart=/usr/bin/python3 /home/pi/src/web_oled_rpi/server/python/serve.py 8000
Restart=on-failure
RestartSec=3
User=pi
StandardOutput=append:/home/pi/src/web_oled_rpi/var/logs/python.log
StandardError=append:/home/pi/src/web_oled_rpi/var/logs/python.log

[Install]
WantedBy=multi-user.target
```

> El servicio Python usa `sudo -n pihole status` para el endpoint `/api/pihole`;
> requiere que el usuario `pi` tenga sudo sin contraseña (NOPASSWD).

Instalar y habilitar:

```bash
sudo cp web-server.service /etc/systemd/system/
sudo cp web-server-py.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now web-server web-server-py
```

Comandos útiles:

```bash
sudo systemctl restart web-server
sudo systemctl restart web-server-py
journalctl -u web-server -f
journalctl -u web-server-py -f
```

## Firewall (UFW)

El servidor escucha en `0.0.0.0`. Con UFW activo abrir el puerto:

```bash
sudo ufw allow 8081/tcp comment 'C++ web server'
```

Verificar:

```bash
sudo ufw status | grep 8081
ss -tln | grep 8081
```

## Acceso desde la red

El hostname `rpi2w.local` resuelve a la IP de la Pi:

```bash
curl http://rpi2w.local:8081/
```

O por IP directa (ej. `192.168.1.38`):

```bash
curl http://192.168.1.38:8081/
```
