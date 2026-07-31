
#ifndef QTSCANNER_NETWORK_1_NETWORKSCANNERWORKER_H
#define QTSCANNER_NETWORK_1_NETWORKSCANNERWORKER_H
#include <vector>
#include <QString>

class NetworkScannerWorker {
private:
    std::vector<QString>* ipList;
    std::vector<QString>* ports;
    bool stop;
    
public:
    bool isPortOpen(const std::vector<QString>& _ipList, const std::vector<QString>& _ports);
    void scan();
};


#endif //QTSCANNER_NETWORK_1_NETWORKSCANNERWORKER_H
