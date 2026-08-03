
#ifndef QTSCANNER_NETWORK_1_NETWORKSCANNERWORKER_H
#define QTSCANNER_NETWORK_1_NETWORKSCANNERWORKER_H
#include <vector>
#include <QString>
#include "SocketUtils.h"
class NetworkScannerWorker {
private:
    std::vector<QString> ipList;
    std::vector<quint16> ports;
    bool stop;

    
public:
    enum class PortState {
        Open,
        Closed,
        Timeout,
        Unreachable,
        Error
    };
    explicit NetworkScannerWorker(const std::vector<QString>& _ipList, const std::vector<quint16>& _ports);
    SocketHandle  checkPort(const QString& ip, quint16 port);
    void scan();
    //void stopScan();
    ~NetworkScannerWorker();
};


#endif //QTSCANNER_NETWORK_1_NETWORKSCANNERWORKER_H
