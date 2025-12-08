#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTextEdit>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <set>
#include <vector>
#include <map>
#include <string>

// --- Data Structures ---
struct Bank {
    std::string name;
    int netAmount;
    std::set<std::string> types;
};

struct Transaction {
    std::string payer;
    std::string payee;
    int amount;
    std::string mode; // Optional: used for result display
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addBankRow();
    void addTransactionRow();
    void calculateMinCashFlow();
    void resetData();
    void saveToFile();
    void loadFromFile();

private:
    // --- UI Elements ---
    QTableWidget *tableBanks;       // Setup Banks & Modes
    QTableWidget *tableTransactions;// Input Debts
    QTextEdit *logOutput;           // Text Result
    QGraphicsScene *scene;          // Visualization
    QGraphicsView *view;

    // --- Logic Helpers ---
    int getMinIndex(const std::vector<Bank>& list);
    int getSimpleMaxIndex(const std::vector<Bank>& list);
    std::pair<int, std::string> getMaxIndex(const std::vector<Bank>& list, int minIndex, const std::vector<Bank>& input, int maxNumTypes);

    void drawGraph(const std::vector<Bank>& banks, const std::vector<Transaction>& results);

    // Helper to extract data from UI
    std::vector<Bank> getBanksFromTable();
    std::vector<Transaction> getTransactionsFromTable();
};

#endif // MAINWINDOW_H
