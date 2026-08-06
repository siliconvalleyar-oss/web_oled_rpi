CXX      = g++
CXXFLAGS = -std=c++11 -O2 -Wall -Wextra -Iinclude

ARCH     := $(shell uname -m)
ifneq ($(strip $(findstring arm,$(ARCH))$(findstring aarch64,$(ARCH))),)
LDFLAGS  = -lSSD1306_OLED_RPI -lbcm2835
else
LDFLAGS  =
endif

VERSION  := $(shell cat ../../VERSION 2>/dev/null || echo 0.0.0)
TARGET   = web_server_v$(VERSION)
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

run: all
	exec ./$(BIN) $(PORT) $(DOC_ROOT)

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

.PHONY: all dirs run start stop restart clean cleanall
