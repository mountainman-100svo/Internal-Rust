/*
    Console Banking System (C++17)
    --------------------------------
    Features:
    - Create accounts
    - Deposit / Withdraw
    - Transfer between accounts
    - Transaction history
    - Persistent storage (file-based)
*/

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>

using namespace std;

// ========================================
// Utility
// ========================================

string currentTime()
{
    time_t now = time(nullptr);
    tm* ltm = localtime(&now);

    stringstream ss;
    ss << 1900 + ltm->tm_year << "-"
       << setw(2) << setfill('0') << 1 + ltm->tm_mon << "-"
       << setw(2) << setfill('0') << ltm->tm_mday << " "
       << setw(2) << setfill('0') << ltm->tm_hour << ":"
       << setw(2) << setfill('0') << ltm->tm_min << ":"
       << setw(2) << setfill('0') << ltm->tm_sec;

    return ss.str();
}

// ========================================
// Transaction
// ========================================

struct Transaction
{
    string timestamp;
    string type;
    double amount;

    string serialize() const
    {
        stringstream ss;
        ss << timestamp << "|" << type << "|" << amount;
        return ss.str();
    }

    static Transaction deserialize(const string& line)
    {
        stringstream ss(line);
        string token;

        Transaction t;
        getline(ss, t.timestamp, '|');
        getline(ss, t.type, '|');
        getline(ss, token, '|');
        t.amount = stod(token);

        return t;
    }
};

// ========================================
// Account
// ========================================

class Account
{
private:
    int id;
    string owner;
    double balance;
    vector<Transaction> history;

public:
    Account() : id(0), balance(0.0) {}

    Account(int id, const string& owner)
        : id(id), owner(owner), balance(0.0) {}

    int getId() const { return id; }
    string getOwner() const { return owner; }
    double getBalance() const { return balance; }

    void deposit(double amount)
    {
        balance += amount;
        history.push_back({currentTime(), "DEPOSIT", amount});
    }

    bool withdraw(double amount)
    {
        if (amount > balance)
            return false;

        balance -= amount;
        history.push_back({currentTime(), "WITHDRAW", amount});
        return true;
    }

    void transferOut(double amount)
    {
        balance -= amount;
        history.push_back({currentTime(), "TRANSFER_OUT", amount});
    }

    void transferIn(double amount)
    {
        balance += amount;
        history.push_back({currentTime(), "TRANSFER_IN", amount});
    }

    void printSummary() const
    {
        cout << "ID: " << id
             << " | Owner: " << owner
             << " | Balance: $" << fixed << setprecision(2)
             << balance << endl;
    }

    void printHistory() const
    {
        cout << "\n--- Transaction History ---\n";
        for (const auto& t : history)
        {
            cout << t.timestamp << " | "
                 << setw(15) << left << t.type
                 << " | $" << fixed << setprecision(2)
                 << t.amount << endl;
        }
    }

    string serialize() const
    {
        stringstream ss;
        ss << id << ";" << owner << ";" << balance << endl;

        for (const auto& t : history)
        {
            ss << "T:" << t.serialize() << endl;
        }

        ss << "END" << endl;
        return ss.str();
    }

    static Account deserialize(ifstream& file, const string& header)
    {
        stringstream ss(header);
        string token;

        getline(ss, token, ';');
        int id = stoi(token);

        string owner;
        getline(ss, owner, ';');

        getline(ss, token, ';');
        double balance = stod(token);

        Account acc(id, owner);
        acc.balance = balance;

        string line;
        while (getline(file, line))
        {
            if (line == "END")
                break;

            if (line.rfind("T:", 0) == 0)
            {
                string data = line.substr(2);
                acc.history.push_back(Transaction::deserialize(data));
            }
        }

        return acc;
    }
};

// ========================================
// Bank System
// ========================================

class Bank
{
private:
    vector<Account> accounts;
    int nextId = 1;
    const string filename = "bank_data.txt";

public:
    Bank()
    {
        load();
    }

    ~Bank()
    {
        save();
    }

    void createAccount()
    {
        string name;
        cin.ignore();
        cout << "Owner name: ";
        getline(cin, name);

        accounts.emplace_back(nextId++, name);
        cout << "Account created successfully.\n";
    }

    Account* findAccount(int id)
    {
        for (auto& acc : accounts)
        {
            if (acc.getId() == id)
                return &acc;
        }
        return nullptr;
    }

    void deposit()
    {
        int id;
        double amount;

        cout << "Account ID: ";
        cin >> id;
        cout << "Amount: ";
        cin >> amount;

        Account* acc = findAccount(id);
        if (!acc)
        {
            cout << "Account not found.\n";
            return;
        }

        acc->deposit(amount);
        cout << "Deposit successful.\n";
    }

    void withdraw()
    {
        int id;
        double amount;

        cout << "Account ID: ";
        cin >> id;
        cout << "Amount: ";
        cin >> amount;

        Account* acc = findAccount(id);
        if (!acc)
        {
            cout << "Account not found.\n";
            return;
        }

        if (!acc->withdraw(amount))
        {
            cout << "Insufficient funds.\n";
        }
        else
        {
            cout << "Withdrawal successful.\n";
        }
    }

    void transfer()
    {
        int from, to;
        double amount;

        cout << "From ID: ";
        cin >> from;
        cout << "To ID: ";
        cin >> to;
        cout << "Amount: ";
        cin >> amount;

        Account* accFrom = findAccount(from);
        Account* accTo = findAccount(to);

        if (!accFrom || !accTo)
        {
            cout << "Invalid account ID.\n";
            return;
        }

        if (accFrom->getBalance() < amount)
        {
            cout << "Insufficient funds.\n";
            return;
        }

        accFrom->transferOut(amount);
        accTo->transferIn(amount);

        cout << "Transfer completed.\n";
    }

    void listAccounts() const
    {
        cout << "\n--- Accounts ---\n";
        for (const auto& acc : accounts)
        {
            acc.printSummary();
        }
    }

    void showHistory()
    {
        int id;
        cout << "Account ID: ";
        cin >> id;

        Account* acc = findAccount(id);
        if (!acc)
        {
            cout << "Account not found.\n";
            return;
        }

        acc->printHistory();
    }

    void save()
    {
        ofstream file(filename);

        for (const auto& acc : accounts)
        {
            file << acc.serialize();
        }

        file.close();
    }

    void load()
    {
        ifstream file(filename);
        if (!file.is_open())
            return;

        string line;
        while (getline(file, line))
        {
            if (line.empty())
                continue;

            Account acc = Account::deserialize(file, line);
            accounts.push_back(acc);
            nextId = max(nextId, acc.getId() + 1);
        }

        file.close();
    }

    void menu()
    {
        cout << "\n=== Console Banking System ===\n";
        cout << "1. Create Account\n";
        cout << "2. Deposit\n";
        cout << "3. Withdraw\n";
        cout << "4. Transfer\n";
        cout << "5. List Accounts\n";
        cout << "6. Show History\n";
        cout << "0. Exit\n";
        cout << "Select: ";
    }

    void run()
    {
        int choice;

        while (true)
        {
            menu();
            cin >> choice;

            switch (choice)
            {
            case 1: createAccount(); break;
            case 2: deposit(); break;
            case 3: withdraw(); break;
            case 4: transfer(); break;
            case 5: listAccounts(); break;
            case 6: showHistory(); break;
            case 0:
                save();
                cout << "Goodbye.\n";
                return;
            default:
                cout << "Invalid choice.\n";
            }
        }
    }
};

// ========================================
// Main
// ========================================

int main()
{
    Bank bank;
    bank.run();
    return 0;
}
