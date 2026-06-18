#include "Interface/InterfaceTUI.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>
#include <iostream>
#include <regex>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace ftxui;

// ─────────────────────────────────────────────
//  Helpers de formatação
// ─────────────────────────────────────────────
static std::string fmtMoney(double v) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << v;
    return "R$ " + ss.str();
}

static std::string fmtTimestamp(std::time_t ts) {
    if (ts == 0) return "Data desconhecida";
    char buf[20];
    struct tm* ti = std::localtime(&ts);
    std::strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M", ti);
    return std::string(buf);
}

// ─────────────────────────────────────────────
//  Construtor
// ─────────────────────────────────────────────
InterfaceTUI::InterfaceTUI(std::shared_ptr<Bank> bank_ptr) : bank(bank_ptr) {}

// ─────────────────────────────────────────────
//  Loop principal
// ─────────────────────────────────────────────
void InterfaceTUI::showMenu() {
    while (current_state != AppState::EXIT) {
        switch (current_state) {
            case AppState::LOGIN:    runLoginScreen();    break;
            case AppState::REGISTER: runRegisterScreen(); break;
            case AppState::DASHBOARD: runDashboard();     break;
            case AppState::EXIT: break;
        }
    }
}

// ─────────────────────────────────────────────
//  LOGIN
// ─────────────────────────────────────────────
void InterfaceTUI::runLoginScreen() {
    auto screen = ScreenInteractive::Fullscreen();
    std::string account_num_str, password, error_message;

    Component input_account  = Input(&account_num_str, " Número da conta...");
    InputOption pass_opt; pass_opt.password = true;
    Component input_password = Input(&password, " Senha...", pass_opt);

    Component btn_login = Button(" ENTRAR ", [&] {
        if (account_num_str.empty() || password.empty()) {
            error_message = "Preencha todos os campos!";
            return;
        }
        try {
            int acc_num = std::stoi(account_num_str);
            auto acc = bank->authenticate(acc_num, password);
            if (acc) {
                acc->updateAccountState();
                bank->saveToStorage();
                current_account = acc;
                current_state   = AppState::DASHBOARD;
                screen.ExitLoopClosure()();
            } else {
                login_attempts++;
                if (login_attempts >= 3) {
                    screen.ExitLoopClosure()();
                    current_state = AppState::EXIT;
                    return;
                }
                int remaining = 3 - login_attempts;
                error_message = "Dados inválidos! " + std::to_string(remaining) + " tentativa(s) restante(s).";
            }
        } catch (...) { error_message = "Número de conta inválido!"; }
    });

    Component btn_register = Button(" CRIAR CONTA ", [&] {
        current_state = AppState::REGISTER;
        screen.ExitLoopClosure()();
    });

    Component btn_exit = Button(" SAIR DO SISTEMA ", [&] {
        current_state = AppState::EXIT;
        screen.ExitLoopClosure()();
    });

    // CORREÇÃO: Colocamos todos os elementos num único nível lógico
    // Assim o FTXUI calcula perfeitamente as coordenadas para o rato e o Enter
    auto main_container = Container::Vertical({
        input_account,
        input_password,
        btn_login,
        btn_register,
        btn_exit
    });

    auto renderer = Renderer(main_container, [&] {
        auto terminal_size = Terminal::Size();
        if (terminal_size.dimx < 45 || terminal_size.dimy < 20) {
            return center(text(" Aumente a janela do terminal! ")) | bold | color(Color::Yellow) | border;
        }

        auto login_box = vbox({
            text(" 🏦 BANKOMP SYSTEM ") | bold | color(Color::Cyan) | center,
            separatorDouble(),
            filler(),
            text(" NÚMERO DA CONTA:") | dim,
            hbox({ input_account->Render() | flex }) | borderRounded,
            separatorEmpty(),
            text(" SENHA DE ACESSO:") | dim,
            hbox({ input_password->Render() | flex }) | borderRounded,
            filler(),
            text(error_message) | color(Color::Red) | center | bold,
            filler(),
            
            // O desenho (Renderer) pode organizá-los lado a lado sem problemas
            hbox({
                btn_login->Render() | flex,
                separatorEmpty(),
                btn_register->Render() | flex
            }),
            
            separatorEmpty(),
            btn_exit->Render() | color(Color::Red) | center
        }) | border | size(WIDTH, EQUAL, 50) | size(HEIGHT, EQUAL, 25);

        return vbox({ filler(), hbox({ filler(), login_box, filler() }), filler() });
    });

    screen.Loop(renderer);
}

// ─────────────────────────────────────────────
//  CADASTRO
// ─────────────────────────────────────────────
void InterfaceTUI::runRegisterScreen() {
    auto screen = ScreenInteractive::Fullscreen();

    std::string name, cpf, deposit_str, income_str, password, confirm_password, msg;
    bool income_enabled = false;
    int  type_selected  = 0;
    std::vector<std::string> type_entries = {" 1. Corrente ", " 2. Poupança "};

    Component input_name     = Input(&name,             " Nome Completo");
    Component input_cpf      = Input(&cpf,              " CPF (XXX.XXX.XXX-XX)");
    Component radio_type     = Radiobox(&type_entries, &type_selected);
    Component input_deposit  = Input(&deposit_str,      " Ex: 100.50");
    Component input_income   = Input(&income_str,       " Ex: 2500.00");
    CheckboxOption cb_opt;
    Component chk_income     = Checkbox(" Realizar análise de crédito (informar renda mensal)", &income_enabled, cb_opt);

    InputOption pass_opt; pass_opt.password = true;
    Component input_password = Input(&password,          " Senha (8-15 chars, alfanumérica)", pass_opt);
    Component input_confirm  = Input(&confirm_password,  " Confirmar Senha", pass_opt);

    Component btn_cadastrar = Button(" CONCLUIR CADASTRO ", [&] {
        msg = "";
        if (name.empty() || cpf.empty() || deposit_str.empty() || password.empty()) {
            msg = "Erro: Preencha todos os campos obrigatórios!"; return;
        }

        std::regex cpfRegex(R"(\d{3}\.\d{3}\.\d{3}-\d{2})");
        if (!std::regex_match(cpf, cpfRegex)) { msg = "Erro: CPF inválido. Use XXX.XXX.XXX-XX"; return; }
        if (!validateCPFMath(cpf)) { msg = "Erro: CPF matematicamente inválido!"; return; }
        if (bank->isCpfRegistered(cpf)) { msg = "Erro: Este CPF já possui uma conta!"; return; }
        if (!validatePasswordRules(password)) { msg = "Erro: Senha deve ter 8-15 caracteres (letras e números)."; return; }
        if (password != confirm_password) { msg = "Erro: As senhas não coincidem!"; return; }

        try {
            double deposit = std::stod(deposit_str);
            double income  = 0.0;

            if (deposit < 0) { msg = "Erro: Depósito não pode ser negativo!"; return; }
            if (income_enabled) {
                if (income_str.empty()) { msg = "Erro: Informe a renda mensal para análise."; return; }
                income = std::stod(income_str);
                if (income < 0) { msg = "Erro: Renda não pode ser negativa!"; return; }
            }

            int type   = type_selected + 1;
            int accNum = bank->createAccount(name, cpf, type, deposit, password, income);

            if (accNum > 0) {
                auto newAcc = bank->findAccount(accNum);
                std::string tier = newAcc ? newAcc->getTierName() : "—";
                msg = "SUCESSO! Conta #" + std::to_string(accNum) + " criada | Nível: " + tier;
            } else { msg = "Erro interno ao criar a conta."; }
        } catch (...) { msg = "Erro: Digite apenas números válidos nos campos monetários!"; }
    });

    Component btn_voltar = Button(" VOLTAR AO LOGIN ", [&] {
        current_state = AppState::LOGIN;
        screen.ExitLoopClosure()();
    });

    // CORREÇÃO: Agrupamento lógico dos botões lado a lado
    Component horizontal_btns = Container::Horizontal({ btn_voltar, btn_cadastrar });

    auto form_container = Container::Vertical({
        input_name, input_cpf, radio_type, input_deposit,
        chk_income, input_income,
        input_password, input_confirm, horizontal_btns
    });

    auto renderer = Renderer(form_container, [&] {
        auto sz = Terminal::Size();
        if (sz.dimx < 62 || sz.dimy < 45) {
            return center(text(" Janela muito pequena para o Cadastro! Aumente o terminal. ")
                   | bold | color(Color::Yellow) | border);
        }

        bool isSuccess = msg.find("SUCESSO") != std::string::npos;
        Color msgColor = isSuccess ? Color::Green : Color::Red;

        auto income_section = income_enabled
            ? vbox({ text(" Renda Mensal:") | bold, hbox({ input_income->Render() | flex }) | borderRounded })
            : vbox({ text("") });

        auto reg_box = vbox({
            text(" ABERTURA DE NOVA CONTA ") | bold | center | color(Color::Cyan),
            separatorDouble(),
            text(" DADOS PESSOAIS:") | bold,
            hbox({ input_name->Render() | flex }) | borderRounded,
            hbox({ input_cpf->Render() | flex }) | borderRounded,
            separatorEmpty(),
            text(" TIPO DE CONTA:") | bold,
            radio_type->Render(),
            separatorEmpty(),
            text(" DADOS FINANCEIROS:") | bold,
            text(" Depósito Inicial:") | dim,
            hbox({ input_deposit->Render() | flex }) | borderRounded,
            chk_income->Render(),
            income_section,
            separatorEmpty(),
            text(" SEGURANÇA:") | bold,
            hbox({ input_password->Render() | flex }) | borderRounded,
            hbox({ input_confirm->Render() | flex }) | borderRounded,
            filler(),
            text(msg) | color(msgColor) | center | bold,
            filler(),
            hbox({ btn_voltar->Render() | flex, separatorEmpty(), btn_cadastrar->Render() | flex })
        }) | border | size(WIDTH, EQUAL, 60) | size(HEIGHT, EQUAL, 40);

        return vbox({ filler(), hbox({ filler(), reg_box, filler() }), filler() });
    });

    screen.Loop(renderer);
}

// ─────────────────────────────────────────────
//  DASHBOARD (5 abas completas)
// ─────────────────────────────────────────────
void InterfaceTUI::runDashboard() {
    auto screen = ScreenInteractive::Fullscreen();

    int  tab_index   = 0;
    bool show_balance = false;

    std::vector<std::string> tab_entries = {
        " Início ", " Depósito ", " Saque ", " Transferência ", " Extrato ", " Perfil "
    };

    // ── ABA: Início ──────────────────────────────
    auto tab_inicio = Renderer([&] {
        auto acc = current_account;
        return vbox({
            filler(),
            text("BEM-VINDO, " + acc->getClient()->getName() + "!") | bold | color(Color::Cyan) | center,
            separatorEmpty(),
            hbox({
                vbox({ text(" Conta Nº:") | bold | center, text(" " + std::to_string(acc->getNumber())) | color(Color::Cyan) | center }) | flex,
                vbox({ text(" Nível:") | bold | center, text(" " + acc->getTierName()) | color(Color::Yellow) | center }) | flex,
            }),
            separatorEmpty(),
            text("Use o menu lateral ou as setas do teclado para navegar.") | dim | center,
            filler()
        }) | flex | center;
    });

    // ── ABA: Depósito ─────────────────────────────
    std::string deposit_val_str, deposit_msg;
    Component input_deposit_val = Input(&deposit_val_str, " Ex: 150.00");

    Component btn_depositar = Button(" CONFIRMAR DEPÓSITO ", [&] {
        deposit_msg = "";
        try {
            double val = std::stod(deposit_val_str);
            if (val <= 0) { deposit_msg = "Erro: Valor deve ser positivo."; return; }
            current_account->deposit(val);
            current_account->addTransaction(4, val, "Depósito em Espécie");
            bank->saveToStorage();
            deposit_msg     = "SUCESSO! Depósito de " + fmtMoney(val) + " realizado.";
            deposit_val_str = "";
        } catch (...) { deposit_msg = "Erro: Digite um valor numérico válido."; }
    });

    auto deposit_controls = Container::Vertical({ input_deposit_val, btn_depositar });

    auto tab_deposito = Renderer(deposit_controls, [&] {
        bool ok = deposit_msg.find("SUCESSO") != std::string::npos;
        auto form_box = vbox({
            text(" 💵 DEPÓSITO ") | bold | color(Color::Green) | center,
            separator(), filler(),
            text(" Valor a depositar (R$):") | bold | center,
            hbox({ input_deposit_val->Render() | flex }) | borderRounded,
            filler(),
            btn_depositar->Render() | center,
            filler(),
            text(deposit_msg) | color(ok ? Color::Green : Color::Red) | center | bold,
        }) | border | size(WIDTH, EQUAL, 45) | size(HEIGHT, EQUAL, 15);
        return center(form_box); // Centraliza o cartão bonito
    });

    // ── ABA: Saque ────────────────────────────────
    std::string withdraw_val_str, withdraw_msg;
    Component input_withdraw_val = Input(&withdraw_val_str, " Ex: 50.00");

    Component btn_sacar = Button(" CONFIRMAR SAQUE ", [&] {
        withdraw_msg = "";
        try {
            double val = std::stod(withdraw_val_str);
            if (val <= 0) { withdraw_msg = "Erro: Valor deve ser positivo."; return; }
            if (current_account->withdraw(val)) {
                current_account->addTransaction(1, val, "Saque em Dinheiro");
                bank->saveToStorage();
                withdraw_msg     = "SUCESSO! Saque de " + fmtMoney(val) + " realizado.";
                withdraw_val_str = "";
            } else {
                withdraw_msg = "Erro: Saldo insuficiente.";
            }
        } catch (...) { withdraw_msg = "Erro: Digite um valor numérico válido."; }
    });

    auto withdraw_controls = Container::Vertical({ input_withdraw_val, btn_sacar });

    auto tab_saque = Renderer(withdraw_controls, [&] {
        std::string bal = show_balance ? fmtMoney(current_account->getBalance()) : "----.--";
        bool ok = withdraw_msg.find("SUCESSO") != std::string::npos;
        auto form_box = vbox({
            text(" 🏧 SAQUE ") | bold | color(Color::RedLight) | center,
            separator(), 
            text(" Saldo Disponível: " + bal) | color(show_balance ? Color::Green : Color::GrayDark) | center,
            filler(),
            text(" Valor a sacar (R$):") | bold | center,
            hbox({ input_withdraw_val->Render() | flex }) | borderRounded,
            filler(),
            btn_sacar->Render() | center,
            filler(),
            text(withdraw_msg) | color(ok ? Color::Green : Color::Red) | center | bold,
        }) | border | size(WIDTH, EQUAL, 45) | size(HEIGHT, EQUAL, 16);
        return center(form_box);
    });

    // ── ABA: Transferência ────────────────────────
    std::string transfer_dest_str, transfer_val_str, transfer_msg;
    bool transfer_confirm_visible = false;
    std::shared_ptr<Account> transfer_dest_acc = nullptr;
    double transfer_val_pending = 0.0;

    Component input_transfer_dest = Input(&transfer_dest_str, " Conta destino");
    Component input_transfer_val  = Input(&transfer_val_str,  " Ex: 200.00");

    Component btn_verificar = Button(" VERIFICAR CONTA ", [&] {
        transfer_msg = ""; transfer_confirm_visible = false;
        try {
            int destNum = std::stoi(transfer_dest_str);
            if (destNum == current_account->getNumber()) { transfer_msg = "Erro: Não pode transferir para si mesmo."; return; }
            auto dest = bank->findAccount(destNum);
            if (!dest) { transfer_msg = "Erro: Conta destino não encontrada."; return; }
            double val = std::stod(transfer_val_str);
            if (val <= 0) { transfer_msg = "Erro: Valor deve ser positivo."; return; }
            
            transfer_dest_acc = dest; transfer_val_pending = val; transfer_confirm_visible = true;
        } catch (...) { transfer_msg = "Erro: Verifique os campos digitados."; }
    });

    Component btn_confirmar_tx = Button(" CONFIRMAR ENVIO ", [&] {
        if (!transfer_dest_acc) return;
        if (bank->transfer(current_account->getNumber(), transfer_dest_acc->getNumber(), transfer_val_pending)) {
            bank->saveToStorage();
            transfer_msg = "SUCESSO! " + fmtMoney(transfer_val_pending) + " enviado.";
        } else { transfer_msg = "Erro: Saldo ou limite insuficiente."; }
        transfer_confirm_visible = false; transfer_dest_acc = nullptr;
        transfer_dest_str = ""; transfer_val_str = "";
    });

    Component btn_cancelar_tx = Button(" CANCELAR ", [&] {
        transfer_confirm_visible = false; transfer_dest_acc = nullptr; transfer_msg = "Transferência cancelada.";
    });

    auto transfer_controls = Container::Vertical({
        input_transfer_dest, input_transfer_val, btn_verificar,
        Container::Horizontal({ btn_cancelar_tx, btn_confirmar_tx })
    });

    auto tab_transferir = Renderer(transfer_controls, [&] {
        bool ok = transfer_msg.find("SUCESSO") != std::string::npos;

        Elements content = {
            text(" 💸 TRANSFERÊNCIA BANCÁRIA ") | bold | color(Color::Cyan) | center,
            separator(), filler(),
            text(" Conta Destino:") | bold,
            hbox({ input_transfer_dest->Render() | flex }) | borderRounded,
            text(" Valor (R$):") | bold,
            hbox({ input_transfer_val->Render() | flex }) | borderRounded,
            separatorEmpty(),
            btn_verificar->Render() | center,
            filler()
        };

        if (transfer_confirm_visible && transfer_dest_acc) {
            content.push_back(vbox({
                text(" CONFIRMAÇÃO ") | bold | color(Color::Yellow) | center,
                separator(),
                text(" Para:  " + transfer_dest_acc->getClient()->getName()),
                text(" Valor: " + fmtMoney(transfer_val_pending)) | bold,
                separatorEmpty(),
                hbox({ btn_cancelar_tx->Render() | flex | color(Color::Red), btn_confirmar_tx->Render() | flex | color(Color::Green) }),
            }) | borderRounded);
        }

        content.push_back(text(transfer_msg) | color(ok ? Color::Green : Color::Red) | center | bold);

        auto form_box = vbox(std::move(content)) | border | size(WIDTH, EQUAL, 50) | size(HEIGHT, EQUAL, 40);
        return center(form_box);
    });

    // ── ABA: Extrato ──────────────────────────────
    auto tab_extrato = Renderer([&] {
        const auto& txs = current_account->getTransactions();
        Elements rows;
        
        if (txs.empty()) {
            rows.push_back(filler());
            rows.push_back(text("  Nenhuma movimentação registrada.") | dim | center);
            rows.push_back(filler());
        } else {
            rows.push_back(hbox({
                text(" Data/Hora        ") | bold | size(WIDTH, EQUAL, 18),
                text(" Tipo         ") | bold | size(WIDTH, EQUAL, 15),
                text(" Valor      ") | bold | size(WIDTH, EQUAL, 15),
                text(" Detalhes ") | bold | flex,
            }) | color(Color::Cyan));
            rows.push_back(separator());

            for (const auto& tx : txs) {
                std::string label; Color lcolor;
                switch (tx.type) {
                    case 1: label = "[SAQUE]";    lcolor = Color::Red; break;
                    case 2: label = "[ENVIO]";    lcolor = Color::Red; break;
                    case 3: label = "[RECEBIDO]"; lcolor = Color::Green; break;
                    case 4: label = "[DEPÓSITO]"; lcolor = Color::Green; break;
                    default: label = "[OUTRO]";   lcolor = Color::GrayLight; break;
                }
                rows.push_back(hbox({
                    text(fmtTimestamp(tx.timestamp)) | size(WIDTH, EQUAL, 18),
                    text(label) | color(lcolor) | size(WIDTH, EQUAL, 15),
                    text(fmtMoney(tx.amount)) | bold | size(WIDTH, EQUAL, 15),
                    text(tx.details) | flex,
                }));
            }
        }

        return vbox({
            text(" 📄 EXTRATO DE TRANSAÇÕES ") | bold | color(Color::Cyan) | center,
            separatorDouble(),
            vbox(std::move(rows)) | yframe,
        }) | flex;
    });

    // ── ABA: Perfil ───────────────────────────────
    // ── ABA: Perfil ───────────────────────────────
    bool show_score_analysis = false;
    Component btn_toggle_analysis = Button(" VER ANÁLISE DE SCORE ", [&] {
        show_score_analysis = !show_score_analysis;
    });

    auto profile_controls = Container::Vertical({ btn_toggle_analysis });

    auto tab_perfil = Renderer(profile_controls, [&] { 
        int score = current_account->calculateCreditScore();
        Color score_color = score >= 700 ? Color::Green : (score >= 300 ? Color::Yellow : Color::Red);
        std::string score_text = score >= 700 ? " (Excelente)" : (score >= 300 ? " (Bom)" : " (Baixo)");

        // 1. Criamos um vetor dinâmico de Elementos
        Elements info_elements = {
            text(" 👤 INFORMAÇÕES DO CLIENTE ") | bold | color(Color::Cyan),
            separator(),
            hbox({ text(" Titular: ") | dim, text(current_account->getClient()->getName()) | bold }),
            hbox({ text(" CPF:     ") | dim, text(current_account->getClient()->getCpf()) | bold }),
            separatorEmpty(),
            text(" 💼 DADOS BANCÁRIOS ") | bold | color(Color::Cyan),
            separator(),
            hbox({ text(" Agência: ") | dim, text("0001-9 (Sede)") | bold }),
            hbox({ text(" Conta:   ") | dim, text(std::to_string(current_account->getNumber())) | bold }),
            hbox({ text(" Nível:   ") | dim, text(current_account->getTierName()) | bold }),
            separatorEmpty(),
            hbox({ text(" Score:   ") | dim, text(std::to_string(score) + " / 1000" + score_text) | color(score_color) | bold }),
            separatorEmpty(),
            btn_toggle_analysis->Render() | size(WIDTH, EQUAL, 25)
        };

        // 2. Adicionamos a análise condicionalmente ao vetor usando push_back
        if (show_score_analysis) {
            double dep = current_account->getMonthlyDeposits();
            double wth = current_account->getMonthlyWithdrawals();
            double bal = current_account->getBalance();

            info_elements.push_back(separatorDouble());
            info_elements.push_back(text(" ANÁLISE DO SEU PERFIL FINANCEIRO:") | bold | color(Color::Yellow));
            info_elements.push_back(separatorEmpty());

            info_elements.push_back(text(" [+] O que está fortalecendo seu score:") | bold | color(Color::Green));
            bool has_good = false;
            if (dep > wth && dep > 0) {
                info_elements.push_back(text("  - Entradas maiores que saídas. Excelente controle financeiro!") | color(Color::Green));
                has_good = true;
            }
            if (bal >= 1000) {
                info_elements.push_back(text("  - Saldo alto e estável, garantindo uma boa pontuação base.") | color(Color::Green));
                has_good = true;
            }
            if (!has_good) {
                info_elements.push_back(text("  - Nenhum ponto forte detectado no momento.") | dim);
            }

            info_elements.push_back(separatorEmpty());
            info_elements.push_back(text(" [~] O que pode melhorar:") | bold | color(Color::Yellow));
            bool has_improve = false;
            if (dep > 0 && wth >= dep) {
                info_elements.push_back(text("  - Há entradas, mas saques/gastos estão equivalentes ou maiores.") | color(Color::Yellow));
                has_improve = true;
            }
            if (bal > 0 && bal < 1000) {
                info_elements.push_back(text("  - O saldo é positivo; guardar mais pode elevar o score.") | color(Color::Yellow));
                has_improve = true;
            }
            if (!has_improve) {
                info_elements.push_back(text("  - Nenhum alerta moderado no momento.") | dim);
            }

            info_elements.push_back(separatorEmpty());
            info_elements.push_back(text(" [-] O que está prejudicando seu score:") | bold | color(Color::Red));
            bool has_bad = false;
            if (dep == 0) {
                info_elements.push_back(text("  - Zero movimentações de entrada. Depósitos pesam bastante na nota.") | color(Color::Red));
                has_bad = true;
            }
            if (bal <= 0) {
                info_elements.push_back(text("  - Saldo zerado. Manter dinheiro na conta fortalece a pontuação.") | color(Color::Red));
                has_bad = true;
            }
            if (!has_bad) {
                info_elements.push_back(text("  - Nenhum fator crítico detectado.") | dim);
            }
        }

        // 3. Só agora criamos o vbox passando a lista completa
        auto info_box = vbox(std::move(info_elements));

        return vbox({ separatorEmpty(), hbox({ text("   "), info_box, filler() }), filler() });
    });
    
    // ── Tab Container + navegação ──────────────────
    Component menu = Menu(&tab_entries, &tab_index);

    Component btn_toggle_balance = Button(" VER/OCULTAR SALDO ", [&] { show_balance = !show_balance; });
    Component btn_logout = Button(" SAIR ", [&] { screen.ExitLoopClosure()(); }); // Logout só fecha o loop

    auto tab_container = Container::Tab({
        tab_inicio, tab_deposito, tab_saque, tab_transferir, tab_extrato, tab_perfil,
    }, &tab_index);

    auto right_header_controls = Container::Horizontal({ btn_toggle_balance, btn_logout });
    auto right_panel_controls  = Container::Vertical({ right_header_controls, tab_container });
    auto main_container        = Container::Horizontal({ menu, right_panel_controls });

    auto renderer = Renderer(main_container, [&] {
        auto terminal_size = Terminal::Size();
        if (terminal_size.dimx < 80 || terminal_size.dimy < 24) {
            return center(text(" Janela muito pequena para o Dashboard! Aumente o terminal. ")) | bold | color(Color::Yellow) | border;
        }

        std::string balance_str = show_balance ? fmtMoney(current_account->getBalance()) : "----.--";
        
        auto left_panel = window(text(" MENU PRINCIPAL ") | bold | center, 
            vbox({ separatorEmpty(), menu->Render(), filler() })
        ) | size(WIDTH, EQUAL, 25);

        auto right_header = window(text(" STATUS DA SESSÃO ") | center, 
            hbox({
                text(" Conta: " + std::to_string(current_account->getNumber())) | dim | flex,
                text(" Saldo: " + balance_str) | bold | color(show_balance ? Color::Green : Color::GrayDark),
                separatorEmpty(), btn_toggle_balance->Render() | color(Color::Cyan),
                separatorEmpty(), btn_logout->Render() | color(Color::Red) 
            })
        ) | size(HEIGHT, EQUAL, 5);

        auto right_content = window(text(tab_entries[tab_index]) | bold | center, tab_container->Render() | flex) | flex;
        auto right_panel = vbox({ right_header, right_content }) | flex;

        return hbox({ left_panel, right_panel }) | flex;
    });

    screen.Loop(renderer);

    // Limpeza segura da conta DEPOIS que o loop da tela encerrou (evita os crashs de Segmentation Fault)
    current_state = AppState::LOGIN;
    current_account = nullptr;
}

// ─────────────────────────────────────────────
//  Validações
// ─────────────────────────────────────────────
bool InterfaceTUI::validateCPFMath(const std::string& cpf) {
    std::string digits;
    for (char c : cpf) if (std::isdigit(c)) digits += c;
    if (digits.length() != 11) return false;
    if (std::all_of(digits.begin(), digits.end(), [&](char c){ return c == digits[0]; })) return false;

    int sum = 0;
    for (int i = 0; i < 9; ++i) sum += (digits[i] - '0') * (10 - i);
    int rem = sum % 11, d1 = (rem < 2) ? 0 : 11 - rem;
    if (digits[9] - '0' != d1) return false;

    sum = 0;
    for (int i = 0; i < 10; ++i) sum += (digits[i] - '0') * (11 - i);
    rem = sum % 11;
    int d2 = (rem < 2) ? 0 : 11 - rem;
    return digits[10] - '0' == d2;
}

bool InterfaceTUI::validatePasswordRules(const std::string& password) {
    if (password.length() < 8 || password.length() > 15) return false;
    std::regex safeRegex("^[a-zA-Z0-9]+$");
    return std::regex_match(password, safeRegex);
}
