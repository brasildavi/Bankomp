# Bankomp

Bankomp é um sistema bancário em C++ executado pelo terminal. O programa permite criar contas, autenticar usuários, realizar depósitos, saques, transferências, consultar extrato e visualizar informações do perfil financeiro.

## Branches do projeto

O projeto possui duas branches principais:

* `main`: versão console. Usa menus textuais simples e compila diretamente com `g++`.
* `visualFTXUI`: versão visual em terminal. Usa a biblioteca FTXUI e possui uma interface mais interativa.

Para alternar entre as branches:

```sh
git checkout main
git checkout visualFTXUI
```

Use este README como referência da branch `main`. Na branch `visualFTXUI`, os comandos e dependências podem ser diferentes, pois ela utiliza FTXUI e CMake.

## Funcionalidades

* Criação de contas bancárias.
* Login por número da conta e senha.
* Validação de CPF no formato `XXX.XXX.XXX-XX`.
* Validação matemática dos dígitos verificadores do CPF.
* Senha alfanumérica com 8 a 15 caracteres.
* Conta corrente com limite especial.
* Conta poupança com rendimento.
* Depósito, saque e transferência entre contas.
* Lista de transferências.
* Extrato com data, tipo, valor e descrição das transações.
* Perfil do cliente com nível da conta e score interno.
* Análise didática do score, mostrando pontos fortes, pontos de melhoria e fatores críticos.
* Persistência local dos dados em arquivos `.dat`.

## Como o programa funciona

Ao iniciar, o Bankomp procura a variável de ambiente `BANKOMP_KEY`. Essa chave é obrigatória, pois o programa a utiliza para ler e gravar os dados locais.

Depois, o sistema carrega os arquivos:

* `banco.dat`: armazena dados das contas, clientes, saldos, tipo de conta e hash da senha.
* `transacoes.dat`: armazena o histórico de transações.

Se esses arquivos não existirem, o programa inicia sem contas cadastradas. Eles são criados automaticamente quando uma conta ou transação é salva.

O fluxo principal é:

1. Menu inicial: acessar conta, abrir nova conta ou sair.
2. Cadastro: coleta nome, CPF, tipo de conta, depósito inicial, renda mensal (opcional) e senha.
3. Login: autentica pelo número da conta e senha.
4. Sessão do usuário: permite movimentações e consultas.
5. Salvamento: operações relevantes atualizam os arquivos `.dat`.

## Tipos de conta

### Conta corrente

A conta corrente permite saque usando o saldo disponível somado ao limite especial. Esse limite depende da renda mensal informada durante a análise de crédito:

* Renda a partir de 20.000: nível Black, limite de 2 vezes a renda.
* Renda a partir de 10.000: nível Platinum, limite de 1,5 vez a renda.
* Renda a partir de 3.000: nível Gold, limite de 1 vez a renda.
* Renda maior que 0 e menor que 3.000: nível Silver, limite de 0,5 vez a renda.
* Sem análise de crédito: nível Standard, sem limite especial.

### Conta poupança

A conta poupança não usa limite especial. Saques só são aprovados quando há saldo suficiente.

A taxa de rendimento depende do nível da conta:

* Black: 10%.
* Platinum: 8%.
* Gold: 6%.
* Standard ou Silver: 5%.

Quando o usuário faz login, a conta atualiza seu estado. No caso da poupança, rendimentos pendentes podem ser aplicados com base no histórico de movimentações.

## Score interno

O sistema calcula um score financeiro interno de 0 a 1000 pontos. Ele considera principalmente:

* Saldo atual.
* Depósitos registrados.
* Saques e transferências enviadas.

No menu de perfil, o usuário pode abrir uma análise com:

* Pontos que fortalecem o score.
* Pontos que podem melhorar.
* Fatores que prejudicam o score.

Esse score é uma regra interna do projeto para fins didáticos. Ele não representa score bancário real.

## Requisitos

Para compilar a branch `main`, instale:

* Um compilador C++ com suporte a C++17.
* `g++`, recomendado porque os scripts `run.bat` e `run.sh` usam esse compilador.
* Um terminal ou prompt de comando.

Compiladores recomendados:

* Windows: MinGW-w64 com `g++`.
* Linux: GCC/G++.
* macOS: GCC instalado via Homebrew, ou Clang com comando manual adaptado para `clang++`.

Não é necessário CMake para a branch `main`.

## Como executar no Windows

O modo mais simples é usar:

```bat
run.bat
```

O script faz automaticamente:

1. Define a variável `BANKOMP_KEY`.
2. Monta a lista de arquivos `.cpp`.
3. Compila o projeto com `g++ -std=c++17`.
4. Gera `bankomp.exe`.
5. Executa o programa.

Execução manual pelo Prompt de Comando:

```bat
set BANKOMP_KEY=IME_SE9_SECRET_KEY_2026
g++ -std=c++17 main.cpp Control\*.cpp Interface\*.cpp Model\*.cpp Storage\*.cpp -o bankomp.exe
bankomp.exe
```

Se estiver usando PowerShell:

```powershell
$env:BANKOMP_KEY="IME_SE9_SECRET_KEY_2026"
g++ -std=c++17 main.cpp Control\*.cpp Interface\*.cpp Model\*.cpp Storage\*.cpp -o bankomp.exe
.\bankomp.exe
```

## Como executar no Linux

Use o script:

```sh
chmod +x run.sh
./run.sh
```

Execução manual:

```sh
export BANKOMP_KEY="IME_SE9_SECRET_KEY_2026"
g++ -std=c++17 main.cpp Control/*.cpp Interface/*.cpp Model/*.cpp Storage/*.cpp -o bankomp
./bankomp
```

## Como executar no macOS

Se estiver usando GCC instalado via Homebrew:

```sh
export BANKOMP_KEY="IME_SE9_SECRET_KEY_2026"
g++ -std=c++17 main.cpp Control/*.cpp Interface/*.cpp Model/*.cpp Storage/*.cpp -o bankomp
./bankomp
```

Se preferir usar Clang:

```sh
export BANKOMP_KEY="IME_SE9_SECRET_KEY_2026"
clang++ -std=c++17 main.cpp Control/*.cpp Interface/*.cpp Model/*.cpp Storage/*.cpp -o bankomp
./bankomp
```

## Como usar o sistema

### Menu inicial

Ao abrir o programa, o usuário verá três opções:

1. Acessar uma conta existente.
2. Abrir uma nova conta.
3. Sair do sistema.

Digite o número da opção desejada e pressione Enter.

### Abrir nova conta

Para criar uma conta:

1. Escolha `Abrir Nova Conta`.
2. Informe o nome do titular.
3. Informe o CPF no formato `XXX.XXX.XXX-XX`.
4. Escolha o tipo de conta: `1` para corrente ou `2` para poupança.
5. Informe o depósito inicial.
6. Escolha se deseja realizar análise de crédito.
7. Caso escolha análise, informe a renda mensal.
8. Crie uma senha com 8 a 15 caracteres, usando apenas letras e números.
9. Confirme a senha.

Ao final do cadastro, o sistema mostra o número da conta criada. Guarde esse número, pois ele será necessário para login.

### Acessar uma conta

Para entrar:

1. Escolha `Acessar Minha Conta`.
2. Digite o número da conta.
3. Digite a senha.

Por segurança, o programa permite até 3 tentativas de senha. Se todas falharem, o sistema encerra.

### Depositar

Na sessão do usuário, escolha `Depósito`, informe um valor positivo e confirme. O saldo e o extrato são atualizados.

### Sacar

Escolha `Saque` e informe o valor.

* Em conta corrente, o saque pode usar o saldo mais o limite especial.
* Em conta poupança, o saque exige saldo suficiente.

### Transferir

Escolha `Transferência Bancária`.

O sistema mostra favorecidos recentes, pede a conta de destino, valida se a conta existe e exibe uma tela de confirmação antes de enviar.

Não é permitido transferir para a própria conta.

### Consultar extrato

Escolha `Extrato de Transações` para ver o histórico com:

* Data e hora.
* Tipo da movimentação.
* Valor.
* Detalhes da operação.

### Ver perfil e score

Escolha `Perfil e Score de Crédito` para consultar:

* Titular.
* CPF.
* Número da conta.
* Nível da conta.
* Score interno.
* Análise e dicas para melhorar o score.

### Ver ou ocultar saldo

Durante a sessão, o saldo aparece mascarado por padrão. Use `Ver/Ocultar Saldo` para alternar entre saldo oculto e saldo visível.

## Estrutura do projeto

```text
Bankomp/
|-- Control/       # Regras de negócio do banco
|-- Interface/     # Interface de usuário no console
|-- Model/         # Entidades do domínio, como cliente e contas
|-- Storage/       # Persistência e criptografia simples
|-- main.cpp       # Ponto de entrada da aplicação
|-- run.bat        # Compilação e execução no Windows
|-- run.sh         # Compilação e execução no Linux/macOS
`-- README.md      # Documentação da branch main
```

## Arquivos gerados

Durante o uso, alguns arquivos podem ser criados:

* `bankomp.exe`: executável gerado no Windows.
* `bankomp`: executável gerado no Linux/macOS.
* `banco.dat`: dados persistidos das contas.
* `transacoes.dat`: histórico persistido das transações.

Esses arquivos são resultado de compilação ou execução. Em geral, não devem ser versionados no Git.

## Observações de segurança

* A variável `BANKOMP_KEY` deve ser a mesma entre execuções. Se ela mudar, os dados salvos anteriormente podem não ser lidos corretamente.
* A senha não é salva diretamente: o sistema armazena um hash simples.
* Os arquivos `.dat` passam por criptografia XOR simples.
* Essa criptografia é adequada apenas para fins educacionais. Não use este projeto como base de segurança para dados reais de produção.

## Solução de problemas

### `BANKOMP_KEY` não configurada

Execute o programa usando `run.bat` ou `run.sh`. Eles definem a variável automaticamente.

Se executar manualmente, defina a variável antes de abrir o programa:

```sh
export BANKOMP_KEY="IME_SE9_SECRET_KEY_2026"
```

No Windows:

```bat
set BANKOMP_KEY=IME_SE9_SECRET_KEY_2026
```

### `g++` não é reconhecido

Instale um compilador C++ e adicione o executável ao `PATH`.

No Windows, instale MinGW-w64 e confirme que a pasta `bin`, por exemplo `C:\mingw64\bin`, está no `PATH`.

### Erros de acentuação no terminal

No Windows, use um terminal com suporte a UTF-8. O programa tenta configurar a página de código para UTF-8 ao iniciar.

### Dados antigos não aparecem

Verifique se:

* Você está executando o programa na mesma pasta onde os arquivos `.dat` foram criados.
* A variável `BANKOMP_KEY` é a mesma usada quando os dados foram salvos.
* Os arquivos `banco.dat` e `transacoes.dat` não foram apagados.
