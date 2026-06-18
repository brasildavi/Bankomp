#ifndef INTERFACE_CONSOLE_H
#define INTERFACE_CONSOLE_H
#include "InterfaceUser.h"
#include "../Control/Bank.h"
#include <memory>

class InterfaceConsole : public InterfaceUser {
private:
    std::shared_ptr<Bank> bank;
    void handleLogin();
    void handleCreateAccount();
    void handleUserSession(std::shared_ptr<Account> acc);
    void handleViewProfile(std::shared_ptr<Account> acc);
    bool validateCPFMath(const std::string& cpf);
    bool validatePasswordRules(const std::string& password);
public:
    InterfaceConsole(std::shared_ptr<Bank> b);
    void showMenu() override;
    void setStdinEcho(bool enable);
};

#endif