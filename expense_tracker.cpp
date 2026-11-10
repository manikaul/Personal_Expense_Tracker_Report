// ============================================================
//  PERSONAL EXPENSE TRACKER  (with unique "Regret Index" feature)
//  Language : C++11 or later
//  Compile  : g++ -std=c++11 -o expense_tracker expense_tracker.cpp
//  Run      : ./expense_tracker
// ============================================================
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <ctime>
using namespace std;

const string DATA_FILE   = "expenses.txt";
const string BUDGET_FILE = "budget.txt";

// ---------------- Data model ----------------
struct Expense {
    int    id;
    string date;        // YYYY-MM-DD
    string category;
    string description;
    double amount;
    int    satisfaction; // UNIQUE FEATURE: 1 (big regret) .. 5 (totally worth it)
};

vector<Expense> expenses;
double monthlyBudget = 0.0;
int nextId = 1;

// ---------------- Input helpers ----------------
void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int readInt(const string& prompt, int lo, int hi) {
    int v;
    while (true) {
        cout << prompt;
        if (cin >> v && v >= lo && v <= hi) { clearInput(); return v; }
        cout << "  Invalid input. Enter a number between " << lo << " and " << hi << ".\n";
        clearInput();
    }
}

double readPositiveDouble(const string& prompt) {
    double v;
    while (true) {
        cout << prompt;
        if (cin >> v && v > 0) { clearInput(); return v; }
        cout << "  Invalid amount. Enter a positive number.\n";
        clearInput();
    }
}

string readLine(const string& prompt) {
    string s;
    while (true) {
        cout << prompt;
        getline(cin, s);
        if (!s.empty()) return s;
        cout << "  Input cannot be empty.\n";
    }
}

bool validDate(const string& d) {
    if (d.size() != 10 || d[4] != '-' || d[7] != '-') return false;
    for (size_t i = 0; i < d.size(); i++)
        if (i != 4 && i != 7 && !isdigit((unsigned char)d[i])) return false;
    int m = stoi(d.substr(5, 2)), day = stoi(d.substr(8, 2));
    return m >= 1 && m <= 12 && day >= 1 && day <= 31;
}

string readDate() {
    string d;
    while (true) {
        cout << "Date (YYYY-MM-DD) or 't' for today: ";
        getline(cin, d);
        if (d == "t" || d == "T") {
            time_t now = time(0);
            char buf[11];
            strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&now));
            return string(buf);
        }
        if (validDate(d)) return d;
        cout << "  Invalid date format.\n";
    }
}

string currentMonth() {
    time_t now = time(0);
    char buf[8];
    strftime(buf, sizeof(buf), "%Y-%m", localtime(&now));
    return string(buf);
}

// Replace '|' so the file format never breaks
string sanitize(string s) {
    replace(s.begin(), s.end(), '|', '/');
    return s;
}

// ---------------- File handling ----------------
void saveData() {
    ofstream out(DATA_FILE.c_str());
    for (size_t i = 0; i < expenses.size(); i++) {
        const Expense& e = expenses[i];
        out << e.id << '|' << e.date << '|' << e.category << '|'
            << e.description << '|' << e.amount << '|' << e.satisfaction << '\n';
    }
    ofstream b(BUDGET_FILE.c_str());
    b << monthlyBudget << '\n';
}

void loadData() {
    ifstream in(DATA_FILE.c_str());
    string line;
    while (getline(in, line)) {
        stringstream ss(line);
        string id, date, cat, desc, amt, sat;
        if (getline(ss, id, '|') && getline(ss, date, '|') && getline(ss, cat, '|') &&
            getline(ss, desc, '|') && getline(ss, amt, '|') && getline(ss, sat, '|')) {
            Expense e;
            e.id = atoi(id.c_str());
            e.date = date; e.category = cat; e.description = desc;
            e.amount = atof(amt.c_str());
            e.satisfaction = atoi(sat.c_str());
            expenses.push_back(e);
            if (e.id >= nextId) nextId = e.id + 1;
        }
    }
    ifstream b(BUDGET_FILE.c_str());
    if (b) b >> monthlyBudget;
}

// ---------------- Display helpers ----------------
void printHeader() {
    cout << left << setw(5) << "ID" << setw(12) << "Date" << setw(15) << "Category"
         << setw(26) << "Description" << right << setw(11) << "Amount" << setw(8) << "Rating" << "\n";
    cout << string(77, '-') << "\n";
}

void printExpense(const Expense& e) {
    string d = e.description.size() > 24 ? e.description.substr(0, 23) + "~" : e.description;
    cout << left << setw(5) << e.id << setw(12) << e.date << setw(15) << e.category
         << setw(26) << d << right << setw(11) << fixed << setprecision(2) << e.amount
         << setw(6) << e.satisfaction << "/5\n";
}

string chooseCategory() {
    const char* cats[] = {"Food", "Transport", "Shopping", "Bills", "Health",
                          "Entertainment", "Education", "Other"};
    cout << "\nCategories:\n";
    for (int i = 0; i < 8; i++) cout << "  " << (i + 1) << ". " << cats[i] << "\n";
    int c = readInt("Choose category (1-8): ", 1, 8);
    return cats[c - 1];
}

double monthTotal(const string& ym) {
    double t = 0;
    for (size_t i = 0; i < expenses.size(); i++)
        if (expenses[i].date.substr(0, 7) == ym) t += expenses[i].amount;
    return t;
}

void budgetAlert() {
    if (monthlyBudget <= 0) return;
    double spent = monthTotal(currentMonth());
    double pct = spent / monthlyBudget * 100.0;
    if (pct >= 100)
        cout << "\n  [!!] BUDGET EXCEEDED: spent " << spent << " of " << monthlyBudget
             << " (" << (int)pct << "%)\n";
    else if (pct >= 80)
        cout << "\n  [!] Warning: " << (int)pct << "% of monthly budget used ("
             << spent << " / " << monthlyBudget << ")\n";
}

// ---------------- Core features ----------------
void addExpense() {
    cout << "\n--- ADD EXPENSE ---\n";
    Expense e;
    e.id = nextId++;
    e.date = readDate();
    e.category = chooseCategory();
    e.description = sanitize(readLine("Description: "));
    e.amount = readPositiveDouble("Amount: ");
    cout << "\nHow worth it was this? (1 = total regret ... 5 = absolutely worth it)\n";
    e.satisfaction = readInt("Satisfaction rating (1-5): ", 1, 5);
    expenses.push_back(e);
    saveData();
    cout << "\nExpense added successfully (ID " << e.id << ").\n";
    budgetAlert();
}

void viewAll() {
    cout << "\n--- ALL EXPENSES ---\n";
    if (expenses.empty()) { cout << "No expenses recorded yet.\n"; return; }
    vector<Expense> sorted = expenses;
    sort(sorted.begin(), sorted.end(),
         [](const Expense& a, const Expense& b) { return a.date < b.date; });
    printHeader();
    double total = 0;
    for (size_t i = 0; i < sorted.size(); i++) { printExpense(sorted[i]); total += sorted[i].amount; }
    cout << string(77, '-') << "\n";
    cout << "TOTAL: " << fixed << setprecision(2) << total << "\n";
}

void deleteExpense() {
    cout << "\n--- DELETE EXPENSE ---\n";
    if (expenses.empty()) { cout << "Nothing to delete.\n"; return; }
    int id = readInt("Enter expense ID to delete: ", 1, 1000000);
    for (size_t i = 0; i < expenses.size(); i++) {
        if (expenses[i].id == id) {
            expenses.erase(expenses.begin() + i);
            saveData();
            cout << "Expense deleted.\n";
            return;
        }
    }
    cout << "No expense with that ID.\n";
}

void editExpense() {
    cout << "\n--- EDIT EXPENSE ---\n";
    if (expenses.empty()) { cout << "Nothing to edit.\n"; return; }
    int id = readInt("Enter expense ID to edit: ", 1, 1000000);
    for (size_t i = 0; i < expenses.size(); i++) {
        if (expenses[i].id == id) {
            cout << "Editing: "; printExpense(expenses[i]);
            expenses[i].description = sanitize(readLine("New description: "));
            expenses[i].amount = readPositiveDouble("New amount: ");
            expenses[i].satisfaction = readInt("New satisfaction (1-5): ", 1, 5);
            saveData();
            cout << "Expense updated.\n";
            return;
        }
    }
    cout << "No expense with that ID.\n";
}

void searchExpenses() {
    cout << "\n--- SEARCH ---\n";
    string key = readLine("Keyword (description/category): ");
    transform(key.begin(), key.end(), key.begin(), ::tolower);
    bool found = false;
    for (size_t i = 0; i < expenses.size(); i++) {
        string d = expenses[i].description + " " + expenses[i].category;
        transform(d.begin(), d.end(), d.begin(), ::tolower);
        if (d.find(key) != string::npos) {
            if (!found) printHeader();
            printExpense(expenses[i]);
            found = true;
        }
    }
    if (!found) cout << "No matching expenses.\n";
}

void categorySummary() {
    cout << "\n--- CATEGORY SUMMARY ---\n";
    if (expenses.empty()) { cout << "No data.\n"; return; }
    map<string, double> totals;
    double grand = 0;
    for (size_t i = 0; i < expenses.size(); i++) {
        totals[expenses[i].category] += expenses[i].amount;
        grand += expenses[i].amount;
    }
    for (map<string, double>::iterator it = totals.begin(); it != totals.end(); ++it) {
        double pct = it->second / grand * 100.0;
        cout << left << setw(15) << it->first << right << setw(12) << fixed << setprecision(2)
             << it->second << setw(8) << setprecision(1) << pct << "%  "
             << string((int)(pct / 4), '#') << "\n";
    }
    cout << "Grand total: " << fixed << setprecision(2) << grand << "\n";
}

void monthlyReport() {
    cout << "\n--- MONTHLY REPORT ---\n";
    string ym = readLine("Month (YYYY-MM): ");
    double total = 0; int count = 0;
    map<string, double> cat;
    for (size_t i = 0; i < expenses.size(); i++) {
        if (expenses[i].date.substr(0, 7) == ym) {
            total += expenses[i].amount; count++;
            cat[expenses[i].category] += expenses[i].amount;
        }
    }
    if (count == 0) { cout << "No expenses in " << ym << ".\n"; return; }
    cout << "Transactions : " << count << "\n";
    cout << "Total spent  : " << fixed << setprecision(2) << total << "\n";
    cout << "Average/item : " << total / count << "\n";
    string top; double mx = 0;
    for (map<string, double>::iterator it = cat.begin(); it != cat.end(); ++it)
        if (it->second > mx) { mx = it->second; top = it->first; }
    cout << "Top category : " << top << " (" << mx << ")\n";
    if (monthlyBudget > 0)
        cout << "Budget left  : " << monthlyBudget - total << "\n";
}

void setBudget() {
    cout << "\n--- SET MONTHLY BUDGET ---\n";
    monthlyBudget = readPositiveDouble("Monthly budget amount: ");
    saveData();
    cout << "Budget set to " << monthlyBudget << ".\n";
}

// ============================================================
//  UNIQUE FEATURE: REGRET INDEX ANALYSER
//  Idea : every expense carries a 1-5 "worth it" rating.
//         Spending rated 1 or 2 is counted as "regret money".
//  Output:
//     1. Overall Regret Index  = regret money / total money * 100
//     2. Regret % per category (shows WHERE money is wasted)
//     3. Worth-Score = amount-weighted average rating
//     4. Projected yearly saving if regret spending is cut
//     5. Personalised advice based on the result
// ============================================================
void regretAnalysis() {
    cout << "\n=========== REGRET INDEX ANALYSER ===========\n";
    if (expenses.empty()) { cout << "No data to analyse.\n"; return; }

    double total = 0, regret = 0, weighted = 0;
    map<string, double> catTotal, catRegret;
    vector<Expense> regretList;

    for (size_t i = 0; i < expenses.size(); i++) {
        const Expense& e = expenses[i];
        total    += e.amount;
        weighted += e.amount * e.satisfaction;
        catTotal[e.category] += e.amount;
        if (e.satisfaction <= 2) {
            regret += e.amount;
            catRegret[e.category] += e.amount;
            regretList.push_back(e);
        }
    }

    double regretIndex = regret / total * 100.0;
    double worthScore  = weighted / total;

    cout << fixed << setprecision(2);
    cout << "Total spent          : " << total << "\n";
    cout << "Regret money (1-2/5) : " << regret << "\n";
    cout << "REGRET INDEX         : " << setprecision(1) << regretIndex << "%\n";
    cout << "Worth-Score (1-5)    : " << setprecision(2) << worthScore << "\n";

    cout << "\nRegret by category:\n";
    cout << left << setw(15) << "Category" << right << setw(12) << "Regret amt" << setw(10) << "Regret %" << "\n";
    cout << string(37, '-') << "\n";
    string worstCat; double worstPct = -1;
    for (map<string, double>::iterator it = catTotal.begin(); it != catTotal.end(); ++it) {
        double r = catRegret[it->first];
        double p = r / it->second * 100.0;
        cout << left << setw(15) << it->first << right << setw(12) << setprecision(2) << r
             << setw(9) << setprecision(1) << p << "%\n";
        if (p > worstPct) { worstPct = p; worstCat = it->first; }
    }

    if (!regretList.empty()) {
        sort(regretList.begin(), regretList.end(),
             [](const Expense& a, const Expense& b) { return a.amount > b.amount; });
        cout << "\nTop regretted purchases:\n";
        for (size_t i = 0; i < regretList.size() && i < 3; i++)
            cout << "  " << (i + 1) << ". " << regretList[i].description << " ("
                 << regretList[i].category << ") - " << setprecision(2) << regretList[i].amount << "\n";
    }

    // Projection: scale regret money to a full year using the span of recorded dates
    string first = expenses[0].date, last = expenses[0].date;
    for (size_t i = 0; i < expenses.size(); i++) {
        if (expenses[i].date < first) first = expenses[i].date;
        if (expenses[i].date > last)  last  = expenses[i].date;
    }
    int y1 = stoi(first.substr(0, 4)), m1 = stoi(first.substr(5, 2));
    int y2 = stoi(last.substr(0, 4)),  m2 = stoi(last.substr(5, 2));
    int months = (y2 - y1) * 12 + (m2 - m1) + 1;
    double yearly = regret / months * 12.0;
    cout << "\nIf you stop regret spending, you could save about "
         << setprecision(2) << yearly << " per year.\n";

    cout << "\nAdvice: ";
    if (regretIndex < 10)       cout << "Excellent! Almost everything you buy feels worth it.\n";
    else if (regretIndex < 25)  cout << "Good, but watch the '" << worstCat << "' category.\n";
    else if (regretIndex < 40)  cout << "Moderate regret. Pause 24 hours before buying in '" << worstCat << "'.\n";
    else                        cout << "High regret! Set a strict limit for '" << worstCat << "' immediately.\n";
}

// ---------------- Menu ----------------
void showMenu() {
    cout << "\n==================================================\n"
         << "        PERSONAL EXPENSE TRACKER\n"
         << "==================================================\n"
         << " 1. Add expense\n"
         << " 2. View all expenses\n"
         << " 3. Edit expense\n"
         << " 4. Delete expense\n"
         << " 5. Search expenses\n"
         << " 6. Category summary\n"
         << " 7. Monthly report\n"
         << " 8. Set monthly budget\n"
         << " 9. Regret Index Analyser  (unique feature)\n"
         << " 0. Exit\n"
         << "--------------------------------------------------\n";
}

int main() {
    loadData();
    cout << "Welcome! " << expenses.size() << " expense(s) loaded.\n";
    budgetAlert();
    while (true) {
        showMenu();
        int ch = readInt("Enter choice: ", 0, 9);
        switch (ch) {
            case 1: addExpense();      break;
            case 2: viewAll();         break;
            case 3: editExpense();     break;
            case 4: deleteExpense();   break;
            case 5: searchExpenses();  break;
            case 6: categorySummary(); break;
            case 7: monthlyReport();   break;
            case 8: setBudget();       break;
            case 9: regretAnalysis();  break;
            case 0: saveData(); cout << "Data saved. Goodbye!\n"; return 0;
        }
    }
}
