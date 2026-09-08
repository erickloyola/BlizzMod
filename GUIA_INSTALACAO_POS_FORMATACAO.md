# Guia de Instalação e Restauração Pós-Formatação

Este guia passo a passo ensina como reinstalar e reativar o **BlizzMod 100% Funcional** caso você formate o computador ou queira utilizá-lo em uma nova máquina.

---

## ⚡ Método 1: Instalação Rápida (Apenas para Jogar - 3 Minutos)

Se você acabou de instalar o sistema operacional e quer apenas jogar com o mod funcionando perfeitamente sem precisar configurar ambiente de programação:

### 1. Instale o Jogo na Steam
- Baixe e instale o **Marvel Contest of Champions** pela sua biblioteca Steam normalmente.

---

### 2. Configure os Parâmetros de Inicialização na Steam
1. Abra a Steam, acesse a sua **Biblioteca**.
2. Clique com o **botão direito** no jogo **Marvel Contest of Champions** e selecione **Propriedades...** (Properties).
3. Na aba **Geral** (General), role até o campo **Opções de Inicialização** (Launch Options).
4. Digite ou cole exatamente:
   ```bash
   WINEDLLOVERRIDES="version=n,b" %command% -force-d3d11
   ```

#### 🔍 O que esses parâmetros fazem:
* `WINEDLLOVERRIDES="version=n,b"`: Obriga o Proton a carregar a nossa `version.dll` modificada presente na pasta do jogo em vez da DLL nativa do Wine/sistema.
* `-force-d3d11`: Força a engine Unity a inicializar no modo **DirectX 11** (o jogo por padrão tenta iniciar em DirectX 12 no Proton com placas NVIDIA modernas, o que impede o ImGui de renderizar).

---

### 3. Baixe e Instale a DLL Funcional

Você pode fazer isso de duas formas:

#### Opção A: Em 1 Comando pelo Terminal (Linux)
Abra o seu terminal e cole o comando abaixo:
```bash
curl -L -o "$HOME/.local/share/Steam/steamapps/common/Marvel Contest of Champions/version.dll" https://github.com/erickloyola/BlizzMod/releases/download/v1.0-stable/version.dll
```

#### Opção B: Pelo Navegador
1. Acesse a página da Release funcional no GitHub:
   👉 **[GitHub Releases - BlizzMod v1.0-stable](https://github.com/erickloyola/BlizzMod/releases/tag/v1.0-stable)**
2. Baixe o arquivo **`version.dll`**.
3. Copie o arquivo baixado e cole dentro do diretório de instalação do jogo:
   * **No Linux (Steam padrão)**:
     `~/.local/share/Steam/steamapps/common/Marvel Contest of Champions/`
   * **No Windows (se instalar no Windows)**:
     `C:\Program Files (x86)\Steam\steamapps\common\Marvel Contest of Champions\`

---

### 4. Iniciar e Jogar
1. Clique em **Jogar** na Steam.
2. O terminal de debug do BlizzMod abrirá exibindo os logs de carregamento do IL2CPP.
3. A janela da interface gráfica do BlizzMod aparecerá na tela.
4. Pressione a tecla **F12** no teclado para exibir ou ocultar o menu quando desejar.
5. Inicie qualquer missão ou luta: a batalha carregará e rodará normalmente sem travamentos!

---

## 🛠️ Método 2: Instalação Completa do Repositório (Para Desenvolver e Modificar)

Se após formatar você também quiser ter o código-fonte na máquina para criar novas funções e modificações:

### 1. Clonar o Repositório
```bash
git clone https://github.com/erickloyola/BlizzMod.git
cd BlizzMod
```

### 2. Acessar a Versão Estável
```bash
git checkout v1.0-stable
```

### 3. Fazer Modificações e Compilar Automaticamente
Toda vez que você alterar o código:
```bash
./atualizar.sh "minhas alterações"
```
O script cuidará do commit, envio para o GitHub, compilação na nuvem e substituição automática do arquivo na pasta do jogo.

---

## 💾 Dica Preventiva: Backup em Pendrive ou Nuvem Pessoal

Se for formatar a máquina, você também pode copiar a pasta criada nesta sessão:
📂 `~/BlizzMod_Backup_Estavel/`
para um pendrive ou Google Drive.

Ao plugar na nova máquina, basta rodar:
```bash
~/BlizzMod_Backup_Estavel/restaurar_dll.sh
```
E o mod já estará instalado e verificado no seu jogo!
