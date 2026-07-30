#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QDebug>



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



}
