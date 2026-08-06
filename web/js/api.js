async function fetchJSON(url) {
    const res = await fetch(url);
    if (!res.ok) throw new Error(url + " -> " + res.status);
    return res.json();
}

function fmtBytes(bytes) {
    if (bytes === null || bytes === undefined) return "-";
    const units = ["B", "KB", "MB", "GB", "TB"];
    let i = 0;
    let v = bytes;
    while (v >= 1024 && i < units.length - 1) {
        v /= 1024;
        i++;
    }
    return v.toFixed(i === 0 ? 0 : 1) + " " + units[i];
}

function fmtUptime(seconds) {
    if (!seconds) return "-";
    const d = Math.floor(seconds / 86400);
    const h = Math.floor((seconds % 86400) / 3600);
    const m = Math.floor((seconds % 3600) / 60);
    if (d > 0) return d + "d " + h + "h " + m + "m";
    if (h > 0) return h + "h " + m + "m";
    return m + "m";
}

function el(id) {
    return document.getElementById(id);
}
