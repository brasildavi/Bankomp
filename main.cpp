#include "Storage/DataManager.h"
#include "Control/Bank.h"
#include "Interface/InterfaceTUI.h"
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif
    
    const char* envKey = std::getenv("BANKOMP_KEY");
    
    if (!envKey) {
        std::cerr << "\033[31m[ERRO CRÍTICO] Variável de ambiente BANKOMP_KEY não configurada!\n";
        std::cerr << "O sistema não pode ser iniciado sem a chave de criptografia mestre.\033[0m\n";
        return 1;
    }

    std::string cryptoKey(envKey);

    auto dataManager = std::make_shared<DataManager>("banco.dat", cryptoKey);
    auto bank = std::make_shared<Bank>(dataManager);
    
    bank->loadFromStorage();

    std::unique_ptr<InterfaceUser> console = std::make_unique<InterfaceTUI>(bank);
    console->showMenu();

    return 0;
}