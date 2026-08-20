#ifndef QTSCANNER_NETWORK_1_MAINWINDOW_H
#define QTSCANNER_NETWORK_1_MAINWINDOW_H
#include <QMainWindow>
#include "../scanner/DatabaseFingerprinter.h"
#include "../scanner/NetworkScannerWorker.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_scanButton_clicked();
    void on_stopButton_clicked();
    void onDatabaseFound(QString ip, quint16 port, DatabaseType type, quint64 responseTime);
    void onScanFinish();
    void on_portDiap_toggled(bool checked);
    std::vector<quint16> collectPorts();
    std::vector<QString> collectIp();
    void startScan(const std::vector<QString>& ipList, const std::vector<quint16>& ports, bool rangeMode);

    void on_connectedButton_clicked();

    DatabaseType findDatabase();
    bool isCorrectInput();
    void isConnected();
private:
    Ui::MainWindow *ui;
private:
    NetworkScannerWorker* currentWorker = nullptr;


};


#endif //QTSCANNER_NETWORK_1_MAINWINDOW_H
