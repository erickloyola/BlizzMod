#!/usr/bin/env bash
set -e

MSG="${1:-Versao estavel com novas modificacoes testadas}"
REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$REPO_DIR"

echo "=========================================================="
echo "      BlizzMod - Publicar Alterações na Master Oficial    "
echo "=========================================================="

CURRENT_BRANCH=$(git branch --show-current)
if [ "$CURRENT_BRANCH" != "testes" ]; then
    echo "Você não está na branch 'testes'. Alternando para 'testes' primeiro..."
    git checkout testes
fi

echo "Mesclando as alterações aprovadas na branch principal 'master'..."
git checkout master
git merge testes -m "$MSG"
git push origin master

echo "Voltando para a branch 'testes' para você continuar seus testes com segurança..."
git checkout testes

echo "=========================================================="
echo " ✅ Publicado com sucesso na branch 'master' do GitHub!"
echo "=========================================================="
