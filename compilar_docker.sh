#!/usr/bin/env bash
set -e

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$REPO_DIR"

GAME_DLL="$HOME/.local/share/Steam/steamapps/common/Marvel Contest of Champions/version.dll"

echo "=========================================================="
echo "    BlizzMod - Compilação Local com Docker (Wine + MSVC)  "
echo "=========================================================="
echo " [Segurança Ativa]"
echo "  - Limite de RAM no container: 4.5 GB"
echo "  - Limite de CPU: 2 núcleos (deixando o PC livre)"
echo "  - Modo sequencial: Ninja -j 1 (evita estouro de memória)"
echo "=========================================================="

docker run --rm \
    --memory=4.5g \
    --memory-swap=4.5g \
    --cpus=2 \
    -v "$REPO_DIR:/work" \
    -w /work \
    madduci/docker-wine-msvc:17.8-2022 \
    cmd /c "cd /d Z:\work && cmake -B build_docker -G Ninja -DCMAKE_BUILD_TYPE=Release && ninja -j 1 -C build_docker"

BUILT_DLL="$REPO_DIR/build_docker/version.dll"

if [ -f "$BUILT_DLL" ]; then
    echo ""
    echo "=========================================================="
    echo " ✅ Compilação concluída com sucesso!"
    echo " 📦 DLL gerada: $BUILT_DLL"
    echo " 🔑 SHA256: $(sha256sum "$BUILT_DLL" | awk '{print $1}')"

    if [ -d "$(dirname "$GAME_DLL")" ]; then
        cp -vf "$BUILT_DLL" "$GAME_DLL"
        echo " 🎮 DLL copiada automaticamente para a pasta do jogo!"
    fi
    echo "=========================================================="
else
    echo "❌ Erro: version.dll não foi encontrada em $BUILT_DLL"
    exit 1
fi
