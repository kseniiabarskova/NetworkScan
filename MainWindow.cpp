#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "IPAddressGenerator.h"
#include <QTableWidget>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent):QMainWindow(parent), ui(new Ui::MainWindow) {

    ui->setupUi(this);
    connect(ui->scanButton, &QPushButton::clicked, this, &MainWindow::on_scanButton_clicked);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_scanButton_clicked() {
    QString startIp = ui->beforeLineEdit->text();
    QString endIp = ui->afterLineEdit->text();

    qDebug() << "Начальный ip: " << startIp;
    qDebug() << "Конечный ip: " << endIp;
    if (startIp.isEmpty() && endIp.isEmpty()) {
        QMessageBox::critical(nullptr, "Пустой адрес", "Введите диапазон ip-адресов");
        return;
    }else if (startIp.isEmpty()) {
        QMessageBox::critical(nullptr, "Пустой адрес", "Введите первый ip-адрес");
        return;
    }else if (endIp.isEmpty()) {
        QMessageBox::critical(nullptr, "Пустой адрес", "Введите второй ip-адрес");
        return;
    }

    std::vector<QString> ipList = IPAddressGenerator::generateListIP(startIp, endIp);

    if (ipList.empty()) {
        QMessageBox::critical(nullptr, "Неверный диапазон значений", "Введите корректный диапазон ip-адресов");
    }

    ui->resultTable->clearContents();
    ui->resultTable->setRowCount(0);
    ui->resultTable->setUpdatesEnabled(false);
    int row = 0;
    for (const auto& ipStr: ipList) {
        ui->resultTable->insertRow(row);
        QTableWidgetItem *item = new QTableWidgetItem(ipStr);
        ui->resultTable->setItem(row, 0, item);
        row++;
    }
    ui->resultTable->setUpdatesEnabled(true);
    
    std::vector<uint16_t> ports;
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







}
