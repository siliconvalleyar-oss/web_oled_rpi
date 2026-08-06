PY       = python3
PY_SRV   = server/python/serve.py
PY_PORT ?= 8000
PY_LOG   = var/logs/python.log

all: cpp

cpp:
	$(MAKE) -C server/cpp

run:
	$(MAKE) -C server/cpp run

send:
	$(MAKE) -C server/cpp send

start:
	$(MAKE) -C server/cpp start

stop:
	$(MAKE) -C server/cpp stop

restart: stop start

py-run:
	$(PY) $(PY_SRV) $(PY_PORT)

py-start:
	@mkdir -p var/logs
	nohup $(PY) $(PY_SRV) $(PY_PORT) > $(PY_LOG) 2>&1 &
	@sleep 0.5
	@echo "Servidor Python iniciado en puerto $(PY_PORT), log en $(PY_LOG)"

py-stop:
	@pgrep -f "[s]erve.py" | xargs -r kill || echo "El servidor Python no estaba corriendo"

clean:
	$(MAKE) -C server/cpp clean

cleanall:
	$(MAKE) -C server/cpp cleanall

.PHONY: all cpp run send start stop restart py-run py-start py-stop clean cleanall
