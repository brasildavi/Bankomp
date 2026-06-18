# Bankomp

Bankomp e um sistema bancario em C++ executado no terminal com uma interface visual interativa feita com FTXUI. O programa permite criar contas, autenticar usuarios, realizar depositos, saques, transferencias, consultar extrato e visualizar informacoes do perfil financeiro.

Esta branch `visualFTXUI` representa a versao visual em terminal. Ela usa CMake para compilar o projeto e baixar a biblioteca FTXUI automaticamente na primeira configuracao.

## Branches do projeto

O projeto possui duas branches principais:

- `main`: versao console. Usa menus textuais simples e compila diretamente com `g++`.
- `visualFTXUI`: versao visual em terminal. Usa FTXUI, CMake e uma interface navegavel com telas, abas, campos de entrada e botoes.

Para alternar entre as branches:

```sh
git checkout main
git checkout visualFTXUI
```

Use este README como referencia da branch `visualFTXUI`. Na branch `main`, os comandos e dependencias sao diferentes, pois ela nao utiliza FTXUI nem CMake.

## Funcionalidades

- Tela visual de login.
- Cadastro de nova conta com formulario interativo.
- Validacao de CPF no formato `XXX.XXX.XXX-XX`.
- Validacao matematica dos digitos verificadores do CPF.
- Senha alfanumerica com 8 a 15 caracteres.
- Conta corrente com limite especial.
- Conta poupanca com rendimento.
- Dashboard com abas para Inicio, Deposito, Saque, Transferencia, Extrato e Perfil.
- Botao para ver ou ocultar saldo.
- Deposito, saque e transferencia entre contas.
- Confirmacao visual antes de transferir dinheiro.
- Extrato com data, tipo, valor e detalhes da movimentacao.
- Perfil do cliente com nivel da conta e score interno.
- Analise didatica do score com pontos fortes, melhorias e fatores criticos.
- Persistencia local dos dados em arquivos `.dat`.

## Como o programa funciona

Ao iniciar, o Bankomp procura a variavel de ambiente `BANKOMP_KEY`. Essa chave e obrigatoria, pois o programa a utiliza para ler e gravar os dados locais.

Depois, o sistema carrega os arquivos:

- `banco.dat`: armazena dados das contas, clientes, saldos, tipo de conta e hash da senha.
- `transacoes.dat`: armazena o historico de transacoes.

Se esses arquivos nao existirem, o programa inicia sem contas cadastradas. Eles sao criados automaticamente quando uma conta ou transacao e salva.

O fluxo principal e:

1. Login: entrar em uma conta existente, criar uma nova conta ou sair.
2. Cadastro: preencher dados pessoais, tipo de conta, deposito inicial, renda opcional e senha.
3. Dashboard: navegar pelas abas de operacoes e consultas.
4. Salvamento: operacoes relevantes atualizam os arquivos `.dat`.
5. Logout: retorna para a tela de login.

## Interface visual

A branch `visualFTXUI` usa a biblioteca FTXUI para criar uma TUI, isto e, uma interface visual dentro do terminal.

Na pratica, o usuario interage com:

- Campos de texto para conta, senha, CPF e valores.
- Botoes para confirmar operacoes.
- Seletores para tipo de conta.
- Checkbox para habilitar analise de credito.
- Menu lateral com abas no dashboard.
- Cores e paineis para destacar mensagens, erros e informacoes importantes.

Use as setas do teclado, Tab, Enter e, quando o terminal permitir, o mouse para navegar entre campos e botoes.

Se a janela do terminal estiver pequena, o programa exibira uma mensagem pedindo para aumentar o tamanho. Isso evita que os componentes da interface fiquem sobrepostos.

## Tipos de conta

### Conta corrente

A conta corrente permite saque usando o saldo disponivel somado ao limite especial. Esse limite depende da renda mensal informada durante a analise de credito:

- Renda a partir de 20.000: nivel Black, limite de 2 vezes a renda.
- Renda a partir de 10.000: nivel Platinum, limite de 1,5 vez a renda.
- Renda a partir de 3.000: nivel Gold, limite de 1 vez a renda.
- Renda maior que 0 e menor que 3.000: nivel Silver, limite de 0,5 vez a renda.
- Sem analise de credito: nivel Standard, sem limite especial.

### Conta poupanca

A conta poupanca nao usa limite especial. Saques so sao aprovados quando ha saldo suficiente.

A taxa de rendimento depende do nivel da conta:

- Black: 10%.
- Platinum: 8%.
- Gold: 6%.
- Standard ou Silver: 5%.

Quando o usuario faz login, a conta atualiza seu estado. No caso da poupanca, rendimentos pendentes podem ser aplicados com base no historico de movimentacoes.

## Score interno

O sistema calcula um score financeiro interno de 0 a 1000 pontos. Ele considera principalmente:

- Saldo atual.
- Depositos registrados.
- Saques e transferencias enviadas.

Na aba `Perfil`, o usuario pode clicar em `VER ANALISE DE SCORE` para visualizar:

- Pontos que fortalecem o score.
- Pontos que podem melhorar.
- Fatores que prejudicam o score.

Esse score e uma regra interna do projeto para fins didaticos. Ele nao representa score bancario real.

## Requisitos

Para compilar a branch `visualFTXUI`, instale:

- CMake 3.14 ou superior.
- Git, necessario para o CMake baixar a dependencia FTXUI.
- Um compilador C++ com suporte a C++17.
- Um terminal com bom suporte a UTF-8.

Compiladores recomendados:

- Windows: MinGW-w64 com `g++` ou MSVC Build Tools.
- Linux: GCC/G++ ou Clang.
- macOS: Clang, instalado pelo Xcode Command Line Tools.

A dependencia FTXUI e declarada no `CMakeLists.txt` com `FetchContent`. Na primeira configuracao, o CMake baixa a versao `v5.0.0` do repositorio oficial da FTXUI.

## Como executar no Windows

O modo mais simples e usar:

```bat
run.bat
```

O script faz automaticamente:

1. Define a variavel `BANKOMP_KEY`.
2. Cria a pasta `build`, se necessario.
3. Executa `cmake ..`.
4. Compila com `cmake --build . --config Release`.
5. Executa o binario gerado.

Execucao manual pelo Prompt de Comando:

```bat
set BANKOMP_KEY=IME_SE9_SECRET_KEY_2026
if not exist build mkdir build
cd build
cmake ..
cmake --build . --config Release
bankomp.exe
```

Se o CMake usar um gerador multi-config, como Visual Studio, o executavel pode ficar dentro da pasta `Release`:

```bat
Release\bankomp.exe
```

Se estiver usando PowerShell:

```powershell
$env:BANKOMP_KEY="IME_SE9_SECRET_KEY_2026"
if (!(Test-Path build)) { New-Item -ItemType Directory build }
Set-Location build
cmake ..
cmake --build . --config Release
.\bankomp.exe
```

Para geradores multi-config no PowerShell:

```powershell
.\Release\bankomp.exe
```

## Como executar no Linux

Use o script:

```sh
chmod +x run.sh
./run.sh
```

Execucao manual:

```sh
export BANKOMP_KEY="IME_SE9_SECRET_KEY_2026"
mkdir -p build
cd build
cmake ..
make
./bankomp
```

Tambem e possivel usar o comando generico do CMake:

```sh
export BANKOMP_KEY="IME_SE9_SECRET_KEY_2026"
cmake -S . -B build
cmake --build build
./build/bankomp
```

## Como executar no macOS

Com Xcode Command Line Tools instalado:

```sh
export BANKOMP_KEY="IME_SE9_SECRET_KEY_2026"
mkdir -p build
cd build
cmake ..
make
./bankomp
```

Usando o comando generico do CMake:

```sh
export BANKOMP_KEY="IME_SE9_SECRET_KEY_2026"
cmake -S . -B build
cmake --build build
./build/bankomp
```

## Como usar o sistema

### Login

Na tela inicial, o usuario pode:

- Informar numero da conta e senha para entrar.
- Criar uma nova conta.
- Sair do sistema.

Depois de 3 tentativas invalidas de login, o programa encerra por seguranca.

### Criar uma conta

Para criar uma conta:

1. Clique em `CRIAR CONTA`.
2. Informe o nome completo.
3. Informe o CPF no formato `XXX.XXX.XXX-XX`.
4. Escolha o tipo de conta: corrente ou poupanca.
5. Informe o deposito inicial.
6. Marque a analise de credito se quiser informar renda mensal.
7. Crie uma senha com 8 a 15 caracteres, usando apenas letras e numeros.
8. Confirme a senha.
9. Clique em `CONCLUIR CADASTRO`.

Ao final do cadastro, o sistema mostra o numero da conta criada. Guarde esse numero, pois ele sera necessario para login.

### Dashboard

Apos o login, o usuario acessa o dashboard com as abas:

- `Inicio`: boas-vindas, numero da conta e nivel.
- `Deposito`: entrada de valores positivos.
- `Saque`: retirada de valores, respeitando saldo ou limite.
- `Transferencia`: envio para outra conta cadastrada.
- `Extrato`: lista de movimentacoes.
- `Perfil`: dados do cliente, score e analise financeira.

O cabecalho do dashboard mostra a conta ativa e o saldo. Use `VER/OCULTAR SALDO` para alternar entre saldo mascarado e visivel.

### Depositar

Na aba `Deposito`, informe um valor positivo e clique em `CONFIRMAR DEPOSITO`. O saldo e o extrato sao atualizados automaticamente.

### Sacar

Na aba `Saque`, informe o valor e clique em `CONFIRMAR SAQUE`.

- Em conta corrente, o saque pode usar o saldo mais o limite especial.
- Em conta poupanca, o saque exige saldo suficiente.

### Transferir

Na aba `Transferencia`:

1. Informe a conta de destino.
2. Informe o valor.
3. Clique em `VERIFICAR CONTA`.
4. Confira o nome do favorecido e o valor.
5. Clique em `CONFIRMAR ENVIO` ou `CANCELAR`.

Nao e permitido transferir para a propria conta.

### Consultar extrato

Na aba `Extrato`, o programa lista as movimentacoes com:

- Data e hora.
- Tipo da movimentacao.
- Valor.
- Detalhes da operacao.

### Ver perfil e score

Na aba `Perfil`, o usuario consulta:

- Titular.
- CPF.
- Agencia.
- Numero da conta.
- Nivel da conta.
- Score interno.

Clique em `VER ANALISE DE SCORE` para mostrar dicas financeiras baseadas no saldo e no historico de movimentacoes.

## Estrutura do projeto

```text
Bankomp/
|-- Control/       # Regras de negocio do banco
|-- Interface/     # Interface visual de terminal com FTXUI
|-- Model/         # Entidades do dominio, como cliente e contas
|-- Storage/       # Persistencia e criptografia simples
|-- CMakeLists.txt # Configuracao de build com CMake e FTXUI
|-- main.cpp       # Ponto de entrada da aplicacao
|-- run.bat        # Build e execucao no Windows
|-- run.sh         # Build e execucao no Linux/macOS
`-- README.md      # Documentacao da branch visualFTXUI
```

## Arquivos gerados

Durante a compilacao e o uso, alguns arquivos podem ser criados:

- `build/`: diretorio de build gerado pelo CMake.
- `bankomp.exe`: executavel gerado no Windows, dependendo do gerador usado.
- `bankomp`: executavel gerado no Linux/macOS.
- `banco.dat`: dados persistidos das contas.
- `transacoes.dat`: historico persistido das transacoes.

Esses arquivos sao resultado de compilacao ou execucao. Em geral, nao devem ser versionados no Git.

## Observacoes de seguranca

- A variavel `BANKOMP_KEY` deve ser a mesma entre execucoes. Se ela mudar, os dados salvos anteriormente podem nao ser lidos corretamente.
- A senha nao e salva diretamente: o sistema armazena um hash simples.
- Os arquivos `.dat` passam por criptografia XOR simples.
- Essa criptografia e adequada apenas para fins educacionais. Nao use este projeto como base de seguranca para dados reais de producao.

## Solucao de problemas

### `BANKOMP_KEY` nao configurada

Execute o programa usando `run.bat` ou `run.sh`. Eles definem a variavel automaticamente.

Se executar manualmente, defina a variavel antes de abrir o programa:

```sh
export BANKOMP_KEY="IME_SE9_SECRET_KEY_2026"
```

No Windows:

```bat
set BANKOMP_KEY=IME_SE9_SECRET_KEY_2026
```

### CMake nao encontra compilador C++

Instale um compilador C++17 e confirme que ele esta no `PATH`.

No Windows, uma opcao comum e instalar MinGW-w64 e garantir que a pasta `bin`, por exemplo `C:\mingw64\bin`, esteja no `PATH`.

### Primeira compilacao demora ou falha ao baixar dependencia

Na primeira configuracao, o CMake baixa a biblioteca FTXUI pelo Git. Verifique se:

- O Git esta instalado.
- Ha conexao com a internet.
- O terminal tem permissao para criar arquivos dentro da pasta `build`.

### Executavel nao esta em `Release`

Dependendo do gerador do CMake, o executavel pode ficar em:

- `build\Release\bankomp.exe`, comum com Visual Studio.
- `build\bankomp.exe`, comum com Ninja ou MinGW Makefiles.
- `build/bankomp`, comum no Linux/macOS.

O `run.bat` ja trata os dois caminhos mais comuns no Windows.

### Interface aparece quebrada ou pequena

Aumente a janela do terminal. Algumas telas exigem largura e altura minimas para renderizar corretamente.

Tambem prefira terminais modernos com suporte a UTF-8, como Windows Terminal, terminal integrado do VS Code, GNOME Terminal, Konsole ou iTerm2.

### Dados antigos nao aparecem

Verifique se:

- Voce esta executando o programa na mesma pasta onde os arquivos `.dat` foram criados.
- A variavel `BANKOMP_KEY` e a mesma usada quando os dados foram salvos.
- Os arquivos `banco.dat` e `transacoes.dat` nao foram apagados.

