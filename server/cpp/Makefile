CXX      = g++
CXXFLAGS = -std=c++11 -O2 -Wall -Wextra -Iinclude

ARCH     := $(shell uname -m)
ifneq ($(strip $(findstring arm,$(ARCH))$(findstring aarch64,$(ARCH))),)
LDFLAGS  = -lSSD1306_OLED_RPI -lbcm2835 -lcrypto
ARM_TARGET = 1
else
LDFLAGS  = -lcrypto
endif

SEND_HOST ?= rpi2w.local
SEND_PORT ?= 8081

VERSION  := $(shell cat ../../VERSION 2>/dev/null || echo 0.0.0)
BRANCH   := $(shell git rev-parse --abbrev-ref HEAD 2>/dev/null || echo desconocida)
TARGET   = web_server_$(BRANCH)_v$(VERSION)
BINDIR   = bin
OBJDIR   = obj
SRCDIR   = src
INCDIR   = include
LOGDIR   = ../../var/logs
LOG      = $(LOGDIR)/server.log
PIDFILE  = $(LOGDIR)/server.pid
PORT    ?= 8081
DOC_ROOT = ../../web

BIN  = $(BINDIR)/$(TARGET)
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

all: dirs $(BIN)

dirs:
	@mkdir -p $(BINDIR) $(OBJDIR)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

ifdef ARM_TARGET
RUN_CMD = ./$(BIN) $(PORT) $(DOC_ROOT)
else
RUN_CMD = ./$(BIN) send $(SEND_HOST) $(SEND_PORT)
endif

ifdef ARM_TARGET
run: all
	@if [ "$$(id -u)" -ne 0 ]; then \
		echo "Se requiere root para el OLED; relanzando con sudo..."; \
		exec sudo -E $(MAKE) -s run; \
	fi
	fuser -k $(PORT)/tcp 2>/dev/null || true; \
	sleep 1; \
	exec $(RUN_CMD)
else
run: all
	exec $(RUN_CMD)
endif

server: all
	exec ./$(BIN) $(PORT) $(DOC_ROOT)

send: all
	exec ./$(BIN) send $(SEND_HOST) $(SEND_PORT)

start: all dirs
	@mkdir -p $(LOGDIR); nohup ./$(BIN) $(PORT) $(DOC_ROOT) > $(LOG) 2>&1 & echo $$! > $(PIDFILE); sleep 0.5; echo "Servidor iniciado en puerto $(PORT), log en $(LOG)"; cat $(LOG)

stop:
	@if [ -f $(PIDFILE) ]; then \
	  kill $$(cat $(PIDFILE)) 2>/dev/null || true; \
	  rm -f $(PIDFILE); \
	  echo "Servidor detenido"; \
	else \
	  pkill -x $(TARGET) 2>/dev/null || echo "El servidor no estaba corriendo"; \
	fi

restart: stop start

clean:
	rm -rf $(OBJDIR)

cleanall: clean
	rm -rf $(BINDIR)

.PHONY: all dirs run server send start stop restart clean cleanall
