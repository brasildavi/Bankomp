#ifndef INTERFACE_TUI_H
#define INTERFACE_TUI_H

#include "Interface/InterfaceUser.h"
#include "Control/Bank.h"
#include <memory>
#include <string>

enum class AppState { LOGIN, REGISTER, DASHBOARD, EXIT };

class InterfaceTUI : public InterfaceUser {
private:
    std::shared_ptr<Bank> bank;
    std::shared_ptr<Account> current_account = nullptr;
    
    AppState current_state = AppState::LOGIN;
    int login_attempts = 0;

    void runLoginScreen();
    void runRegisterScreen();
    void runDashboard();

    bool validateCPFMath(const std::string& cpf);
    bool validatePasswordRules(const std::string& password);

public:
    InterfaceTUI(std::shared_ptr<Bank> bank_ptr);
    void showMenu() override;
};

#endif