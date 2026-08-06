async function loadSystem() {
    const data = await fetchJSON("/api/status");
    el("server-dot").className = "dot ok";
    el("server-name").textContent = data.hostname;
    el("sys-hostname").textContent = data.hostname;
    el("sys-ip").textContent = data.ip || "-";
    el("sys-uptime").textContent = fmtUptime(data.uptime_seconds);
    el("sys-load").textContent = Array.isArray(data.loadavg)
        ? data.loadavg.map((x) => x.toFixed(2)).join(" / ")
        : data.loadavg;
    el("sys-cpu").textContent = data.cpu_model || data.cores + " núcleos";
    el("sys-temp").textContent = data.temp_c !== null ? data.temp_c + " °C" : "-";

    el("mem-total").textContent = fmtBytes(data.memory.total);
    el("mem-used").textContent = fmtBytes(data.memory.used);
    el("mem-avail").textContent = fmtBytes(data.memory.available);
    const memPct = data.memory.total ? (data.memory.used / data.memory.total) * 100 : 0;
    el("mem-fill").style.width = memPct.toFixed(1) + "%";

    el("disk-total").textContent = fmtBytes(data.disk.total);
    el("disk-used").textContent = fmtBytes(data.disk.used);
    el("disk-avail").textContent = fmtBytes(data.disk.available);
    const diskPct = data.disk.total ? (data.disk.used / data.disk.total) * 100 : 0;
    el("disk-fill").style.width = diskPct.toFixed(1) + "%";
}

async function loadPihole() {
    let data;
    try {
        data = await fetchJSON("/api/pihole");
    } catch (err) {
        const box = el("pihole-status");
        box.textContent = "Pi-hole no disponible desde este servidor";
        box.className = "status-line warn";
        return;
    }
    const box = el("pihole-status");
    if (data.installed && data.blocking_enabled) {
        box.textContent = "Pi-hole activo y bloqueando";
        box.className = "status-line ok";
    } else if (data.installed) {
        box.textContent = "Pi-hole instalado pero bloqueo desactivado";
        box.className = "status-line warn";
    } else {
        box.textContent = "Pi-hole no instalado";
        box.className = "status-line err";
    }
    el("pihole-domains").textContent = data.domains_blocked;
    el("pihole-updated").textContent = data.updated || "-";
}

async function refresh() {
    const now = new Date().toLocaleString();
    try {
        await Promise.all([loadSystem(), loadPihole()]);
        el("updated-at").textContent = now;
    } catch (err) {
        el("server-dot").className = "dot err";
        el("server-name").textContent = "error: " + err.message;
        el("updated-at").textContent = now;
    }
}

refresh();
setInterval(refresh, 10000);
