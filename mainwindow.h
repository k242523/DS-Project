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

struct Bank{
    std::string name;
    int netAmount;
    std::set<std::string> types;
};

struct Transaction{
    std::string payer;
    std::string payee;
    int amount;
    std::string mode;
};

class MainWindow:public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *p=nullptr);
    ~MainWindow();

private slots:
    void addBankRow();
    void addTransactionRow();
    void calculateMinCashFlow();
    void resetData();
    void saveToFile();
    void loadFromFile();

private:
    QTableWidget *tableBanks;
    QTableWidget *tableTransactions;
    QTextEdit *logOutput;
    QGraphicsScene *scene;
    QGraphicsView *view;

    int getMinIndex(const std::vector<Bank>& l);
    int getSimpleMaxIndex(const std::vector<Bank>& l);
    std::pair<int,std::string> getMaxIndex(const std::vector<Bank>& lst,int minIdx,const std::vector<Bank>& inp,int M);

    void drawGraph(const std::vector<Bank>& banks,const std::vector<Transaction>& res);

    std::vector<Bank> getBanksFromTable();
    std::vector<Transaction> getTransactionsFromTable();
};

#endif
