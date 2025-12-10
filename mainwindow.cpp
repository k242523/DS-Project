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
#include <QPainterPath>
#include <algorithm>
#include <climits>
#include <cmath>

MainWindow::MainWindow(QWidget* p):QMainWindow(p)
{
    setWindowTitle("Cash Flow Minimizer");
    resize(1400,900);

    QWidget* c = new QWidget(this);
    setCentralWidget(c);

    auto *lay1 = new QHBoxLayout(c);
    auto *layL = new QVBoxLayout();
    auto *layR = new QVBoxLayout();

    auto *gb1 = new QGroupBox("Setup Banks & Payment Modes");
    auto *bbb = new QVBoxLayout();
    tableBanks = new QTableWidget(0,2);
    tableBanks->setHorizontalHeaderLabels({"Bank Name","Payment Modes (comma separated)"});
    tableBanks->horizontalHeader()->setStretchLastSection(true);
    tableBanks->setMinimumHeight(200);
    auto *btnA = new QPushButton("Add Bank");
    connect(btnA,&QPushButton::clicked,this,&MainWindow::addBankRow);

    bbb->addWidget(new QLabel("Note: First row should be World Bank"));
    bbb->addWidget(tableBanks);
    bbb->addWidget(btnA);
    gb1->setLayout(bbb);

    auto *gbT = new QGroupBox("Input Transactions");
    auto *tLay = new QVBoxLayout();
    tableTransactions = new QTableWidget(0,3);
    tableTransactions->setHorizontalHeaderLabels({"Debtor","Creditor","Amount"});
    tableTransactions->horizontalHeader()->setStretchLastSection(true);
    tableTransactions->setMinimumHeight(200);
    auto *btnB = new QPushButton("Add Transaction");
    connect(btnB,&QPushButton::clicked,this,&MainWindow::addTransactionRow);

    tLay->addWidget(tableTransactions);
    tLay->addWidget(btnB);
    gbT->setLayout(tLay);

    auto *act = new QHBoxLayout();
    auto *x1=new QPushButton("Minimize Cash Flow");
    auto *x2=new QPushButton("Save");
    auto *x3=new QPushButton("Load");
    auto *x4=new QPushButton("Reset");
    for(auto *pp:{x1,x2,x3,x4}) pp->setMinimumHeight(35);

    act->addWidget(x1);
    act->addWidget(x2);
    act->addWidget(x3);
    act->addWidget(x4);

    layL->addWidget(gb1,2);
    layL->addWidget(gbT,2);
    layL->addLayout(act);

    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setBackgroundBrush(QColor(250,250,250));
    view->setMinimumWidth(500);

    logOutput=new QTextEdit();
    logOutput->setReadOnly(true);
    logOutput->setMaximumHeight(220);

    layR->addWidget(new QLabel("<b>Visualization:</b>"));
    layR->addWidget(view,3);
    layR->addWidget(new QLabel("<b>Transaction Log:</b>"));
    layR->addWidget(logOutput,1);

    lay1->addLayout(layL,2);
    lay1->addLayout(layR,3);

    connect(x1,&QPushButton::clicked,this,&MainWindow::calculateMinCashFlow);
    connect(x4,&QPushButton::clicked,this,&MainWindow::resetData);
    connect(x2,&QPushButton::clicked,this,&MainWindow::saveToFile);
    connect(x3,&QPushButton::clicked,this,&MainWindow::loadFromFile);

    addBankRow();
    tableBanks->setItem(0,0,new QTableWidgetItem("World_Bank"));
    tableBanks->setItem(0,1,new QTableWidgetItem("GooglePay,PayTM,Wire"));
}

MainWindow::~MainWindow(){}

void MainWindow::addBankRow(){
    int r = tableBanks->rowCount();
    tableBanks->insertRow(r);
    tableBanks->setItem(r,0,new QTableWidgetItem(""));
    tableBanks->setItem(r,1,new QTableWidgetItem(""));
}

void MainWindow::addTransactionRow(){
    int r = tableTransactions->rowCount();
    tableTransactions->insertRow(r);
    tableTransactions->setItem(r,0,new QTableWidgetItem(""));
    tableTransactions->setItem(r,1,new QTableWidgetItem(""));
    tableTransactions->setItem(r,2,new QTableWidgetItem("0"));
}

int MainWindow::getMinIndex(const std::vector<Bank>& a){
    int idx=-1, v=INT_MAX;
    for (int i=0;i<(int)a.size();i++){
        if(a[i].netAmount==0) continue;
        if(a[i].netAmount < v){
            v=a[i].netAmount; idx=i;
        }
    }
    return idx;
}

int MainWindow::getSimpleMaxIndex(const std::vector<Bank>& a){
    int ii=-1, vv=INT_MIN;
    for(int i=0;i<(int)a.size();i++){
        if(a[i].netAmount==0) continue;
        if(a[i].netAmount > vv){ vv=a[i].netAmount; ii=i; }
    }
    return ii;
}

std::pair<int,std::string> MainWindow::getMaxIndex(const std::vector<Bank>& lst,
                                                    int mi,
                                                    const std::vector<Bank>& input,
                                                    int M)
{
    int idx=-1, val=INT_MIN;
    std::string mode="";
    if(mi<0 || mi>= (int)lst.size()) return {-1,""};
    for(int i=0;i<(int)lst.size();i++){
        if(lst[i].netAmount<=0) continue;
        std::vector<std::string> tmp(M+10);
        auto it = std::set_intersection(lst[mi].types.begin(), lst[mi].types.end(),
                                        lst[i].types.begin(), lst[i].types.end(),
                                        tmp.begin());
        if(it!=tmp.begin() && lst[i].netAmount>val){
            val=lst[i].netAmount;
            idx=i;
            mode=*tmp.begin();
        }
    }
    return {idx, mode};
}

void MainWindow::calculateMinCashFlow(){
    std::vector<Bank> BB;
    std::map<std::string,int> idx;
    int mmm=0;

    if(tableBanks->rowCount()==0){
        QMessageBox::warning(this,"Error","Add at least one bank!");
        return;
    }

    for(int r=0;r<tableBanks->rowCount();r++){
        if(!tableBanks->item(r,0)) continue;
        QString nm = tableBanks->item(r,0)->text().trimmed();
        if(nm.isEmpty()) continue;
        Bank b;
        b.name=nm.toStdString();
        QString md = tableBanks->item(r,1)? tableBanks->item(r,1)->text() : "";
        for(QString m: md.split(',')){
            QString t=m.trimmed();
            if(!t.isEmpty()) b.types.insert(t.toStdString());
        }
        if(b.types.empty()){
            QMessageBox::warning(this,"Error",QString("Bank '%1' needs payment modes!").arg(nm));
            return;
        }
        if((int)b.types.size()>mmm) mmm=b.types.size();
        b.netAmount=0;
        idx[b.name]=(int)BB.size();
        BB.push_back(b);
    }

    if(BB.size()<2){
        QMessageBox::warning(this,"Error","Need at least 2 banks!");
        return;
    }

    int n = BB.size();
    std::vector<std::vector<int>> G(n, std::vector<int>(n,0));

    for(int r=0;r<tableTransactions->rowCount();r++){
        if(!tableTransactions->item(r,0) || !tableTransactions->item(r,1) || !tableTransactions->item(r,2))
            continue;
        std::string A = tableTransactions->item(r,0)->text().trimmed().toStdString();
        std::string B = tableTransactions->item(r,1)->text().trimmed().toStdString();
        int am = tableTransactions->item(r,2)->text().toInt();
        if(A.empty() || B.empty()) continue;
        if(idx.find(A)==idx.end() || idx.find(B)==idx.end()){
            QString miss = idx.find(A)==idx.end()? QString::fromStdString(A) : QString::fromStdString(B);
            QMessageBox::warning(this,"Error",QString("Unknown bank: %1").arg(miss));
            return;
        }
        G[idx[A]][idx[B]] = am;
    }

    std::vector<Bank> net = BB;
    for(int i=0;i<n;i++){
        int inc=0, out=0;
        for(int x=0;x<n;x++) inc+=G[x][i];
        for(int x=0;x<n;x++) out+=G[i][x];
        net[i].netAmount = inc - out;
    }

    int bal=0;
    for(auto &x:net) if(x.netAmount==0) bal++;

    std::vector<Transaction> plan;
    int SAFE=0;

    while(bal!=n && SAFE<1000){
        SAFE++;
        int deb = getMinIndex(net);
        if(deb<0) break;

        auto good = getMaxIndex(net,deb,BB,mmm);
        int cred = good.first;

        if(cred<0){
            int amt = std::abs(net[deb].netAmount);
            std::string md = *BB[deb].types.begin();
            plan.push_back({BB[deb].name, BB[0].name, amt, md});
            int sm = getSimpleMaxIndex(net);
            if(sm<0) break;
            std::string md2 = *BB[sm].types.begin();
            plan.push_back({BB[0].name, BB[sm].name, amt, md2});
            net[sm].netAmount += net[deb].netAmount;
            net[deb].netAmount = 0;
            if(net[deb].netAmount==0) bal++;
            if(net[sm].netAmount==0) bal++;
        } else {
            int aa = std::min(std::abs(net[deb].netAmount), net[cred].netAmount);
            plan.push_back({BB[deb].name, BB[cred].name, aa, good.second});
            net[deb].netAmount += aa;
            net[cred].netAmount -= aa;
            if(net[deb].netAmount==0) bal++;
            if(net[cred].netAmount==0) bal++;
        }
    }

    logOutput->clear();
    logOutput->append("<b>Minimization Plan:</b><br>");
    if(plan.empty()){
        logOutput->append("<i>All accounts balanced!</i>");
    } else {
        for(auto &t:plan){
            logOutput->append(QString("%1 pays Rs %2 to %3 via <b>%4</b>")
                                  .arg(QString::fromStdString(t.payer))
                                  .arg(t.amount)
                                  .arg(QString::fromStdString(t.payee))
                                  .arg(QString::fromStdString(t.mode)));
        }
    }

    drawGraph(BB,plan);
}

void MainWindow::drawGraph(const std::vector<Bank>& BB,const std::vector<Transaction>& R){
    scene->clear();
    if(BB.empty()) return;
    double CR=280, ND=90, NR=ND/2;

    QColor c1(66,165,245), b1(30,30,30);
    QFont f1("Arial",10,QFont::Bold), f2("Arial",9,QFont::Bold);

    std::map<std::string,QPointF> p;
    int wi=-1; std::vector<int> oth;
    for(int i=0;i<(int)BB.size();i++){
        if(BB[i].name=="World_Bank") wi=i;
        else oth.push_back(i);
    }

    if(wi>=0) p[BB[wi].name]=QPointF(0,0);

    int m = oth.size();
    double st = m>0? 2*M_PI/m : 1;
    for(int k=0;k<m;k++){
        double ag = k*st - M_PI/2;
        p[BB[oth[k]].name] = QPointF(CR*cos(ag), CR*sin(ag));
    }

    std::map<std::pair<std::string,std::string>,int> Cnt,Idx;
    for(auto &t:R){
        auto key = t.payer < t.payee? std::make_pair(t.payer,t.payee) : std::make_pair(t.payee,t.payer);
        Cnt[key]++;
    }
    for(auto &x:Cnt) Idx[x.first]=0;

    for(auto &t:R){
        if(!p.count(t.payer) || !p.count(t.payee)) continue;

        QPointF s = p[t.payer], e=p[t.payee];
        QLineF L(s,e);
        if(L.length()<=ND) continue;

        auto key = t.payer<t.payee? std::make_pair(t.payer,t.payee):std::make_pair(t.payee,t.payer);
        int tot=Cnt[key], id=Idx[key]++;

        double off=0;
        if(tot>1) off=(id-(tot-1)/2.0)*20;

        QPointF d=(e-s)/L.length();
        QPointF pp(-d.y(), d.x());
        QPointF of=pp*off;
        QPointF ss=s+of, ee=e+of;

        QPointF rs=ss + d*(NR+3);
        QPointF re=ee - d*(NR+3);

        QPainterPath pt;
        pt.moveTo(rs);
        if(std::abs(off)>5){
            QPointF mid=(rs+re)/2.0 + of*0.3;
            pt.quadTo(mid,re);
        } else pt.lineTo(re);

        auto *pa=scene->addPath(pt,QPen(QColor(50,50,50),2.5));
        pa->setZValue(0);

        double as=14;
        QPointF tip=re;
        QPointF base = std::abs(off)>5? (re*0.95 + (rs+re)/2.0*0.05) : (re*0.95 + rs*0.05);
        double ang = atan2(tip.y()-base.y(), tip.x()-base.x());
        QPointF p1 = tip - QPointF(cos(ang - M_PI/6)*as, sin(ang - M_PI/6)*as);
        QPointF p2 = tip - QPointF(cos(ang + M_PI/6)*as, sin(ang + M_PI/6)*as);

        QPolygonF hd; hd << tip << p1 << p2;
        scene->addPolygon(hd,QPen(QColor(50,50,50)),QBrush(QColor(50,50,50)))->setZValue(1);

        QPointF lp = std::abs(off)>5? (rs+re)/2.0 + of*0.15 : (rs+re)/2.0;
        auto *TT = new QGraphicsTextItem(QString("Rs %1").arg(t.amount));
        TT->setFont(f2);
        TT->setDefaultTextColor(QColor(200,0,0));

        QRectF bb=TT->boundingRect();
        QPointF pos(lp.x()-bb.width()/2, lp.y()-bb.height()/2);
        scene->addRect(pos.x()-2,pos.y()-1,bb.width()+4,bb.height()+2,QPen(Qt::NoPen),QBrush(Qt::white))->setZValue(2);
        TT->setPos(pos); scene->addItem(TT); TT->setZValue(3);
    }

    for(auto &b:BB){
        if(!p.count(b.name)) continue;
        QPointF O=p[b.name];
        double xx=O.x()-NR, yy=O.y()-NR;
        scene->addEllipse(xx+4,yy+4,ND,ND,Qt::NoPen,QBrush(QColor(180,180,180,120)));
        auto *cc=scene->addEllipse(xx,yy,ND,ND,QPen(b1,2.5),QBrush(c1));
        cc->setZValue(4);

        QString L = QString::fromStdString(b.name);
        if(L.size()>12){
            int mid=L.size()/2;
            int ub=L.indexOf('_',mid-3);
            if(ub>0) L[ub]='\n';
            else if(!L.contains('\n')) L.insert(mid,'\n');
        }

        auto *txt=scene->addText(L,f1);
        txt->setDefaultTextColor(Qt::white);
        QRectF bb = txt->boundingRect();
        txt->setPos(O.x()-bb.width()/2,O.y()-bb.height()/2);
        txt->setZValue(5);
    }

    scene->setSceneRect(scene->itemsBoundingRect().adjusted(-50,-50,50,50));
    view->fitInView(scene->sceneRect(),Qt::KeepAspectRatio);
}

void MainWindow::resetData(){
    tableBanks->setRowCount(0);
    tableTransactions->setRowCount(0);
    logOutput->clear();
    scene->clear();
    addBankRow();
    tableBanks->setItem(0,0,new QTableWidgetItem("World_Bank"));
    tableBanks->setItem(0,1,new QTableWidgetItem("GooglePay,PayTM,Wire"));
}

void MainWindow::saveToFile(){
    QString F = QFileDialog::getSaveFileName(this,"Save Data","","CSV Files (*.csv)");
    if(F.isEmpty()) return;
    QFile file(F);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text)) return;
    QTextStream o(&file);
    o<<"BANKS\n";
    for(int r=0;r<tableBanks->rowCount();r++){
        if(tableBanks->item(r,0) && !tableBanks->item(r,0)->text().isEmpty()){
            QString md = tableBanks->item(r,1)? tableBanks->item(r,1)->text():"";
            o<<tableBanks->item(r,0)->text()<<"|"<<md<<"\n";
        }
    }
    o<<"TRANS\n";
    for(int r=0;r<tableTransactions->rowCount();r++){
        if(tableTransactions->item(r,0) && !tableTransactions->item(r,0)->text().isEmpty()){
            QString a = tableTransactions->item(r,0)->text();
            QString b = tableTransactions->item(r,1)?tableTransactions->item(r,1)->text():"";
            QString c = tableTransactions->item(r,2)?tableTransactions->item(r,2)->text():"0";
            o<<a<<","<<b<<","<<c<<"\n";
        }
    }
    file.close();
    QMessageBox::information(this,"Success","Data saved!");
}

void MainWindow::loadFromFile(){
    QString F = QFileDialog::getOpenFileName(this,"Load Data","","CSV Files (*.csv)");
    if(F.isEmpty()) return;
    QFile file(F);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) return;

    resetData();
    tableBanks->setRowCount(0);

    QTextStream in(&file);
    bool rb=false, rt=false;

    while(!in.atEnd()){
        QString L = in.readLine().trimmed();
        if(L=="BANKS"){ rb=true; rt=false; continue; }
        if(L=="TRANS"){ rb=false; rt=true; continue; }
        if(rb){
            QStringList parts=L.split("|");
            if(parts.size()>=1){
                int rr=tableBanks->rowCount();
                tableBanks->insertRow(rr);
                tableBanks->setItem(rr,0,new QTableWidgetItem(parts[0]));
                if(parts.size()>1) tableBanks->setItem(rr,1,new QTableWidgetItem(parts[1]));
            }
        } else if(rt){
            QStringList t = L.split(",");
            if(t.size()>=3){
                int rr=tableTransactions->rowCount();
                tableTransactions->insertRow(rr);
                tableTransactions->setItem(rr,0,new QTableWidgetItem(t[0]));
                tableTransactions->setItem(rr,1,new QTableWidgetItem(t[1]));
                tableTransactions->setItem(rr,2,new QTableWidgetItem(t[2]));
            }
        }
    }
    file.close();
}
