#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "../network/IPAddressGenerator.h"
#include <QTableWidget>
#include <QMessageBox>
#include "../scanner/NetworkScannerWorker.h"
#include "../database/ConnectionManager.h"
#include "../database/DatabaseListProvider.h"
#include "../database/RiskDetector.h"
#include <QThread>


MainWindow::MainWindow(QWidget *parent):QMainWindow(parent), ui(new Ui::MainWindow) {

    ui->setupUi(this);
    connect(ui->portDiap,&QRadioButton::toggled,this,&MainWindow::on_portDiap_toggled);
    on_portDiap_toggled(ui->portDiap->isChecked());
    connect(ui->connected, &QPushButton::clicked, this, &MainWindow::on_connectedButton_clicked);
    ui->loginLine->setEnabled(true);
    ui->passLine->setEnabled(true);
}


MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_portDiap_toggled(bool checked) {
    ui->afterLabelPorts->setEnabled(checked);
    ui->afterLineEditPorts->setEnabled(checked);
    ui->beforeLineEditPorts->setEnabled(checked);
    ui->beforeLabelPorts->setEnabled(checked);
}


void MainWindow::onDatabaseFound(QString ip, quint16 port, DatabaseType type, quint64 responseTime) {
    qDebug() << ip << port << static_cast<int>(type);
    int row = ui->resultTable->rowCount();
    ui->resultTable->insertRow(row);
    ui->resultTable->setItem(row, 0, new QTableWidgetItem(ip));
    ui->resultTable->setItem(row, 1, new QTableWidgetItem(QString::number(port)));
    ui->resultTable->setItem(row, 3, new QTableWidgetItem(QString::number(responseTime, 'f', 2)+" мс"));
    QString dbName;
    switch (type)
    {
        case DatabaseType::MySQL:
            dbName = "MySQL";
            break;
        case DatabaseType::MariaDB:
            dbName = "MariaDB";
            break;
        case DatabaseType::Redis:
            dbName = "Redis";
            break;
        case DatabaseType::PostgreSQL:
            dbName = "PostgreSQL";
            break;
        case DatabaseType::MSSQL:
            dbName = "MSSQL";
            break;
        case DatabaseType::Oracle:
            dbName = "Oracle";
            break;
        case DatabaseType::IBM_DB2:
            dbName = "IBM_DB2";
            break;

        case DatabaseType::MongoDB:

            dbName = "MongoDB";
            break;
        case DatabaseType::Unknown:
        default:
            dbName = "Unknown";
            break;
    }
    ui->resultTable->setItem(row, 2, new QTableWidgetItem(dbName));
}



void MainWindow::onScanFinish() {
    std::cout << "ON SCAN FINISHED\n";
    if (ui->resultTable->rowCount() == 0) {
        QMessageBox::information(this,"Сканирование завершено","В указанном диапазоне не найдено ни одной базы данных.");
    }
    ui->scanButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    currentWorker = nullptr;
    if (currentWorker && currentWorker->wasStopped())
    {

    }
    else
    {
        ui->progressBar->setValue(0);
    }

}

std::vector<QString> MainWindow::collectIp() {
    QString startIp = ui->beforeLineEdit->text();
    QString endIp = ui->afterLineEdit->text();

    if (startIp.isEmpty() && endIp.isEmpty()) {
        QMessageBox::critical(this, "Пустой адрес", "Введите диапазон ip-адресов");
        return {};
    }if (startIp.isEmpty()) {
        QMessageBox::critical(this, "Пустой адрес", "Введите первый ip-адрес");
        return {};
    }if (endIp.isEmpty()) {
        QMessageBox::critical(this, "Пустой адрес", "Введите второй ip-адрес");
        return {};
    }
    auto ipList = IPAddressGenerator::generateListIP(startIp, endIp);

    if (ipList.empty()) {
        QMessageBox::critical(this, "Неверный диапазон значений", "Введите корректный диапазон ip-адресов");
        return {};
    }
    return ipList;
}


std::vector<quint16> MainWindow::collectPorts() {
    std::vector<quint16> ports;

    if (ui->portDiap->isChecked()) {
        QString startPort = ui->beforeLineEditPorts->text();
        QString endPort = ui->afterLineEditPorts->text();
        if (startPort.isEmpty() && endPort.isEmpty()) {
            QMessageBox::critical(nullptr, "Пустой порт", "Введите диапазон портов");
            return {};
        }else if (startPort.isEmpty()) {
            QMessageBox::critical(this, "Пустой порт", "Введите первый порт");
            return {};
        }else if (endPort.isEmpty()) {
            QMessageBox::critical(this, "Пустой порт", "Введите второй порт");
            return {};
        }
        bool ok1, ok2;
        int start = startPort.toInt(&ok1);
        int end = endPort.toInt(&ok2);
        if (!ok1 || !ok2) {
            QMessageBox::critical(this, "Ошибка", "Порт должен быть числом");
            return {};

        }
        if (start < 0 || start > 65535 || end < 0 || end > 65535) {
            QMessageBox::critical(this, "Некоррекный ввод", "Введите порт от 0 до 65535");
            return {};
        }
        if (start > end) {
            QMessageBox::critical(this, "Некоррекный ввод", "Введите корректный диапазон портов");
            return {};
        }
        for (quint16 i = start; i <= end; ++i) {
            ports.push_back(static_cast<quint16>(i));
        }
        return ports;
    }
    if (ui->postgreCheckBox->isChecked()) {
        ports.push_back(5432);
    }
    if (ui->mySQLcheckBox->isChecked()) {
        ports.push_back(3306);
    }
    if (ui->MSScheckBox->isChecked()) {
        ports.push_back(1433);
    }
    if (ui->mongoDBcheckBox->isChecked()) {
        ports.push_back(27017);
    }
    if (ui->redisCheckBox->isChecked()) {
        ports.push_back(6379);
    }
    if (ui->Oracle->isChecked()) {
        ports.push_back(1521);
    }
    if (ui->IBMDB2->isChecked()) {
        ports.push_back(50000);
    }

    if (ports.empty()) {
        QMessageBox::warning(this,"Нет портов","Выберите хотя бы одну базу данных.");
        return {};
    }
    return ports;
}



void MainWindow::startScan(const std::vector<QString> &ipList, const std::vector<quint16> &ports, bool rangeMode) {
    ui->stopButton->setEnabled(true);
    ui->scanButton->setEnabled(false);
    ui->progressBar->setValue(0);
    QThread* thread = new QThread(this);
    NetworkScannerWorker* nsw = new NetworkScannerWorker(ipList, ports, rangeMode);
   currentWorker = nsw;
    nsw->moveToThread(thread);


    connect(thread, &QThread::started, nsw, &NetworkScannerWorker::scan);

    connect(nsw, &NetworkScannerWorker::progressChanged, ui->progressBar, &QProgressBar::setValue);
    connect(nsw, &NetworkScannerWorker::databaseFound, this, &MainWindow::onDatabaseFound);
    connect(nsw, &NetworkScannerWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, nsw, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(nsw, &NetworkScannerWorker::finished, this, &MainWindow::onScanFinish);
    ui->resultTable->setRowCount(0);

    thread->start();
}

void MainWindow::on_stopButton_clicked() {
    if (currentWorker)
    {
        currentWorker->requestStop();
    }
}

void MainWindow::on_scanButton_clicked() {
    std::vector<QString> ipList = collectIp();

    if (ipList.empty())
        return;

    std::vector<quint16> ports = collectPorts();
    if (ports.empty()) {
        return;
    }
    bool rangeMode = ui->portDiap->isChecked();

   startScan(ipList, ports, rangeMode);

}





void MainWindow::on_connectedButton_clicked() {
    if (!isCorrectInput()) {
        return;
    }
    DatabaseType type = findDatabase();
    if (type == DatabaseType::Unknown) {
        return;
    }
    isConnected();


}

DatabaseType MainWindow::findDatabase() {
    if (!ui->radioIBM->isChecked() && !ui->radioMaria->isChecked() && !ui->radioMongo->isChecked() && !ui->radioMSSQL->isChecked() && !ui->radioMySQL->isChecked() && !ui->radioOracle->isChecked() && !ui->radioRedis->isChecked() && !ui->radioPostgre->isChecked()) {
        QMessageBox::warning(this, "Ошибка", "Выберите тип базы данных");
        return DatabaseType::Unknown;
    }
    DatabaseType dbType;
    if (ui->radioMSSQL->isChecked()) {
        dbType = DatabaseType::MSSQL;
    } else if (ui->radioMySQL->isChecked()) {
        dbType = DatabaseType::MySQL;
    } else if (ui->radioPostgre->isChecked()) {
        dbType = DatabaseType::PostgreSQL;
    }else if (ui->radioIBM->isChecked()) {
        dbType = DatabaseType::IBM_DB2;
    }else if (ui->radioMaria->isChecked()) {
        dbType = DatabaseType::MariaDB;

    }else if (ui->radioOracle->isChecked()) {
        dbType = DatabaseType::Oracle;
    }else if (ui->radioMongo->isChecked()) {
        dbType = DatabaseType::MongoDB;
    }else if (ui->radioRedis->isChecked()) {
        dbType = DatabaseType::Redis;
    }
    return DatabaseType::Unknown;
}

bool MainWindow::isCorrectInput() {


    if (ui->hostLine->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите адрес хоста");
        return false;
    }
    if (ui->portLine->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите номер порта");
        return false;
    }
    if (!ui->checkBoxLog->isChecked()) {
        if (ui->loginLine->text().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Введите логин");
            return false;
        }
    }
    return true;

}

void MainWindow::isConnected(DatabaseType type) {

    QString ip = ui->hostLine->text();
    int port = ui->portLine->text().toInt();
    QString user = ui->loginLine->text();
    QString pass = ui->passLine->text();
    Mode mode;
    if (ui->checkBoxLog->isChecked()) {
        mode = Mode::WinInt;
    }
    else if (ui->passLine->text().isEmpty()) {
        mode = Mode::Log;
    }
    else {
        mode = Mode::LogPass;
    }

    ConnectionManager cm;
    if (!cm.connect(type, ip, port, user, pass, mode)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться:\n" + cm.getError());
        return;
    }
    QStringList dbs = DatabaseListProvider::getDatabases(cm.getHDBC(),  type);


}



