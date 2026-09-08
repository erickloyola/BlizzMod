#!/usr/bin/env bash
set -e

MSG="${1:-Atualizacao de mod}"
REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$REPO_DIR"

GAME_DLL="$HOME/.local/share/Steam/steamapps/common/Marvel Contest of Champions/version.dll"

echo "=========================================="
echo "  BlizzMod - Compilação e Deploy Rápido   "
echo "=========================================="

# 1. Verificar se há alterações
if [ -z "$(git status --porcelain)" ]; then
    echo "Nenhuma alteração detectada no código."
    read -p "Deseja forçar a recompilação mesmo assim? (s/N) " force
    if [[ ! "$force" =~ ^[sSyY]$ ]]; then
        exit 0
    fi
    git commit --allow-empty -m "$MSG"
else
    echo "Salvando alterações no Git..."
    git add -A
    git commit -m "$MSG"
fi

# 2. Enviar para o GitHub
echo "Enviando para o repositório..."
git push origin master

# 3. Obter ID da compilação iniciada
echo "Aguardando início do workflow no GitHub Actions..."
sleep 6

RUN_ID=$(gh run list -R erickloyola/BlizzMod --limit 1 --json databaseId -q '.[0].databaseId')
echo "Compilação iniciada (Run ID: $RUN_ID)."
echo "Acompanhando progresso..."

# 4. Acompanhar a compilação até o fim
gh run watch "$RUN_ID" -R erickloyola/BlizzMod --exit-status

# 5. Baixar o binário gerado
echo "Compilação finalizada com sucesso! Baixando nova DLL..."
TMP_DIR=$(mktemp -d)
gh run download "$RUN_ID" -R erickloyola/BlizzMod --dir "$TMP_DIR"

# 6. Atualizar a DLL no jogo automaticamente
if [ -f "$TMP_DIR/BlizzMod-Release-x64/version.dll" ]; then
    cp -vf "$TMP_DIR/BlizzMod-Release-x64/version.dll" "$GAME_DLL"
    echo "=========================================="
    echo " Sucesso! Nova version.dll instalada no jogo!"
    echo " SHA256: $(sha256sum "$GAME_DLL" | awk '{print $1}')"
    echo "=========================================="
else
    echo "Erro: version.dll não encontrada no pacote baixado."
fi

# Limpeza
rm -rf "$TMP_DIR"
