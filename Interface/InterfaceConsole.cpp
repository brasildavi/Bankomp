#include "InterfaceConsole.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <limits> 
#include <regex> 
#include <ctime>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"

InterfaceConsole::InterfaceConsole(std::shared_ptr<Bank> b) : bank(b) {}

int readIntSafe(const std::string& prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cout << RED << "Entrada inválida! Por favor, digite um número inteiro.\n" << RESET;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

double readDoubleSafe(const std::string& prompt) {
    double value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cout << RED << "Entrada inválida! Por favor, digite um valor numérico válido.\n" << RESET;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

std::string readStringSafe(const std::string& prompt, bool allowSpaces = false) {
    std::string value;
    while (true) {
        std::cout << prompt;
        if (allowSpaces) {
            std::getline(std::cin, value);
        } else {
            std::cin >> value;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        if (!value.empty()) {
            return value;
        }
        std::cout << RED << "O campo não pode ficar vazio.\n" << RESET;
    }
}

void InterfaceConsole::showMenu() {
    while (true) {
        std::cout << CYAN << "\n=== BANKOMP - SISTEMA BANCÁRIO ===\n" << RESET;
        std::cout << "1. Acessar Minha Conta\n";
        std::cout << "2. Abrir Nova Conta\n";
        std::cout << "3. Sair\n";
        
        int option = readIntSafe("Escolha uma opção: ");

        if (option == 1) handleLogin();
        else if (option == 2) handleCreateAccount();
        else if (option == 3) break;
        else std::cout << RED << "Opção inválida!\n" << RESET;
    }
}

void InterfaceConsole::handleLogin() {
    int number = readIntSafe("\nNúmero da Conta: ");
    int attempts = 3;

    while (attempts > 0) {
        std::cout << "Senha: ";
        setStdinEcho(false);
        std::string password = readStringSafe("");
        setStdinEcho(true);
        std::cout << "\n";

        auto acc = bank->authenticate(number, password);
        if (acc) {
            std::cout << GREEN << "Login efetuado com sucesso!\n" << RESET;
            acc->updateAccountState(); 
            bank->saveToStorage();
            handleUserSession(acc);
            return;
        } else {
            attempts--;
            if (attempts > 0) {
                std::cout << RED << "Conta ou senha incorretas. Você tem " << attempts << " tentativa(s) restante(s).\n" << RESET;
            } else {
                std::cout << RED << "\n[SEGURANÇA] Número máximo de tentativas atingido. Encerrando o programa...\n" << RESET;
                std::exit(1);
            }
        }
    }
}

void InterfaceConsole::handleCreateAccount() {
    std::string name, cpf, password, passwordconfirm;
    int type;
    double deposit;

    std::cout << CYAN << "\n=== ABERTURA DE NOVA CONTA ===\n" << RESET;
    
    name = readStringSafe("Nome do Titular: ", true);
    
    std::regex cpfRegex(R"(\d{3}\.\d{3}\.\d{3}-\d{2})");
    while (true) {
        cpf = readStringSafe("CPF (formato XXX.XXX.XXX-XX ou 0 para voltar): ", false);
        if (cpf == "0") return;

        if (!std::regex_match(cpf, cpfRegex)) {
            std::cout << RED << "Formato inválido! Use os pontos e o traço.\n" << RESET;
            continue;
        }
        if (!validateCPFMath(cpf)) {
            std::cout << RED << "Erro: Este CPF não é um número válido real.\n" << RESET;
            continue;
        }
        if (bank->isCpfRegistered(cpf)) {
            std::cout << RED << "Erro: Este CPF já possui uma conta no sistema!\n" << RESET;
            return;
        }
        break;
    }
    
    while (true) {
        type = readIntSafe("Tipo (1 - Corrente, 2 - Poupança): ");
        if (type == 1 || type == 2) break;
        std::cout << RED << "Tipo inválido! Escolha 1 ou 2.\n" << RESET;
    }
    
    while (true) {
        deposit = readDoubleSafe("Depósito Inicial: R$ ");
        if (deposit >= 0) break;
        std::cout << RED << "O depósito inicial não pode ser negativo.\n" << RESET;
    }

    double income = 0.0;
    int wantsAnalysis = readIntSafe("\nDeseja realizar análise de crédito para obter benefícios e limites maiores? (1 - Sim, 2 - Não): ");
    
    if (wantsAnalysis == 1) {
        while (true) {
            income = readDoubleSafe("Informe sua renda mensal comprovada: R$ ");
            if (income >= 0) break;
            std::cout << RED << "Renda não pode ser negativa.\n" << RESET;
        }
        std::cout << BLUE << "Analisando perfil...\n" << RESET;
    }
    
    while (true) {
        std::cout << "Crie uma Senha (8-15 caracteres, apenas letras e números): ";
        setStdinEcho(false);
        password = readStringSafe("");
        setStdinEcho(true);
        std::cout << "\n";

        if (!validatePasswordRules(password)) {
            std::cout << RED << "Senha inválida! Deve ter entre 8 e 15 caracteres e conter apenas letras e números.\n" << RESET;
            continue;
        }
        
        std::cout << "Confirme sua Senha: ";
        setStdinEcho(false);
        passwordconfirm = readStringSafe("");
        setStdinEcho(true);
        std::cout << "\n";

        if (password == passwordconfirm) break;
        std::cout << RED << "As senhas não correspondem! Tente novamente.\n" << RESET;
    }

    int accountNumber = bank->createAccount(name, cpf, type, deposit, password, income);

    if (accountNumber > 0) {
        auto newAcc = bank->findAccount(accountNumber);
        std::cout << GREEN << "\n=======================================\n" << RESET;
        std::cout << GREEN << "      CONTA CRIADA COM SUCESSO!        \n" << RESET;
        std::cout << GREEN << "=======================================\n" << RESET;
        std::cout << " Titular: " << name << "\n";
        std::cout << " CPF: " << cpf << "\n";
        std::cout << " Nível da Conta: " << CYAN << newAcc->getTierName() << RESET << "\n";
        std::cout << " Saldo Inicial: R$ " << std::fixed << std::setprecision(2) << deposit << "\n";
        std::cout << "---------------------------------------\n";
        std::cout << " NÚMERO DA CONTA: " << CYAN << accountNumber << RESET << "\n";
        std::cout << "---------------------------------------\n";
        std::cout << RED << " ATENÇÃO: Guarde este número para Login!\n" << RESET;
        std::cout << GREEN << "=======================================\n\n" << RESET;
    }
}

void InterfaceConsole::handleUserSession(std::shared_ptr<Account> acc) {
    bool showSessionBalance = false; // Controle de visibilidade do saldo no menu principal

    while (true) {
        std::cout << BLUE << "\n--- Bem-vindo, " << acc->getClient()->getName() << " ---\n" << RESET;
        
        // Exibição do Saldo (Mascarado ou Visível)
        std::cout << "Saldo Flexível: ";
        if (showSessionBalance) {
            std::cout << GREEN << "R$ " << std::fixed << std::setprecision(2) << acc->getBalance() << RESET << "\n";
        } else {
            std::cout << MAGENTA << "R$ XXXX.XX" << RESET << "\n";
        }

        std::cout << "1. Depósito\n";
        std::cout << "2. Saque\n";
        std::cout << "3. Transferência Bancária\n";
        std::cout << "4. Extrato de Transações\n";
        std::cout << "5. Perfil e Score de Crédito\n";
        std::cout << "6. Ver/Ocultar Saldo\n";
        std::cout << "7. Logout (Sair da Conta)\n";
        
        int option = readIntSafe("Escolha uma opção: ");

        if (option == 1) {
            double val = readDoubleSafe("Valor do depósito (ou 0 para voltar): R$ ");
            if (val == 0) continue;

            if (val > 0) {
                acc->deposit(val);
                acc->addTransaction(4, val, "Depósito em Espécie");
                bank->saveToStorage();
                std::cout << GREEN << "Depósito realizado com sucesso!\n" << RESET;
            } else {
                std::cout << RED << "Valor inválido para depósito.\n" << RESET;
            }
        } else if (option == 2) {
            double val = readDoubleSafe("Valor do saque (ou 0 para voltar): R$ ");
            if (val == 0) continue;
            
            if (val > 0 && acc->withdraw(val)) {
                acc->addTransaction(1, val, "Saque em Dinheiro");
                bank->saveToStorage();
                std::cout << GREEN << "Saque realizado com sucesso!\n" << RESET;
            } else {
                std::cout << RED << "Saldo ou limite de crédito insuficiente ou valor inválido.\n" << RESET;
            }
        } else if (option == 3) {
            std::cout << CYAN << "\n=== NOVA TRANSFERÊNCIA ===\n" << RESET;
            std::cout << "Favorecidos Recentes:\n";
            std::vector<std::string> uniqueDests;
            bool hasRecents = false;
            for (const auto& tx : acc->getTransactions()) {
                if (tx.type == 2) { 
                    if (std::find(uniqueDests.begin(), uniqueDests.end(), tx.details) == uniqueDests.end()) {
                        uniqueDests.push_back(tx.details);
                        std::cout << " • " << tx.details << "\n";
                        hasRecents = true;
                    }
                }
            }
            if (!hasRecents) std::cout << " (Nenhum histórico de transferência encontrado)\n";
            std::cout << "---------------------------------------\n";

            int destNumber = readIntSafe("Digite o número da conta destino (ou 0 para VOLTAR): ");
            if (destNumber == 0) continue;

            if (destNumber == acc->getNumber()) {
                std::cout << RED << "Erro: Não é possível transferir fundos para a sua própria conta.\n" << RESET;
                continue;
            }

            auto destAcc = bank->findAccount(destNumber);
            if (!destAcc) {
                std::cout << RED << "Erro: Conta informada não existe no sistema.\n" << RESET;
                continue;
            }

            double val = readDoubleSafe("Digite o valor a transferir (ou 0 para VOLTAR): R$ ");
            if (val <= 0) continue;

            std::cout << CYAN << "\n┌──────────────────────────────────────┐\n";
            std::cout << "│        CONFIRMAÇÃO DE ENVIO          │\n";
            std::cout << "└──────────────────────────────────────┘\n" << RESET;
            std::cout << " FAVORECIDO: " << destAcc->getClient()->getName() << "\n";
            std::cout << " CONTA:      " << destAcc->getNumber() << "\n";
            std::cout << " VALOR:      R$ " << val << "\n";
            std::cout << "───────────────────────────────────────\n";
            std::cout << "1. Confirmar e Enviar Dinheiro\n";
            std::cout << "2. Cancelar Transação e Voltar\n";
            
            int confirm = readIntSafe("Escolha uma opção: ");

            if (confirm != 1) {
                std::cout << RED << "Transferência abortada pelo usuário.\n" << RESET;
                continue;
            }

            if (bank->transfer(acc->getNumber(), destNumber, val)) {
                std::cout << GREEN << "Transferência concluída com sucesso!\n" << RESET;
            } else {
                std::cout << RED << "Falha: Saldo ou limite de crédito insuficiente.\n" << RESET;
            }
        } else if (option == 4) {
            std::cout << CYAN << "\n=======================================================\n";
            std::cout << "                 EXTRATO DE TRANSAÇÕES                 \n";
            std::cout << "=======================================================\n" << RESET;
            
            const auto& txs = acc->getTransactions();
            if (txs.empty()) {
                std::cout << "  Nenhuma movimentação registrada.\n";
            } else {
                for (const auto& tx : txs) {
                    std::string label;
                    if (tx.type == 1) label = RED "[SAQUE]       " RESET;
                    else if (tx.type == 2) label = RED "[TX ENVIADA]  " RESET;
                    else if (tx.type == 3) label = GREEN "[TX RECEBIDA] " RESET;
                    else if (tx.type == 4) label = GREEN "[DEPÓSITO]    " RESET;
                    else if (tx.type == 5) label = GREEN "[RENDIMENTO]  " RESET;
                    
                    // Formatação da Data/Hora
                    char timeStr[20] = "Data Desconhecida";
                    if (tx.timestamp != 0) {
                        struct tm* timeinfo = std::localtime(&tx.timestamp);
                        std::strftime(timeStr, sizeof(timeStr), "%d/%m/%Y %H:%M", timeinfo);
                    }

                    std::cout << "[" << timeStr << "] " << label 
                              << "R$ " << std::setw(8) << std::fixed << std::setprecision(2) << tx.amount 
                              << " -> " << tx.details << "\n";
                }
            }
            std::cout << CYAN << "=======================================================\n" << RESET;
        } else if (option == 5) {
            handleViewProfile(acc);
        } else if (option == 6) {
            showSessionBalance = !showSessionBalance; 
        } else if (option == 7) {
            std::cout << GREEN << "Saindo da conta... Até logo!\n" << RESET;
            break;
        } else {
            std::cout << RED << "Opção Inválida!\n" << RESET;
        }
    }
}

void InterfaceConsole::setStdinEcho(bool enable) {
    #ifdef _WIN32
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(hStdin, &mode);
        if (!enable) {
            mode &= ~ENABLE_ECHO_INPUT;
        } else {
            mode |= ENABLE_ECHO_INPUT;
        }
        SetConsoleMode(hStdin, mode);
    #else
        struct termios tty;
        tcgetattr(STDIN_FILENO, &tty);
        if (!enable) {
            tty.c_lflag &= ~ECHO;
        } else {
            tty.c_lflag |= ECHO;
        }
        (void)tcsetattr(STDIN_FILENO, TCSANOW, &tty);
    #endif
    }

void InterfaceConsole::handleViewProfile(std::shared_ptr<Account> acc) {
    while (true) {
        std::cout << CYAN << "\n=======================================\n" << RESET;
        std::cout << CYAN << "      PERFIL E INFORMAÇÕES DA CONTA    \n" << RESET;
        std::cout << CYAN << "=======================================\n" << RESET;
        
        std::cout << " Titular: " << acc->getClient()->getName() << "\n";
        std::cout << " CPF: " << acc->getClient()->getCpf() << "\n";
        std::cout << " Conta: " << acc->getNumber() << "\n";
        std::cout << " Nível: " << acc->getTierName() << "\n";
        
        int score = acc->calculateCreditScore();
        std::cout << " Score de Crédito Interno: ";
        
        if (score >= 700) std::cout << GREEN << score << " / 1000 (Excelente)\n" << RESET;
        else if (score >= 300) std::cout << YELLOW << score << " / 1000 (Bom)\n" << RESET;
        else std::cout << RED << score << " / 1000 (Baixo)\n" << RESET;

        std::cout << "---------------------------------------\n";
        std::cout << "1. Análise e Dicas para o Score\n";
        std::cout << "0. Voltar\n";
        
        int choice = readIntSafe("Escolha uma opção: ");
        
        if (choice == 1) {
            while (true) {
                std::cout << CYAN << "\n--- ANÁLISE DO SEU PERFIL FINANCEIRO ---\n" << RESET;
                
                double dep = acc->getMonthlyDeposits();
                double wth = acc->getMonthlyWithdrawals();
                double bal = acc->getBalance();

                std::cout << GREEN << "[+] O que está fortalecendo seu Score:\n" << RESET;
                bool hasGood = false;
                if (dep > wth && dep > 0) { 
                    std::cout << "  - Entradas maiores que saídas. Excelente controle financeiro!\n"; 
                    hasGood = true; 
                }
                if (bal >= 1000) { 
                    std::cout << "  - Saldo alto e estável, o que garante uma excelente pontuação base.\n"; 
                    hasGood = true; 
                }
                if (!hasGood) std::cout << "  - (Nenhum ponto forte detectado no momento)\n";

                std::cout << YELLOW << "\n[~] O que pode melhorar:\n" << RESET;
                bool hasImprove = false;
                if (dep > 0 && wth >= dep) { 
                    std::cout << "  - Você possui entradas, mas os saques/gastos estão equivalentes ou maiores.\n"; 
                    hasImprove = true; 
                }
                if (bal > 0 && bal < 1000) { 
                    std::cout << "  - Seu saldo está positivo, mas aumentar a quantia guardada subirá seu score.\n"; 
                    hasImprove = true; 
                }
                if (!hasImprove) std::cout << "  - (Nenhum alerta moderado no momento)\n";

                std::cout << RED << "\n[-] O que está prejudicando seu Score:\n" << RESET;
                bool hasBad = false;
                if (dep == 0) {
                    std::cout << "  - Zero movimentações de entrada. Depósitos representam 60% da nota.\n";
                    hasBad = true;
                }
                if (bal <= 0) { 
                    std::cout << "  - Saldo zerado. Manter dinheiro na conta compõe 40% da sua nota.\n"; 
                    hasBad = true; 
                }
                if (!hasBad) std::cout << "  - (Nenhum fator crítico detectado)\n";
                
                std::cout << "---------------------------------------\n";
                
                int backChoice = readIntSafe("Digite 0 para voltar: ");
                if (backChoice == 0) {
                    break;
                } else {
                    std::cout << RED << "Opção inválida.\n" << RESET;
                }
            }
        } else if (choice == 0) {
            break;
        } else {
            std::cout << RED << "Opção inválida.\n" << RESET;
        }
    }
}

bool InterfaceConsole::validateCPFMath(const std::string& cpf) {
    std::string digits = "";
    for (char c : cpf) {
        if (std::isdigit(c)) digits += c;
    }

    if (digits.length() != 11) return false;

    if (std::all_of(digits.begin(), digits.end(), [&](char c) { return c == digits[0]; })) {
        return false;
    }

    int sum = 0;
    for (int i = 0; i < 9; ++i) sum += (digits[i] - '0') * (10 - i);
    int rem = sum % 11;
    int d1 = (rem < 2) ? 0 : 11 - rem;
    if (digits[9] - '0' != d1) return false;

    sum = 0;
    for (int i = 0; i < 10; ++i) sum += (digits[i] - '0') * (11 - i);
    rem = sum % 11;
    int d2 = (rem < 2) ? 0 : 11 - rem;
    if (digits[10] - '0' != d2) return false;

    return true;
}

bool InterfaceConsole::validatePasswordRules(const std::string& password) {
    if (password.length() < 8 || password.length() > 15) return false;

    std::regex safeRegex("^[a-zA-Z0-9]+$");
    return std::regex_match(password, safeRegex);
}