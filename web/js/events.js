(function () {
    var term = document.getElementById("term");
    var dot = document.getElementById("status-dot");
    var statusText = document.getElementById("status-text");
    var btnToggle = document.getElementById("btn-toggle");
    var btnClear = document.getElementById("btn-clear");
    var autoScroll = document.getElementById("autoscroll");
    var countEl = document.getElementById("count");

    var es = null;
    var total = 0;

    function colorFor(msg) {
        if (msg.indexOf("[SEGURIDAD]") === 0) {
            return msg.indexOf("OK") !== -1 ? "var(--ok)" : "var(--err)";
        }
        if (msg.indexOf("[PEER]") === 0) return "var(--accent)";
        if (msg.indexOf("[OLED]") === 0) return "var(--warn)";
        if (msg.indexOf("[MONITOR]") === 0) return "var(--muted)";
        if (msg.indexOf("[SERVIDOR]") === 0) return "var(--ok)";
        return "var(--text)";
    }

    function appendLine(t, msg) {
        var line = document.createElement("div");
        line.className = "line";
        var time = document.createElement("span");
        time.className = "time";
        time.textContent = t;
        var text = document.createElement("span");
        text.className = "msg";
        text.style.color = colorFor(msg);
        text.textContent = msg;
        line.appendChild(time);
        line.appendChild(text);
        term.appendChild(line);
        total++;
        countEl.textContent = total + " eventos";
        if (autoScroll.checked) term.scrollTop = term.scrollHeight;
    }

    function setStatus(state, txt) {
        dot.className = "dot" + (state === true ? " ok" : state === false ? " err" : "");
        statusText.textContent = txt;
    }

    function stop() {
        if (es) {
            es.close();
            es = null;
        }
        setStatus(null, "sin conexi\u00f3n");
        btnToggle.textContent = "Iniciar monitoreo";
        btnToggle.classList.remove("ghost");
    }

    function start() {
        setStatus(null, "conectando...");
        es = new EventSource("/api/events");
        es.onopen = function () {
            setStatus(true, "en vivo");
            btnToggle.textContent = "Detener monitoreo";
            btnToggle.classList.add("ghost");
        };
        es.onmessage = function (ev) {
            try {
                var data = JSON.parse(ev.data);
                appendLine(data.t || "??:??:??", data.msg || "");
            } catch (e) {
                appendLine("", ev.data);
            }
        };
        es.onerror = function () {
            setStatus(false, "reconectando...");
        };
    }

    btnToggle.addEventListener("click", function () {
        if (es) stop();
        else start();
    });

    btnClear.addEventListener("click", function () {
        term.innerHTML = "";
        var p = document.createElement("span");
        p.className = "prompt";
        p.textContent = "$ ";
        term.appendChild(p);
        total = 0;
        countEl.textContent = "0 eventos";
    });

    document.getElementById("autoscroll").addEventListener("change", function () {
        if (this.checked) term.scrollTop = term.scrollHeight;
    });
})();
