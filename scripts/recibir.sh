#!/usr/bin/env bash
# Recibe y despliega en la Raspberry: pull, build y run con display OLED.
# Uso: ./scripts/recibir.sh
set -euo pipefail

BRANCH="recepcion-rpi"
REMOTE="origin"

if [ "$(git rev-parse --abbrev-ref HEAD)" != "$BRANCH" ]; then
    git fetch "$REMOTE"
    git checkout "$BRANCH" 2>/dev/null || git checkout -b "$BRANCH" --track "$REMOTE/$BRANCH"
fi

git pull --ff-only "$REMOTE" "$BRANCH"

make clean
make -j4
sudo make run
