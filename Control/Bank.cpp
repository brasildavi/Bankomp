#include "Bank.h"
#include "../Model/CurrentAccount.h"
#include "../Model/SavingAccount.h"
#include "../Storage/Cryptography.h"
#include <cstring>
#include <ctime>

Bank::Bank(std::shared_ptr<DataManager> dm) : dataManager(dm) {}

void Bank::loadFromStorage() {
    accounts.clear();
    auto records = dataManager->loadRecords();
    for (const auto& rec : records) {
        auto client = std::make_shared<Client>(rec.clientName, rec.clientCpf);
        if (rec.accountType == 1) {
            accounts.push_back(std::make_shared<CurrentAccount>(rec.accountNumber, rec.balance, client, rec.passwordHash, rec.specialLimitOrInterest));
        } else {
            accounts.push_back(std::make_shared<SavingAccount>(rec.accountNumber, rec.balance, client, rec.passwordHash, rec.specialLimitOrInterest));
        }
    }

    auto txRecords = dataManager->loadTransactions();
    for (const auto& rec : txRecords) {
        auto acc = findAccount(rec.accountNumber);
        if (acc) {
            // Agora passamos o timestamp recuperado do arquivo binário
            acc->addTransaction(rec.type, rec.amount, rec.details, rec.timestamp); 
        }
    }
}

void Bank::saveToStorage() {
    std::vector<RecordData> records;
    for (const auto& acc : accounts) {
        RecordData rec;
        std::strncpy(rec.clientName, acc->getClient()->getName().c_str(), sizeof(rec.clientName) - 1);
        rec.clientName[sizeof(rec.clientName) - 1] = '\0';
        
        std::strncpy(rec.clientCpf, acc->getClient()->getCpf().c_str(), sizeof(rec.clientCpf) - 1);
        rec.clientCpf[sizeof(rec.clientCpf) - 1] = '\0';
        
        rec.accountNumber = acc->getNumber();
        rec.balance = acc->getBalance();
        rec.accountType = acc->getType();
        rec.specialLimitOrInterest = acc->getSpecialAttribute();
        
        std::strncpy(rec.passwordHash, acc->getPasswordHash().c_str(), sizeof(rec.passwordHash) - 1);
        rec.passwordHash[sizeof(rec.passwordHash) - 1] = '\0';
        
        records.push_back(rec);
    }
    dataManager->saveRecords(records);

    std::vector<TransactionRecord> txRecords;
    for (const auto& acc : accounts) {
        for (const auto& tx : acc->getTransactions()) {
            TransactionRecord rec;
            rec.accountNumber = acc->getNumber();
            rec.type = tx.type;
            rec.amount = tx.amount;
            std::strncpy(rec.details, tx.details.c_str(), sizeof(rec.details) - 1);
            rec.details[sizeof(rec.details) - 1] = '\0';
            rec.timestamp = tx.timestamp; // Salva o timestamp na estrutura de I/O
            txRecords.push_back(rec);
        }
    }
    dataManager->saveTransactions(txRecords);
}

bool Bank::isCpfRegistered(const std::string& cpf) const {
    for (const auto& acc : accounts) {
        if (acc->getClient()->getCpf() == cpf) return true;
    }
    return false;
}

int Bank::createAccount(const std::string& name, const std::string& cpf, int type, double initialDeposit, const std::string& password, double income) {
    int number = 1000 + accounts.size(); 
    auto client = std::make_shared<Client>(name, cpf);
    std::string hash = Cryptography::generateHash(password);
    
    int tier = 1; 
    double specialAttr = 0.0;

    if (income > 0) {
        if (income >= 20000) tier = 5;       
        else if (income >= 10000) tier = 4;  
        else if (income >= 3000) tier = 3;   
        else tier = 2;                       
    }

    if (type == 1) {
        if (tier == 5) specialAttr = income * 2.0;       
        else if (tier == 4) specialAttr = income * 1.5;  
        else if (tier == 3) specialAttr = income * 1.0; 
        else if (tier == 2) specialAttr = income * 0.5;  
        else specialAttr = 0.0;

        accounts.push_back(std::make_shared<CurrentAccount>(number, initialDeposit, client, hash, specialAttr, tier, income));
    } else {
        if (tier == 5) specialAttr = 0.10;       
        else if (tier == 4) specialAttr = 0.08;  
        else if (tier == 3) specialAttr = 0.06;  
        else specialAttr = 0.05;                 

        accounts.push_back(std::make_shared<SavingAccount>(number, initialDeposit, client, hash, specialAttr, tier, income));
    }
    
    saveToStorage();
    return number;
}

std::shared_ptr<Account> Bank::authenticate(int number, const std::string& password) {
    auto acc = findAccount(number);
    if (acc && acc->getPasswordHash() == Cryptography::generateHash(password)) {
        return acc;
    }
    return nullptr;
}

std::shared_ptr<Account> Bank::findAccount(int number) {
    for (auto acc : accounts) {
        if (acc->getNumber() == number) return acc;
    }
    return nullptr;
}

bool Bank::transfer(int srcNumber, int destNumber, double amount) {
    if (amount <= 0 || srcNumber == destNumber) return false;
    auto src = findAccount(srcNumber);
    auto dest = findAccount(destNumber);
    if (!src || !dest) return false;

    if (src->withdraw(amount)) {
        dest->deposit(amount);
        src->addTransaction(2, amount, "Para: " + dest->getClient()->getName() + " (" + std::to_string(destNumber) + ")");
        dest->addTransaction(3, amount, "De: " + src->getClient()->getName() + " (" + std::to_string(srcNumber) + ")");
        saveToStorage();
        return true;
    }
    return false;
}