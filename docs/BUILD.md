# BUILD.md — Compilación

## Requisitos

- `g++` con soporte para C++11 (probado con g++ 10.2.1 en Raspberry Pi OS)
- `make`
- `python3` (para el servidor Python, no requiere compilación)
- Sistema Linux (usa sockets POSIX y `/proc`)

Verificar:

```bash
g++ --version
make --version
python3 --version
```

## Compilar

```bash
make cleanall && make
```

Resultado:

- Objetos en `server/cpp/obj/`
- Binario en `server/cpp/bin/web_server`

## Flags de compilación

Definidas en `server/cpp/Makefile`:

```make
CXXFLAGS = -std=c++11 -O2 -Wall -Wextra -Iinclude
```

- `-O2` : optimización
- `-Wall -Wextra` : advertencias completas
- `-Iinclude` : búsqueda de cabeceras

## Objectivos de make

| Target | Descripción |
|---|---|
| `all` | Compila el servidor C++ (por defecto) |
| `cpp` | Igual que `all` (delega en `server/cpp/Makefile`) |
| `run` | Compila y ejecuta (`PORT=...` opcional) |
| `start` | Ejecuta en segundo plano (démón) |
| `stop` | Detiene el servidor C++ |
| `py-run` / `py-start` / `py-stop` | Servidor Python |
| `clean` | Elimina `server/cpp/obj/` |
| `cleanall` | Elimina `server/cpp/obj/` y `server/cpp/bin/` |
