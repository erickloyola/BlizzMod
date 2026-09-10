# BlizzMod - Mod Menu & Modloader para Marvel Contest of Champions

Repositório mantido por [erickloyola](https://github.com/erickloyola/BlizzMod) com suporte total a Linux (Proton/Steam) e Windows, renderização em Direct3D 11, interface gráfica ImGui acionada por **F12**, hooks dinâmicos de IL2CPP e combate 100% estável.

---

##  Status da Versão
* **Versão Estável**: [`v1.0-stable`](https://github.com/erickloyola/BlizzMod/releases/tag/v1.0-stable) (**100% Funcional**)
* **Backend Gráfico**: Direct3D 11 com ImGui
* **Tecla de Atalho**: `F12` (Abrir/Fechar Menu)
* **Compatibilidade**: Proton 9 / GE-Proton / Windows 10/11

---

##  Guias Rápidos

* 📖 **[Guia de Instalação Pós-Formatação](./GUIA_INSTALACAO_POS_FORMATACAO.md)**
  *Passo a passo completo de como instalar o mod em um computador formatado ou em uma máquina nova em apenas 3 minutos.*

* ⚙️ **[Guia Completo de Métodos de Compilação](./GUIA_COMPILACAO.md)**
  *Como compilar usando o script automatizado `./atualizar.sh`, Docker local com MSVC, MinGW-w64 ou Visual Studio nativo.*

---

##  Como Jogar (Instalação Rápida no Linux)

1. Nas propriedades do jogo na Steam, adicione em **Opções de Inicialização**:
   ```bash
   WINEDLLOVERRIDES="version=n,b" %command% -force-d3d11
   ```
2. Baixe a DLL estável com o comando no terminal:
   ```bash
   curl -L -o "$HOME/.local/share/Steam/steamapps/common/Marvel Contest of Champions/version.dll" https://github.com/erickloyola/BlizzMod/releases/download/v1.0-stable/version.dll
   ```
3. Inicie o jogo pela Steam e aperte **F12**!

---

##  Como Atualizar / Modificar o Código

Após fazer alterações nos arquivos de código C++:
```bash
./atualizar.sh "minhas alterações"
```
O script fará o commit, envio para o GitHub, compilação em nuvem multi-core e substituição automática do arquivo na pasta do jogo.

---

##  Notas sobre Easy Anti-Cheat (EAC/EOS)
Caso necessite contornar validações do EOS, configure a variável de ambiente:
```bash
EOS_USE_ANTICHEATCLIENTNULL=1
```
*(No Linux, você pode adicionar diretamente antes do comando na Steam: `EOS_USE_ANTICHEATCLIENTNULL=1 WINEDLLOVERRIDES="version=n,b" %command% -force-d3d11`).*
