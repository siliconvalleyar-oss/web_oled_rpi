# TROUBLESHOOTING.md — Solución de problemas

## El puerto 8081 no responde desde otro equipo

**Causa:** UFW bloquea el puerto (default deny incoming).

**Solución:**

```bash
sudo ufw allow 8081/tcp
sudo ufw status | grep 8081
```

## El puerto ya está en uso

**Error al iniciar:** `bind() failed: Address already in use`

**Causa:** otra instancia del servidor sigue corriendo.

**Solución:**

```bash
make stop
ss -tln | grep 8081   # debe estar vacío
```

## El proceso no se detiene con pkill -f

`pkill -f web_server` coincide con cualquier proceso cuya línea de
comandos contenga `web_server`, incluido el propio shell que lo ejecuta.
Usar siempre el nombre exacto. El binario se nombra con la versión
(`web_server_v<VERSION>`, ej: `web_server_v1.0.6`):

```bash
pkill -x web_server_v$(cat ../../VERSION)
```

## ifconfig: command not found al conectar por SSH

**Causa:** `/sbin` y `/usr/sbin` no están en el `PATH` (el `~/.bash_profile`
sobrescribe el PATH del sistema).

**Solución** (ya aplicada en esta Raspberry Pi):

```bash
# /etc/environment
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

# ~/.bash_profile  (se añadió al final)
export PATH=$PATH:/usr/sbin:/sbin
```

En una sesión ya abierta:

```bash
source ~/.bash_profile
```

## La web de Pi-Hole no carga en el puerto 80

**Causa:** Pi-Hole v6 usa su propio servidor web integrado; en esta Pi quedó
configurado en el puerto **8080** porque Apache ocupaba el 80.

**Solución:** usar `http://rpi2w.local:8080/admin`. Si se prefiere el 80,
desactivar Apache (`sudo systemctl disable --now apache2`) y cambiar
`webserver.port` en `/etc/pihole/pihole.toml`.

## No hay logs en el servidor

**Causa:** el proceso se inició en primer plano o el directorio `var/logs/` no existe.

**Solución:**

```bash
mkdir -p var/logs
nohup ./server/cpp/bin/web_server 8081 web > var/logs/server.log 2>&1 &
tail -f var/logs/server.log
```

## Reset de la base de datos gravity de Pi-Hole

Error en FTL: `SQL logic error` al actualizar `gravity.db`.

**Solución:**

```bash
sudo pihole -g            # regenera la lista de bloqueo
sudo systemctl restart pihole-FTL
```

La integridad se verifica con:

```bash
python3 -c "import sqlite3; db=sqlite3.connect('/etc/pihole/gravity.db'); print(db.execute('PRAGMA integrity_check').fetchone())"
```
