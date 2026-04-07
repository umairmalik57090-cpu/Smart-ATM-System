#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <algorithm>
using namespace std;

struct AccountNode {
    int accountNumber;
    string name;
    string pin;
    string cardType;
    bool isBlocked;
    double balance;
    AccountNode* prev;
    AccountNode* next;
    vector<string> txStack;

    AccountNode(int acc, string n, string p, string ctype, double b) {
        accountNumber = acc;
        name = n;
        pin = p;
        cardType = ctype;
        balance = b;
        isBlocked = false;
        prev = nullptr;
        next = nullptr;
    }
};

class AccountList {
private:
    AccountNode* head;
    AccountNode* tail;

public:
    AccountList() {
        head = nullptr;
        tail = nullptr;
    }

    void append(AccountNode* node) {
        if (!head) {
            head = node;
            tail = node;
        } else {
            tail->next = node;
            node->prev = tail;
            tail = node;
        }
    }

    AccountNode* find(int acc) {
        AccountNode* cur = head;
        while (cur) {
            if (cur->accountNumber == acc) return cur;
            cur = cur->next;
        }
        return nullptr;
    }

    AccountNode* getHead() { return head; }

    void loadFromFile(const string& filename) {
        ifstream fin(filename);
        if (!fin) return;
        int acc;
        string name, pin, type;
        double bal;
        while (fin >> acc >> ws && getline(fin, name, '|') && fin >> pin >> type >> bal) {
            if (!name.empty() && name[0] == ' ') name.erase(0, 1);
            AccountNode* node = new AccountNode(acc, name, pin, type, bal);
            append(node);
            string txfile = "tx_" + to_string(acc) + ".txt";
            ifstream tfin(txfile);
            string line;
            while (getline(tfin, line)) {
                if (!line.empty()) node->txStack.push_back(line);
            }
        }
    }

    void saveToFile(const string& filename) {
        ofstream fout(filename);
        AccountNode* cur = head;
        while (cur) {
            fout << cur->accountNumber << " "
                 << cur->name << "| "
                 << cur->pin << " "
                 << cur->cardType << " "
                 << cur->balance << "\n";

            string txfile = "tx_" + to_string(cur->accountNumber) + ".txt";
            ofstream tf(txfile);

            int start = max(0, (int)cur->txStack.size() - 50);
            for (int i = start; i < (int)cur->txStack.size(); i++)
                tf << cur->txStack[i] << "\n";

            cur = cur->next;
        }
    }
};
bool isMultipleOf100(long amt) {
    return amt % 100 == 0;
}
bool isMultipleOf500(long amt) {
    return amt % 500 == 0;
}
string nowString() {
    time_t t = time(nullptr);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return string(buf);
}

long getCardLimit(string card) {
    if (card == "Gold") return 20000;
    if (card == "Platinum") return 50000;
    if (card == "Diamond") return 100000;
    return 10000;
}

class SmartATM {
private:
    AccountList accounts;
    string accountsFile = "accounts.txt";
    string blockedFile = "blocked.txt";      
    string complaintsFile = "complaints.txt";  
    
    void loadBlockedFile() {
    ifstream fin(blockedFile);
    if (!fin) return;

    int acc;
    while (fin >> acc) {
        AccountNode* node = accounts.find(acc);
        if (node) node->isBlocked = true;
    }
}

        void saveBlockedFile() {
        ofstream fout(blockedFile);
        AccountNode* cur = accounts.getHead();
        while (cur) {
            if (cur->isBlocked)
                fout << cur->accountNumber << "\n";
            cur = cur->next;
        }
    }
        void saveComplaint(int acc, const string& text) {
    ofstream fout(complaintsFile, ios::app);
    fout << nowString() << " | Account: " << acc << " | " << text << "\n";
}
        void viewComplaints() {
        ifstream fin(complaintsFile);
        if (!fin) {
            cout << "No complaints found.\n";
            return;
        }

        cout << "\n--- All Complaints ---\n";
        string line;
        while (getline(fin, line)) cout << line << "\n";
    }
    void viewTransactionsAdmin() {
        cout << "Enter account number: ";
        string s; cin >> s;

        if (!all_of(s.begin(), s.end(), ::isdigit)) {
            cout << "Invalid.\n";
            return;
        }
        int acc;
        try {
            acc = stoi(s);
        } 
        catch (...) {
            cout << "Invalid number.\n";
            return;
        }
        AccountNode* node = accounts.find(acc);
        if (!node) {
            cout << "Account not found.\n";
            return;
        }

        cout << "\n--- Transactions for Account " << node->accountNumber << " ---\n";
        if (node->txStack.empty()) {
            cout << "No transactions.\n";
            return;
        }
       for (int i = 0; i < node->txStack.size(); i++){
        cout << node->txStack[i] << "\n";
        }
    }
    void blockUnblockAccount() {
    cout << "Enter account number: ";
    string s;
    cin >> s;

    if (!all_of(s.begin(), s.end(), ::isdigit)) {
        cout << "Invalid input. Digits only.\n";
        return;
    }

    int acc = stoi(s);
    AccountNode* node = accounts.find(acc);

    if (!node) {
        cout << "Account not found.\n";
        return;
    }

    cout << "\nThis account is currently: "
         << (node->isBlocked ? "BLOCKED" : "ACTIVE") << endl;

    cout << "Choose option:\n";
    cout << "1. Block this account\n";
    cout << "2. Unblock this account\n";
    cout << "Enter choice: ";

    string choiceStr;
    cin >> choiceStr;

    if (!all_of(choiceStr.begin(), choiceStr.end(), ::isdigit)) {
        cout << "Invalid option.\n";
        return;
    }

    int choice = stoi(choiceStr);

    if (choice == 1) {
        if (node->isBlocked) {
            cout << "Account is already BLOCKED.\n";
            return;
        }
        node->isBlocked = true;
        cout << "Account BLOCKED successfully.\n";

        node->txStack.push_back(nowString() + " | Admin BLOCKED the account");
    }
    else if (choice == 2) {
        if (!node->isBlocked) {
            cout << "Account is already ACTIVE.\n";
            return;
        }
        node->isBlocked = false;
        cout << "Account UNBLOCKED successfully.\n";

        node->txStack.push_back(nowString() + " | Admin UNBLOCKED the account");
    }
    else {
        cout << "Invalid choice.\n";
        return;
    }
    saveBlockedFile();
    accounts.saveToFile(accountsFile);
}

public:
    SmartATM() {
        accounts.loadFromFile(accountsFile);
         loadBlockedFile();
        seedSampleDataIfEmpty();
    }

    void seedSampleDataIfEmpty() {
        if (!accounts.find(13544)) {
            AccountNode* n = new AccountNode(13544, "Muhammad Umair", "1234", "Diamond", 400000);
            n->txStack.push_back(nowString() + " | Account created with balance 400000");
            accounts.append(n);
            accounts.saveToFile(accountsFile);
        }
    }

    AccountNode* authenticate() {
    cout << "Enter Account Number: ";
    string accStr;
    cin >> accStr;
    if (!all_of(accStr.begin(), accStr.end(), ::isdigit)) {
        cout << "Invalid input. Only digits allowed.\n";
        return nullptr;
    }
    int acc;
    try {
        acc = stoi(accStr);
    }
    catch (...) {
        cout << "Invalid account number.\n";
        return nullptr;
    }
    AccountNode* user = accounts.find(acc);
    if (!user) {
        cout << "Account not found.\n";
        return nullptr;
    }
    if (user->isBlocked) {
        cout << "\n⚠ YOUR ACCOUNT IS BLOCKED.\n";
        cout << "Please contact the admin to reactivate your account.\n";
        return nullptr;
    }
    cout << "Enter PIN: ";
    string pin;
    cin >> pin;
    if (pin != user->pin) {
        cout << "Incorrect PIN.\n";
        return nullptr;
    }
    cout << "Login Successful!\n";
    return user;
}
    void customerMenu(AccountNode* user) {
        while (true) {
            cout << "\n------ CUSTOMER MENU ------\n";
            cout << "1. View Account Info\n2. Deposit\n3. Withdraw\n4. Mini Statement\n5. Change PIN\n6. Submit Complain \n7. Logout\nEnter choice: ";
            string chStr;
            cin >> chStr;
            if (!all_of(chStr.begin(), chStr.end(), ::isdigit)) {
                cout << "Invalid input. Only digits allowed.\n";
                continue;
            }
            int ch;
        try {
            ch = stoi(chStr);
        }
        catch (...) {
            cout << "Invalid option. Number too large.\n";
            continue;
        }
            switch (ch) {
                case 1: viewInfo(user); break;
                case 2: deposit(user); break;
                case 3: withdraw(user); break;
                case 4: miniStatement(user); break;
                case 5: changePIN(user); break;
                case 6: {
                        cin.ignore();
                        string comp;
                        cout << "Enter complaint: ";
                        getline(cin, comp);
                        saveComplaint(user->accountNumber, comp);
                        cout << "Complaint submitted.\n";
                    break;
                }
                case 7: return;
                default: cout << "Invalid option.\n";
            }
        }
    }

    void viewInfo(AccountNode* user) {
        cout << "\nAccount#: " << user->accountNumber
             << "\nName: " << user->name
             << "\nCard Type: " << user->cardType
             << "\nBalance: " << user->balance << "\n";
    }

    void deposit(AccountNode* user) {
    	if (user->isBlocked) {
    cout << "Account is BLOCKED. Contact admin.\n";
    return;
}
    cout << "Enter amount to deposit: ";
    string amtStr;
    cin >> amtStr;

    if (!all_of(amtStr.begin(), amtStr.end(), ::isdigit)) {
        cout << "Invalid input. Only numbers allowed.\n";
        return;
    }
    long amt;
 try {
            amt = stol(amtStr);
        }
        catch (...) {
            cout << "Invalid choice. Number too large.\n";
            return;
        }
    if (amt < 500) {
        cout << "Minimum deposit amount is 500.\n";
        return;
    }
    if (!isMultipleOf100(amt)) {
    cout << "Amount must be in multiples of 100 (e.g., 500, 600, 700).\n";
    return;
}
    user->balance += amt;
    user->txStack.push_back(nowString() + " | Deposit: " + to_string(amt));

    accounts.saveToFile(accountsFile);

    cout << "Deposit Successful!\n";
}

    void withdraw(AccountNode* user) {
    	if (user->isBlocked) {
    cout << "Account is BLOCKED. Contact admin.\n";
    return;
}
        cout << "Enter amount to withdraw: ";
        string amtStr;
        cin >> amtStr;
        if (!all_of(amtStr.begin(), amtStr.end(), ::isdigit)) {
            cout << "Invalid input. Only numbers allowed.\n";
            return;
        }
            long amt;
        try {
            amt = stol(amtStr);
        }
        catch (...) {
            cout << "Invalid choice. Number too large.\n";
            return;
        }
         if (amt < 500) {
        cout << "Minimum widraw amount is 500.\n";
        return;
    }
    if (!isMultipleOf500(amt)) {
    cout << "Amount must be in multiples of 500 (e.g., 500, 1000, 1500).\n";
    return;
}
        long limit = getCardLimit(user->cardType);
        if (amt > limit) {
            cout << "Withdraw limit exceeded! Limit: " << limit << "\n";
            return;
        }

        if (amt > user->balance) {
            cout << "Insufficient balance.\n";
            return;
        }

        user->balance -= amt;
        user->txStack.push_back(nowString() + " | Withdraw: " + to_string(amt));

        accounts.saveToFile(accountsFile);

        cout << "Withdraw Successful!\n";
    }

    void miniStatement(AccountNode* user) {
        cout << "\n--- Last 10 Transactions ---\n";
        int start = max(0, (int)user->txStack.size() - 10);
        for (int i = start; i < (int)user->txStack.size(); i++)
            cout << user->txStack[i] << "\n";
        if (user->txStack.empty()) cout << "No transactions.\n";
    }

    void changePIN(AccountNode* user) {
    if (user->isBlocked) {
        cout << "Your account is BLOCKED. Contact admin.\n";
        return;
    }
    int attempts = 3;
    string oldPin;
    cin.ignore();
    while (attempts-- > 0) {
        cout << "Enter old PIN: ";
        getline(cin, oldPin);

        if (oldPin == user->pin && oldPin.length() == 4 &&
            all_of(oldPin.begin(), oldPin.end(), ::isdigit)) 
        {
            string newPin, confirmPin;
            cout << "Enter new 4-digit PIN: ";
            cin >> newPin;

            if (newPin.length() != 4 || !all_of(newPin.begin(), newPin.end(), ::isdigit)) {
                cout << "PIN must be 4 digits.\n";
                return;
            }

            cout << "Re-enter new PIN: ";
            cin >> confirmPin;

            if (confirmPin != newPin) {
                cout << "PINs do not match.\n";
                return;
            }
            user->pin = newPin;
            user->txStack.push_back(nowString() + " | PIN Changed Successfully");
            accounts.saveToFile(accountsFile);

            cout << "PIN Updated Successfully!\n";
            return;
        }
        cout << "Incorrect PIN.\n";
        cout << attempts << " attempt(s) remaining.\n";
    }
    cout << "\nYou entered the wrong PIN 3 times.\n";
    cout << "Your account has been BLOCKED for security reasons.\n";
    user->isBlocked = true;
    user->txStack.push_back(nowString() + " | Account BLOCKED due to 3 wrong PIN attempts");
    saveBlockedFile();                
    accounts.saveToFile(accountsFile);
}
    void adminLogin() {
        string id, pass;
        cout << "Enter Admin ID: ";
        cin >> id;
        cout << "Enter Admin Password: ";
        cin >> pass;

        if ((id == "admin" || id=="Admin") && pass == "12345") {
            cout << "Admin Login Successful!\n";
            adminMenu();
        } else cout << "Invalid Admin Credentials.\n";
    }

    void adminMenu() {
    while (true) {
        cout << "\n------ ADMIN MENU ------\n";
        cout << "1. View All Accounts\n";
        cout << "2. View Account Transactions\n";
        cout << "3. Block/Unblock Account\n";
        cout << "4. View Complaints\n";
        cout << "5. Create New Account\n";
        cout << "6. Logout\n";
        cout << "Enter choice: ";

        string s;
        cin >> s;

        if (!all_of(s.begin(), s.end(), ::isdigit)) {
            cout << "Invalid.\n";
            continue;
        }

        int ch = stoi(s);

        switch (ch) {
        case 1: viewAllAccounts(); break;
        case 2: viewTransactionsAdmin(); break;
        case 3: blockUnblockAccount(); break;
        case 4: viewComplaints(); break;
        case 5: createAccount(); break;
        case 6: return;
        default: cout << "Invalid.\n";
        }
    }
}
    void viewAllAccounts() {
        AccountNode* cur = accounts.getHead();
        cout << "\n--- All Accounts ---\n";
        while (cur) {
            cout << "Account#: " << cur->accountNumber
                 << " | Name: " << cur->name
                 << " | Card: " << cur->cardType
                 << " | Balance: " << cur->balance << endl;
            cur = cur->next;
        }
    }

    void createAccount() {
        int acc;
        string name, pin, type, balStr;

        cout << "Enter new account number: ";
        string accStr;
        cin >> accStr;
        if (!all_of(accStr.begin(), accStr.end(), ::isdigit)) {
            cout << "Invalid input. Digits only.\n";
            return;
        }
        try {
             acc = stoi(accStr);
    } 
catch (...) {
    cout << "Account number too large. Enter a valid number.\n";
    return;
}
        cin.ignore();
        cout << "Enter name: ";
        getline(cin, name);
bool validName = true;
for (char c : name) {
    if (!isalpha(c) && c != ' ') {
        validName = false;
        break;
    }
}
if (!validName || name.empty()) {
    cout << "Invalid Name. Only alphabets allowed.\n";
    return;
}
        cout << "Enter PIN: ";
        cin >> pin;
        if (pin.length() != 4 || !all_of(pin.begin(), pin.end(), ::isdigit)) {
            cout << "PIN must be 4 digits.\n";
            return;
        }

        cout << "Enter card type: ";
        cin >> type;
        if (type != "Gold" && type != "Platinum" && type != "Diamond") {
            cout << "Invalid card type.\n";
            return;
        }
        cout << "Enter starting balance: ";
        cin >> balStr;
        if (!all_of(balStr.begin(), balStr.end(), ::isdigit)) {
            cout << "Invalid input. Only numbers allowed.\n";
            return;
        }
          long bal;
         try {
            bal = stol(balStr);
        }
        catch (...) {
            cout << "Invalid choice. Number too large.\n";
            return;
        }
        AccountNode* n = new AccountNode(acc, name, pin, type, bal);
        n->txStack.push_back(nowString() + " | Account Created");
        accounts.append(n);
        accounts.saveToFile(accountsFile);

        cout << "New account created.\n";
    }

    void shutdown() {
        accounts.saveToFile(accountsFile);
        cout << "All data saved successfully.\n";
    }

    void run() {
        while (true) {
            cout << "\n----- SMART ATM -----\n1. Customer Login\n2. Admin Login\n3. Exit\nEnter choice: ";
            string chStr;
            cin >> chStr;
            if (!all_of(chStr.begin(), chStr.end(), ::isdigit)) {
                cout << "Invalid input. Only digits allowed.\n";
                continue;
            }
             int ch;
        try {
            ch = stoi(chStr);
        } 
        catch (...) {
            cout << "Invalid choice. Number too large.\n";
            continue;
        }
            switch (ch) {
                case 1: {
                    AccountNode* user = authenticate();
                    if (user) customerMenu(user);
                    break;
                }
                case 2: adminLogin(); break;
                case 3: shutdown(); return;
                default: cout << "Invalid option.\n";
            }
        }
    }
};

int main() {
    SmartATM atm;
    atm.run();
    return 0;
}