# Guia Completo de Métodos de Compilação do BlizzMod

Este documento reúne todos os métodos para compilar o BlizzMod, desde a automação em 1 comando até opções de compilação 100% locais sem depender do GitHub.

---

## 📌 Contexto Técnico
O BlizzMod é compilado como uma DLL de 64 bits do Windows (`version.dll` / `BlizzMod.dll`) que atua como proxy DLL para o jogo *Marvel Contest of Champions*. Ele depende de:
- **Microsoft Detours** (para interceptação de funções x64 em memória).
- **DirectX 11** (`d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib`) para renderização do menu ImGui.
- **Unity IL2CPP** (`appdata/il2cpp-types.h` com mais de 100 MB e 2,7 milhões de linhas de declarações C++).
- **Tratamento de Exceções Estruturadas (SEH)** (`__try / __except`) para estabilidade.

---

## 🚀 Método 1: Automação Total com o Script `atualizar.sh` (Recomendado no Linux)

Este método utiliza o poder dos servidores Windows do GitHub Actions através do script local [`atualizar.sh`](./atualizar.sh). Você não precisa abrir o navegador nem copiar arquivos manualmente.

### Como usar:
1. Faça as modificações desejadas nos arquivos C++ (ex: em [`libraries/pipeline/hooks/InitHooks.cpp`](./libraries/pipeline/hooks/InitHooks.cpp)).
2. No terminal do seu computador, execute:
   ```bash
   cd ~/BlizzMod
   ./atualizar.sh "descrição das minhas alterações"
   ```

### O que o script faz automaticamente:
1. Salva suas alterações no Git local (`git add` e `git commit`).
2. Envia para o seu repositório no GitHub (`git push origin master`).
3. O GitHub Actions inicia a compilação com **MSBuild multi-núcleos (`-m`)** em uma máquina Windows dedicada.
4. O script acompanha o progresso no seu terminal em tempo real.
5. Ao concluir, **baixa automaticamente** a nova `version.dll`.
6. Substitui o arquivo diretamente na pasta do jogo:
   `~/.local/share/Steam/steamapps/common/Marvel Contest of Champions/version.dll`
7. Exibe o hash SHA256 de confirmação.

---

## 🐳 Método 2: Compilação Local com Docker e MSVC (Sem GitHub)

Se você quiser compilar **100% no seu processador Linux** sem enviar nada para o GitHub e com o compilador oficial da Microsoft (`cl.exe` / `msbuild` rodando via Wine dentro de um container isolado):

### 1. Instalar o Docker no Arch Linux:
```bash
sudo pacman -S docker
sudo systemctl enable --now docker
sudo usermod -aG docker $USER
```
*(Reinicie a sessão ou dê logout/login para aplicar o grupo docker).*

### 2. Baixar uma imagem com o compilador MSVC para Wine:
Existem imagens prontas mantidas pela comunidade para compilar projetos Visual Studio no Linux, por exemplo a `dockcross/windows-static-x64` ou imagens com o MSVC Build Tools instalado via Wine.

### 3. Executar o Build dentro da pasta do projeto:
```bash
docker run --rm -v "$(pwd)":/work -w /work <imagem-msvc> msbuild BlizzMod.sln /p:Configuration=Release /p:Platform=x64 -m
```
A DLL gerada sairá diretamente na sua pasta `x64/Release/BlizzMod.dll`.

---

## 🐧 Método 3: Compilação Cruzada Local com MinGW-w64 (`mingw-w64-gcc`)

O MinGW-w64 é o compilador GNU para Linux capaz de gerar executáveis e DLLs do Windows sem precisar de Wine.

### 1. Instalar o MinGW no Arch Linux:
```bash
sudo pacman -S mingw-w64-gcc mingw-w64-binutils mingw-w64-headers
```

### 2. O que é necessário para adaptar o projeto para o MinGW:
Como o BlizzMod original usa arquivos de projeto do Visual Studio (`.sln` e `.vcxproj`), para compilar com `x86_64-w64-mingw32-g++` é necessário:
1. **Criar um `CMakeLists.txt`** listando todos os arquivos de código-fonte (`.cpp`).
2. **Flags de Linkagem**: Linkar contra as bibliotecas do Windows:
   `-ld3d11 -ldxgi -ld3dcompiler -lkernel32 -luser32 -lgdi32`
3. **Definições de Exportação**: Passar o arquivo `version.def` através da flag `-Wl,--def,version.def`.
4. **Substituir SEH específico do MSVC**: O GCC/MinGW não suporta `__try / __except` do MSVC por padrão; é necessário utilizar as extensões de SEH do MinGW ou compilar com Clang targeting `x86_64-w64-windows-gnu`.

---

## 🪟 Método 4: Compilação Nativa no Windows ou Máquina Virtual

Se você possuir uma partição Windows (Dual Boot) ou uma máquina virtual Windows:

1. **Instale o Visual Studio Community (Gratuito)**:
   - Durante a instalação, selecione a carga de trabalho **"Desenvolvimento para desktop com C++"**.
2. **Abra o Projeto**:
   - Dê dois cliques no arquivo [`BlizzMod.sln`](./BlizzMod.sln).
3. **Selecione a Configuração**:
   - Na barra superior, mude para **Release** e **x64**.
4. **Compilar**:
   - Pressione **Ctrl + Shift + B** (ou clique em *Compilar* -> *Compilar Solução*).
   - O binário compilado estará em: `x64\Release\BlizzMod.dll`.
   - Renomeie para `version.dll` e coloque na pasta do jogo.
   - Tempo médio de compilação: **15 a 30 segundos**.

---

## 📊 Comparativo dos Métodos

| Método | Ambiente | Tempo de Build | Complexidade | Necessita Internet? |
| :--- | :--- | :--- | :--- | :--- |
| **`./atualizar.sh`** | Linux (Arch) | ~3-4 minutos | Nenhuma (1 comando) | Sim (GitHub) |
| **Docker + MSVC** | Linux (Arch) | ~30 segundos | Média (configurar container) | Não (apenas para baixar a imagem) |
| **MinGW-w64** | Linux (Arch) | ~15 segundos | Alta (adaptar headers/MSVC) | Não |
| **Visual Studio** | Windows / VM | ~15 segundos | Baixa (abrir e apertar F7) | Não |
