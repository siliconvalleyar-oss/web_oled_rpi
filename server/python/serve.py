#!/usr/bin/env python3
"""
Servidor web en Python para el dashboard de la Raspberry Pi.

Sirve los archivos estaticos de web/ y expone una API JSON:
    GET /             -> index.html
    GET /api/status   -> informacion del sistema
    GET /api/pihole   -> estado de Pi-hole

Uso:
    python3 serve.py [PUERTO]
"""

import json
import os
import shutil
import socket
import sqlite3
import subprocess
import time
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import unquote

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
WEB_DIR = os.path.join(ROOT, "web")
GRAVITY_DB = "/etc/pihole/gravity.db"
MIME = {
    ".html": "text/html; charset=utf-8",
    ".css": "text/css; charset=utf-8",
    ".js": "application/javascript",
    ".json": "application/json",
    ".svg": "image/svg+xml",
    ".png": "image/png",
    ".ico": "image/x-icon",
    ".md": "text/plain; charset=utf-8",
}


def read_first(path, default=""):
    try:
        with open(path) as f:
            return f.read().strip()
    except OSError:
        return default


def local_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        return s.getsockname()[0]
    except OSError:
        return None
    finally:
        s.close()


def meminfo():
    mem = {}
    for line in read_first("/proc/meminfo").splitlines():
        parts = line.split()
        if len(parts) >= 2:
            mem[parts[0].rstrip(":")] = int(parts[1]) * 1024
    return {
        "total": mem.get("MemTotal"),
        "free": mem.get("MemFree"),
        "available": mem.get("MemAvailable"),
        "used": (mem.get("MemTotal") or 0) - (mem.get("MemAvailable") or 0),
    }


def api_status():
    try:
        with open("/proc/uptime") as f:
            uptime = float(f.read().split()[0])
    except (OSError, IndexError):
        uptime = None

    temp = None
    raw = read_first("/sys/class/thermal/thermal_zone0/temp")
    if raw.isdigit():
        temp = int(raw) // 1000

    disk = shutil.disk_usage("/")
    model = read_first("/proc/cpuinfo", "")
    for line in model.splitlines():
        if line.startswith("Model"):
            model = line.split(":", 1)[1].strip()
            break
    else:
        model = os.uname().machine

    return {
        "hostname": socket.gethostname(),
        "ip": local_ip(),
        "uptime_seconds": uptime,
        "loadavg": list(os.getloadavg()),
        "cores": os.cpu_count(),
        "cpu_model": model,
        "temp_c": temp,
        "time": datetime.now(timezone.utc).isoformat(),
        "memory": meminfo(),
        "disk": {
            "total": disk.total,
            "used": disk.used,
            "free": disk.free,
            "available": disk.free,
        },
    }


def run_pihole_status():
    try:
        out = subprocess.run(
            ["sudo", "-n", "pihole", "status"],
            capture_output=True,
            text=True,
            timeout=15,
        )
        return out.stdout + out.stderr
    except (OSError, subprocess.SubprocessError):
        return ""


def api_pihole():
    installed = shutil.which("pihole") is not None
    result = {
        "installed": installed,
        "blocking_enabled": None,
        "domains_blocked": None,
        "updated": None,
    }
    if not installed:
        return result

    status = run_pihole_status()
    result["blocking_enabled"] = "blocking is enabled" in status.lower()

    if os.path.exists(GRAVITY_DB):
        try:
            db = sqlite3.connect(GRAVITY_DB)
            row = db.execute("SELECT COUNT(*) FROM gravity").fetchone()
            result["domains_blocked"] = row[0] if row else 0
            db.close()
        except sqlite3.Error:
            pass

        updated_ts = os.path.getmtime(GRAVITY_DB)
        result["updated"] = datetime.fromtimestamp(updated_ts).strftime("%Y-%m-%d %H:%M:%S")

    return result


class Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):
        print("[%s] %s" % (self.client_address[0], fmt % args), flush=True)

    def send_json(self, obj, code=200):
        body = json.dumps(obj, indent=2).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def send_file(self, path):
        if not os.path.exists(path) or not os.path.isfile(path):
            return self.send_error(404, "Not Found")
        try:
            with open(path, "rb") as f:
                body = f.read()
        except OSError:
            return self.send_error(500, "Read error")
        ext = os.path.splitext(path)[1].lower()
        self.send_response(200)
        self.send_header("Content-Type", MIME.get(ext, "application/octet-stream"))
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        path = unquote(self.path.split("?", 1)[0])

        if path == "/api/status":
            return self.send_json(api_status())
        if path == "/api/pihole":
            return self.send_json(api_pihole())

        if path == "/":
            path = "/index.html"
        if path.startswith("/"):
            path = path[1:]

        rel = os.path.normpath(path)
        if rel.startswith(".."):
            return self.send_error(403, "Forbidden")
        self.send_file(os.path.join(WEB_DIR, rel))

    do_HEAD = do_GET


def main():
    import sys
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
    server = ThreadingHTTPServer(("0.0.0.0", port), Handler)
    print("Servidor Python escuchando en 0.0.0.0:%d (web_dir=%s)" % (port, WEB_DIR), flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("Detenido", flush=True)


if __name__ == "__main__":
    main()
