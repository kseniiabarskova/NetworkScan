
#ifndef QTSCANNER_NETWORK_1_NETWORKSCANNERWORKER_H
#define QTSCANNER_NETWORK_1_NETWORKSCANNERWORKER_H
#include <qobject.h>
#include <vector>
#include <QString>
#include "SocketUtils.h"
#include "DatabaseFingerprinter.h"
#include <mutex>
#include <atomic>

struct ScanTask {
    QString ip;
    quint16 port;
};

class NetworkScannerWorker : public QObject{
    Q_OBJECT
private:

    std::vector<QString> ipList;
    size_t currentIp = 0;
    std::vector<quint16> ports;
    size_t currentPort = 0;

    std::mutex mutex;
    bool getNextTask(ScanTask& task);
    void workerThread();
    bool rangeMode;
    std::atomic<bool> stopReq = false;
    std::atomic<int> lastPercent{-1};
    std::atomic<bool> stoppedByUser{false};





    std::atomic<size_t> completedTask{0};
    size_t totalTasks = 0;




    
public:
    void requestStop();

    explicit NetworkScannerWorker(std::vector<QString> _ipList, std::vector<quint16> _ports, bool _rangeMode);
    ~NetworkScannerWorker();
    bool wasStopped() const;

public slots:
    void scan();


signals:
    void finished();
    void databaseFound(QString ip, quint16 port, DatabaseType type, double responseTime);
    void progressChanged(int proc);
};


#endif //QTSCANNER_NETWORK_1_NETWORKSCANNERWORKER_H
