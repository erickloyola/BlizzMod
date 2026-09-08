#!/usr/bin/env bash
set -e

MSG="${1:-Teste de modificação local - $(date '+%Y-%m-%d %H:%M:%S')}"
REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$REPO_DIR"

GAME_DLL="$HOME/.local/share/Steam/steamapps/common/Marvel Contest of Champions/version.dll"

echo "=========================================================="
echo "    BlizzMod - Compilação de Testes (Branch Isolada)     "
echo "=========================================================="

# 1. Garantir que estamos na branch de testes 'testes'
CURRENT_BRANCH=$(git branch --show-current)
if [ "$CURRENT_BRANCH" != "testes" ]; then
    echo "Alternando para a branch de testes isolada ('testes')..."
    git checkout -B testes
fi

# 2. Verificar e registrar alterações
if [ -z "$(git status --porcelain)" ]; then
    echo "Nenhuma alteração de código detectada."
    read -p "Deseja forçar a recompilação da branch de testes mesmo assim? (s/N) " force
    if [[ ! "$force" =~ ^[sSyY]$ ]]; then
        exit 0
    fi
    git commit --allow-empty -m "$MSG"
else
    echo "Registrando alterações de teste..."
    git add -A
    git commit -m "$MSG"
fi

# 3. Enviar para o GitHub APENAS na branch de testes (a master permanece 100% intacta)
echo "Enviando alterações exclusivamente para a branch 'testes' no GitHub..."
git push -f origin testes

# 4. Obter ID da compilação iniciada para a branch 'testes'
echo "Aguardando GitHub Actions iniciar a compilação da branch 'testes'..."
sleep 6

RUN_ID=""
for i in {1..10}; do
    RUN_ID=$(gh run list --branch testes -R erickloyola/BlizzMod --limit 1 --json databaseId -q '.[0].databaseId' 2>/dev/null || true)
    if [ -n "$RUN_ID" ]; then
        break
    fi
    sleep 2
done

if [ -z "$RUN_ID" ]; then
    echo "Erro: Não foi possível localizar o workflow da branch 'testes'."
    exit 1
fi

echo "Compilação de teste iniciada (Run ID: $RUN_ID)."
echo "Acompanhando progresso..."

# 5. Acompanhar a compilação até o fim
gh run watch "$RUN_ID" -R erickloyola/BlizzMod --exit-status

# 6. Baixar a DLL gerada
echo "Compilação concluída com sucesso! Baixando nova version.dll..."
TMP_DIR=$(mktemp -d)
gh run download "$RUN_ID" -R erickloyola/BlizzMod --dir "$TMP_DIR"

# 7. Instalar a DLL no jogo automaticamente
if [ -f "$TMP_DIR/BlizzMod-Release-x64/version.dll" ]; then
    cp -vf "$TMP_DIR/BlizzMod-Release-x64/version.dll" "$GAME_DLL"
    echo "=========================================================="
    echo " ✅ Sucesso! Nova version.dll de teste instalada no jogo!"
    echo " 🔒 A branch 'master' permanece 100% limpa e inalterada."
    echo " 🔑 SHA256: $(sha256sum "$GAME_DLL" | awk '{print $1}')"
    echo "=========================================================="
else
    echo "Erro: version.dll não encontrada no artefato baixado."
fi

# Limpeza
rm -rf "$TMP_DIR"
