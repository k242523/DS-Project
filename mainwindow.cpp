#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QGroupBox>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <cmath>
#include <algorithm>
#include <climits>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    setWindowTitle("Cash Flow Minimizer System (Payment Mode Aware)");
    resize(1400, 900);

    // --- Layouts ---
    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    QVBoxLayout *leftLayout = new QVBoxLayout();
    QVBoxLayout *rightLayout = new QVBoxLayout();

    // --- 1. Bank Setup Section ---
    QGroupBox *grpBanks = new QGroupBox("1. Setup Banks & Payment Modes");
    QVBoxLayout *bankLayout = new QVBoxLayout();

    tableBanks = new QTableWidget(0, 2);
    QStringList bankHeaders;
    bankHeaders << "Bank Name" << "Payment Modes (comma separated)";
    tableBanks->setHorizontalHeaderLabels(bankHeaders);
    tableBanks->horizontalHeader()->setStretchLastSection(true);
    tableBanks->setMinimumHeight(200);

    QPushButton *btnAddBank = new QPushButton("Add Bank");
    connect(btnAddBank, &QPushButton::clicked, this, &MainWindow::addBankRow);

    bankLayout->addWidget(new QLabel("Note: Row 1 is the Intermediary (World Bank)"));
    bankLayout->addWidget(tableBanks);
    bankLayout->addWidget(btnAddBank);
    grpBanks->setLayout(bankLayout);

    // --- 2. Transaction Input Section ---
    QGroupBox *grpTrans = new QGroupBox("2. Input Transactions");
    QVBoxLayout *transLayout = new QVBoxLayout();

    tableTransactions = new QTableWidget(0, 3);
    QStringList transHeaders;
    transHeaders << "Debtor (From)" << "Creditor (To)" << "Amount";
    tableTransactions->setHorizontalHeaderLabels(transHeaders);
    tableTransactions->horizontalHeader()->setStretchLastSection(true);
    tableTransactions->setMinimumHeight(200);

    QPushButton *btnAddTrans = new QPushButton("Add Transaction");
    connect(btnAddTrans, &QPushButton::clicked, this, &MainWindow::addTransactionRow);

    transLayout->addWidget(tableTransactions);
    transLayout->addWidget(btnAddTrans);
    grpTrans->setLayout(transLayout);

    // --- Buttons ---
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnCalc = new QPushButton("Minimize Cash Flow");
    QPushButton *btnReset = new QPushButton("Reset");
    QPushButton *btnSave = new QPushButton("Save Data");
    QPushButton *btnLoad = new QPushButton("Load Data");

    btnCalc->setMinimumHeight(35);
    btnReset->setMinimumHeight(35);
    btnSave->setMinimumHeight(35);
    btnLoad->setMinimumHeight(35);

    btnLayout->addWidget(btnCalc);
    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnLoad);
    btnLayout->addWidget(btnReset);

    // Add to Left Panel
    leftLayout->addWidget(grpBanks, 2);
    leftLayout->addWidget(grpTrans, 2);
    leftLayout->addLayout(btnLayout);

    // --- 3. Output Section (Right Panel) ---
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setBackgroundBrush(QColor(250, 250, 250));
    view->setMinimumWidth(500);

    logOutput = new QTextEdit();
    logOutput->setReadOnly(true);
    logOutput->setMaximumHeight(220);

    rightLayout->addWidget(new QLabel("<b>Visualization:</b>"));
    rightLayout->addWidget(view, 3);
    rightLayout->addWidget(new QLabel("<b>Detailed Logic Output:</b>"));
    rightLayout->addWidget(logOutput, 1);

    // Final Assembly
    mainLayout->addLayout(leftLayout, 2);
    mainLayout->addLayout(rightLayout, 3);

    // Connect Main Logic
    connect(btnCalc, &QPushButton::clicked, this, &MainWindow::calculateMinCashFlow);
    connect(btnReset, &QPushButton::clicked, this, &MainWindow::resetData);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::saveToFile);
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadFromFile);

    // Initialize with World Bank Row
    addBankRow();
    tableBanks->setItem(0, 0, new QTableWidgetItem("World_Bank"));
    tableBanks->setItem(0, 1, new QTableWidgetItem("GooglePay,PayTM,Wire"));
}

MainWindow::~MainWindow() {}

void MainWindow::addBankRow() {
    int row = tableBanks->rowCount();
    tableBanks->insertRow(row);
    tableBanks->setItem(row, 0, new QTableWidgetItem(""));
    tableBanks->setItem(row, 1, new QTableWidgetItem(""));
}

void MainWindow::addTransactionRow() {
    int row = tableTransactions->rowCount();
    tableTransactions->insertRow(row);
    tableTransactions->setItem(row, 0, new QTableWidgetItem(""));
    tableTransactions->setItem(row, 1, new QTableWidgetItem(""));
    tableTransactions->setItem(row, 2, new QTableWidgetItem("0"));
}

// --- ALGORITHM IMPLEMENTATION ---

int MainWindow::getMinIndex(const std::vector<Bank>& list) {
    int min = INT_MAX, minIndex = -1;
    for(size_t i = 0; i < list.size(); i++) {
        if(list[i].netAmount == 0) continue;
        if(list[i].netAmount < min) {
            minIndex = i;
            min = list[i].netAmount;
        }
    }
    return minIndex;
}

int MainWindow::getSimpleMaxIndex(const std::vector<Bank>& list) {
    int max = INT_MIN, maxIndex = -1;
    for(size_t i = 0; i < list.size(); i++) {
        if(list[i].netAmount == 0) continue;
        if(list[i].netAmount > max) {
            maxIndex = i;
            max = list[i].netAmount;
        }
    }
    return maxIndex;
}

std::pair<int, std::string> MainWindow::getMaxIndex(const std::vector<Bank>& list, int minIndex,
                                                    const std::vector<Bank>& input, int maxNumTypes) {
    int max = INT_MIN;
    int maxIndex = -1;
    std::string matchingType = "";

    for(size_t i = 0; i < list.size(); i++) {
        if(list[i].netAmount == 0) continue;
        if(list[i].netAmount < 0) continue;

        std::vector<std::string> v(maxNumTypes + 10);
        auto it = std::set_intersection(list[minIndex].types.begin(), list[minIndex].types.end(),
                                        list[i].types.begin(), list[i].types.end(),
                                        v.begin());

        if((it - v.begin()) != 0 && max < list[i].netAmount) {
            max = list[i].netAmount;
            maxIndex = i;
            matchingType = *(v.begin());
        }
    }
    return {maxIndex, matchingType};
}

void MainWindow::calculateMinCashFlow() {
    // 1. Parse Inputs
    std::vector<Bank> banks;
    std::map<std::string, int> indexOf;
    int maxNumTypes = 0;

    // Validate we have at least some banks
    if(tableBanks->rowCount() == 0) {
        QMessageBox::warning(this, "Error", "Please add at least one bank!");
        return;
    }

    for(int i=0; i<tableBanks->rowCount(); i++) {
        Bank b;
        if(!tableBanks->item(i, 0) || tableBanks->item(i, 0)->text().trimmed().isEmpty())
            continue;

        b.name = tableBanks->item(i, 0)->text().trimmed().toStdString();

        QString modes = tableBanks->item(i, 1) ? tableBanks->item(i, 1)->text() : "";
        QStringList modeList = modes.split(",");
        for(const QString& m : modeList) {
            QString clean = m.trimmed();
            if(!clean.isEmpty()) b.types.insert(clean.toStdString());
        }

        if(b.types.empty()) {
            QMessageBox::warning(this, "Error",
                                 QString("Bank '%1' has no payment modes!").arg(QString::fromStdString(b.name)));
            return;
        }

        if((int)b.types.size() > maxNumTypes) maxNumTypes = b.types.size();

        b.netAmount = 0;
        banks.push_back(b);
        indexOf[b.name] = banks.size() - 1;
    }

    int numBanks = banks.size();
    if(numBanks < 2) {
        QMessageBox::warning(this, "Error", "Please add at least 2 banks!");
        return;
    }

    // 2. Parse Transactions & Build Graph
    std::vector<std::vector<int>> graph(numBanks, std::vector<int>(numBanks, 0));

    for(int i=0; i<tableTransactions->rowCount(); i++) {
        if(!tableTransactions->item(i,0) || !tableTransactions->item(i,1) || !tableTransactions->item(i,2))
            continue;

        std::string p1 = tableTransactions->item(i,0)->text().trimmed().toStdString();
        std::string p2 = tableTransactions->item(i,1)->text().trimmed().toStdString();
        int amt = tableTransactions->item(i,2)->text().toInt();

        if(p1.empty() || p2.empty()) continue;

        if(indexOf.find(p1) != indexOf.end() && indexOf.find(p2) != indexOf.end()) {
            graph[indexOf[p1]][indexOf[p2]] = amt;
        } else {
            QString missing = (indexOf.find(p1) == indexOf.end()) ? QString::fromStdString(p1) : QString::fromStdString(p2);
            QMessageBox::warning(this, "Error",
                                 QString("Transaction references unknown bank: %1").arg(missing));
            return;
        }
    }

    // 3. Calculate Net Amount
    std::vector<Bank> listOfNetAmounts = banks;
    for(int b=0; b<numBanks; b++) {
        int amount = 0;
        for(int i=0; i<numBanks; i++) amount += graph[i][b];
        for(int j=0; j<numBanks; j++) amount += (-1) * graph[b][j];
        listOfNetAmounts[b].netAmount = amount;
    }

    // 4. Greedy Algorithm Execution
    std::vector<Transaction> resultLogData;
    int numZeroNetAmounts = 0;
    for(const auto& b : listOfNetAmounts)
        if(b.netAmount == 0) numZeroNetAmounts++;

    int iterations = 0;
    const int MAX_ITERATIONS = 1000;

    while(numZeroNetAmounts != numBanks && iterations < MAX_ITERATIONS) {
        iterations++;

        int minIndex = getMinIndex(listOfNetAmounts);
        if(minIndex == -1) break;

        std::pair<int, std::string> maxAns = getMaxIndex(listOfNetAmounts, minIndex, banks, maxNumTypes);
        int maxIndex = maxAns.first;

        if(maxIndex == -1) {
            // No common type -> Route via World Bank (Index 0)
            int amount = std::abs(listOfNetAmounts[minIndex].netAmount);
            std::string mode = *(banks[minIndex].types.begin());

            resultLogData.push_back({banks[minIndex].name, banks[0].name, amount, mode});

            int simpleMaxIndex = getSimpleMaxIndex(listOfNetAmounts);
            if(simpleMaxIndex == -1) break;

            std::string mode2 = *(banks[simpleMaxIndex].types.begin());
            resultLogData.push_back({banks[0].name, banks[simpleMaxIndex].name, amount, mode2});

            listOfNetAmounts[simpleMaxIndex].netAmount += listOfNetAmounts[minIndex].netAmount;
            listOfNetAmounts[minIndex].netAmount = 0;

            if(listOfNetAmounts[minIndex].netAmount == 0) numZeroNetAmounts++;
            if(listOfNetAmounts[simpleMaxIndex].netAmount == 0) numZeroNetAmounts++;
        }
        else {
            int transactionAmount = std::min(std::abs(listOfNetAmounts[minIndex].netAmount),
                                             listOfNetAmounts[maxIndex].netAmount);

            resultLogData.push_back({banks[minIndex].name, banks[maxIndex].name,
                                     transactionAmount, maxAns.second});

            listOfNetAmounts[minIndex].netAmount += transactionAmount;
            listOfNetAmounts[maxIndex].netAmount -= transactionAmount;

            if(listOfNetAmounts[minIndex].netAmount == 0) numZeroNetAmounts++;
            if(listOfNetAmounts[maxIndex].netAmount == 0) numZeroNetAmounts++;
        }
    }

    // 5. Display Text Results
    logOutput->clear();
    logOutput->append("<b>Minimization Plan:</b><br>");
    if(resultLogData.empty()) {
        logOutput->append("<i>No transactions needed - all accounts are balanced!</i>");
    } else {
        for(const auto& t : resultLogData) {
            logOutput->append(QString("%1 pays Rs %2 to %3 via <b>%4</b>")
                                  .arg(QString::fromStdString(t.payer))
                                  .arg(t.amount)
                                  .arg(QString::fromStdString(t.payee))
                                  .arg(QString::fromStdString(t.mode)));
        }
    }

    // 6. Draw Graph
    drawGraph(banks, resultLogData);
}

void MainWindow::drawGraph(const std::vector<Bank>& banks, const std::vector<Transaction>& results) {
    scene->clear();
    if(banks.empty()) return;

    // --- CONFIGURATION ---
    double circleRadius = 280;
    double nodeDiameter = 90;
    double nodeRadius = nodeDiameter / 2.0;

    QColor nodeColor(66, 165, 245);
    QColor nodeBorderColor(30, 30, 30);
    QColor textColor(255, 255, 255);
    QColor edgeColor(50, 50, 50);
    QColor amountBgColor(255, 255, 255);
    QColor amountTextColor(200, 0, 0);

    QFont nodeFont("Arial", 10, QFont::Bold);
    QFont amountFont("Arial", 9, QFont::Bold);

    // --- 1. CALCULATE POSITIONS ---
    std::map<std::string, QPointF> positions;
    int worldBankIndex = -1;
    std::vector<int> otherIndices;

    for(size_t i=0; i<banks.size(); i++) {
        if(banks[i].name == "World_Bank") worldBankIndex = i;
        else otherIndices.push_back(i);
    }

    if(worldBankIndex != -1) {
        positions[banks[worldBankIndex].name] = QPointF(0, 0);
    }

    int N = otherIndices.size();
    double angleStep = (2 * M_PI) / (N > 0 ? N : 1);

    for(int i=0; i<N; i++) {
        double angle = i * angleStep - M_PI / 2;
        double x = circleRadius * cos(angle);
        double y = circleRadius * sin(angle);
        positions[banks[otherIndices[i]].name] = QPointF(x, y);
    }

    // --- 2. COUNT EDGES BETWEEN SAME PAIRS (for offset calculation) ---
    std::map<std::pair<std::string, std::string>, int> edgeCount;
    std::map<std::pair<std::string, std::string>, int> edgeIndex;

    for(const auto& t : results) {
        std::string from = t.payer;
        std::string to = t.payee;
        // Create canonical ordering for bidirectional edge counting
        std::pair<std::string, std::string> key = (from < to) ?
                                                      std::make_pair(from, to) : std::make_pair(to, from);
        edgeCount[key]++;
    }

    // Reset for actual drawing
    for(auto& p : edgeCount) edgeIndex[p.first] = 0;

    // --- 3. DRAW EDGES ---
    for(const auto& t : results) {
        if(positions.find(t.payer) == positions.end() ||
            positions.find(t.payee) == positions.end()) continue;

        QPointF start = positions[t.payer];
        QPointF end = positions[t.payee];

        QLineF lineVector(start, end);
        double length = lineVector.length();

        if (length > nodeDiameter) {
            // Calculate perpendicular offset for multiple edges
            std::string from = t.payer;
            std::string to = t.payee;
            std::pair<std::string, std::string> key = (from < to) ?
                                                          std::make_pair(from, to) : std::make_pair(to, from);

            int totalEdges = edgeCount[key];
            int currentIndex = edgeIndex[key]++;

            // Calculate offset: spread edges symmetrically
            double offsetDistance = 0;
            if(totalEdges > 1) {
                double maxOffset = 20.0; // pixels
                offsetDistance = (currentIndex - (totalEdges - 1) / 2.0) * maxOffset;
            }

            // Direction vector
            QPointF dir = (end - start) / length;
            // Perpendicular vector (rotate 90 degrees)
            QPointF perpDir(-dir.y(), dir.x());

            // Apply offset to start and end
            QPointF offsetVector = perpDir * offsetDistance;
            QPointF offsetStart = start + offsetVector;
            QPointF offsetEnd = end + offsetVector;

            // Trim to node edges
            QPointF realStart = offsetStart + dir * (nodeRadius + 3);
            QPointF realEnd = offsetEnd - dir * (nodeRadius + 3);

            // Draw curved line for better visualization
            QPainterPath path;
            path.moveTo(realStart);

            if(std::abs(offsetDistance) > 5) {
                // Bezier curve for offset edges
                QPointF midPoint = (realStart + realEnd) / 2 + offsetVector * 0.3;
                path.quadTo(midPoint, realEnd);
            } else {
                // Straight line for single edges
                path.lineTo(realEnd);
            }

            QGraphicsPathItem* pathItem = scene->addPath(path, QPen(edgeColor, 2.5));
            pathItem->setZValue(0);

            // Arrowhead pointing to CREDITOR (payee)
            double arrowSize = 14;
            QPointF arrowTip = realEnd;

            // Calculate arrow angle from the curve direction
            QPointF arrowBase = (std::abs(offsetDistance) > 5) ?
                                    (realEnd * 0.95 + (realStart + realEnd) / 2 * 0.05) : // curved
                                    (realEnd * 0.95 + realStart * 0.05); // straight

            double angle = std::atan2(arrowTip.y() - arrowBase.y(),
                                      arrowTip.x() - arrowBase.x());

            QPointF arrowP1 = arrowTip - QPointF(cos(angle - M_PI / 6) * arrowSize,
                                                 sin(angle - M_PI / 6) * arrowSize);
            QPointF arrowP2 = arrowTip - QPointF(cos(angle + M_PI / 6) * arrowSize,
                                                 sin(angle + M_PI / 6) * arrowSize);

            QPolygonF arrowHead;
            arrowHead << arrowTip << arrowP1 << arrowP2;
            QGraphicsPolygonItem* arrowItem = scene->addPolygon(arrowHead,
                                                                QPen(edgeColor),
                                                                QBrush(edgeColor));
            arrowItem->setZValue(1);

            // Amount Label - position along the path
            QPointF labelPos;
            if(std::abs(offsetDistance) > 5) {
                // Position on curve
                labelPos = (realStart + realEnd) / 2 + offsetVector * 0.15;
            } else {
                labelPos = (realStart + realEnd) / 2;
            }

            QGraphicsTextItem* amtText = new QGraphicsTextItem(QString("Rs %1").arg(t.amount));
            amtText->setFont(amountFont);
            amtText->setDefaultTextColor(amountTextColor);

            QRectF br = amtText->boundingRect();
            QPointF textPos(labelPos.x() - br.width()/2, labelPos.y() - br.height()/2);
            amtText->setPos(textPos);

            QGraphicsRectItem* bgRect = scene->addRect(textPos.x() - 2, textPos.y() - 1,
                                                       br.width() + 4, br.height() + 2,
                                                       QPen(Qt::NoPen), QBrush(amountBgColor));
            bgRect->setZValue(2);

            scene->addItem(amtText);
            amtText->setZValue(3);
        }
    }

    // --- 4. DRAW NODES ---
    for(const auto& b : banks) {
        if(positions.find(b.name) == positions.end()) continue;
        QPointF pos = positions[b.name];

        double tlX = pos.x() - nodeRadius;
        double tlY = pos.y() - nodeRadius;

        // Shadow
        scene->addEllipse(tlX + 4, tlY + 4, nodeDiameter, nodeDiameter,
                          Qt::NoPen, QBrush(QColor(180, 180, 180, 120)));

        // Circle
        QGraphicsEllipseItem* circle = scene->addEllipse(tlX, tlY, nodeDiameter, nodeDiameter,
                                                         QPen(nodeBorderColor, 2.5),
                                                         QBrush(nodeColor));
        circle->setZValue(4);

        // Text
        QString label = QString::fromStdString(b.name);
        if(label.length() > 12) {
            int mid = label.length() / 2;
            int spacePos = label.indexOf('_', mid - 3);
            if(spacePos > 0) label[spacePos] = '\n';
            else if(!label.contains('\n')) label.insert(mid, "\n");
        }

        QGraphicsTextItem* nameText = scene->addText(label, nodeFont);
        nameText->setDefaultTextColor(textColor);

        QRectF br = nameText->boundingRect();
        nameText->setPos(pos.x() - br.width()/2, pos.y() - br.height()/2);
        nameText->setZValue(5);
    }

    scene->setSceneRect(scene->itemsBoundingRect().adjusted(-50, -50, 50, 50));
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::resetData() {
    tableBanks->setRowCount(0);
    tableTransactions->setRowCount(0);
    logOutput->clear();
    scene->clear();

    addBankRow();
    tableBanks->setItem(0, 0, new QTableWidgetItem("World_Bank"));
    tableBanks->setItem(0, 1, new QTableWidgetItem("GooglePay,PayTM,Wire"));
}

void MainWindow::saveToFile() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Data", "", "CSV Files (*.csv)");
    if(fileName.isEmpty()) return;

    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "BANKS\n";
        for(int i=0; i<tableBanks->rowCount(); i++) {
            if(tableBanks->item(i,0) && !tableBanks->item(i,0)->text().isEmpty()) {
                QString modes = tableBanks->item(i,1) ? tableBanks->item(i,1)->text() : "";
                out << tableBanks->item(i,0)->text() << "|" << modes << "\n";
            }
        }
        out << "TRANS\n";
        for(int i=0; i<tableTransactions->rowCount(); i++) {
            if(tableTransactions->item(i,0) && !tableTransactions->item(i,0)->text().isEmpty()) {
                QString p1 = tableTransactions->item(i,0)->text();
                QString p2 = tableTransactions->item(i,1) ? tableTransactions->item(i,1)->text() : "";
                QString amt = tableTransactions->item(i,2) ? tableTransactions->item(i,2)->text() : "0";
                out << p1 << "," << p2 << "," << amt << "\n";
            }
        }
        file.close();
        QMessageBox::information(this, "Success", "Data saved successfully!");
    }
}

void MainWindow::loadFromFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Load Data", "", "CSV Files (*.csv)");
    if(fileName.isEmpty()) return;

    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        resetData();
        tableBanks->setRowCount(0);

        QTextStream in(&file);
        bool readingBanks = false;
        bool readingTrans = false;

        while(!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if(line == "BANKS") { readingBanks = true; readingTrans = false; continue; }
            if(line == "TRANS") { readingBanks = false; readingTrans = true; continue; }

            if(readingBanks && !line.isEmpty()) {
                QStringList parts = line.split("|");
                if(parts.size() >= 2) {
                    int r = tableBanks->rowCount();
                    tableBanks->insertRow(r);
                    tableBanks->setItem(r, 0, new QTableWidgetItem(parts[0]));
                    tableBanks->setItem(r, 1, new QTableWidgetItem(parts[1]));
                }
            }
            if(readingTrans && !line.isEmpty()) {
                QStringList parts = line.split(",");
                if(parts.size() >= 3) {
                    int r = tableTransactions->rowCount();
                    tableTransactions->insertRow(r);
                    tableTransactions->setItem(r, 0, new QTableWidgetItem(parts[0]));
                    tableTransactions->setItem(r, 1, new QTableWidgetItem(parts[1]));
                    tableTransactions->setItem(r, 2, new QTableWidgetItem(parts[2]));
                }
            }
        }
        file.close();
        QMessageBox::information(this, "Success", "Data loaded successfully!");
    }
}
