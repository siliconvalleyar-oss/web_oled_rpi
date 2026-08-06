#!/usr/bin/env bash
# Envia los cambios desde la PC: bump de VERSION, commit, tag, push a main
# y sincroniza la rama de la Raspberry con el nuevo codigo.
# Uso: ./scripts/enviar.sh [mensaje de commit]
set -euo pipefail

BRANCH="envio-pc"
RECV="recepcion-rpi"
REMOTE="origin"

if [ "$(git rev-parse --abbrev-ref HEAD)" != "$BRANCH" ]; then
    echo "Este script debe ejecutarse en la rama $BRANCH. Actual: $(git rev-parse --abbrev-ref HEAD)"
    exit 1
fi

if [ -z "$(git status --porcelain)" ]; then
    echo "No hay cambios pendientes que enviar."
    exit 0
fi

MSG="${1:-Actualizacion desde PC}"

next_version() {
    local tag="${1:-v0.0.0}" x y z
    tag="${tag#v}"
    IFS=. read -r x y z <<<"$tag"
    if [ "$z" -eq 9 ]; then
        if [ "$y" -eq 9 ]; then x=$((x + 1)); y=0; else y=$((y + 1)); fi
        z=0
    else
        z=$((z + 1))
    fi
    echo "$x.$y.$z"
}

trap 'git checkout "$BRANCH" 2>/dev/null || true' EXIT

NEXT="$(next_version "$(git describe --tags --abbrev=0 2>/dev/null || echo v0.0.0)")"

echo "$NEXT" > VERSION
git add -A
git commit -m "${MSG}"
git tag "v${NEXT}"

git push "$REMOTE" "$BRANCH:main" --tags
git fetch "$REMOTE"
git branch -f main "$REMOTE/main"

git checkout "$RECV" 2>/dev/null || git checkout -b "$RECV" --track "$REMOTE/$RECV"
git merge --no-edit "$REMOTE/main"
git push "$REMOTE" "$RECV"

echo "Enviado v${NEXT} a main y sincronizada $RECV."
