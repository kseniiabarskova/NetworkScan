
#ifndef QTSCANNER_NETWORK_1_IPADDRESSGENERATOR_H
#define QTSCANNER_NETWORK_1_IPADDRESSGENERATOR_H
#include <qstring.h>
#include <vector>

class IPAddressGenerator {
private:
    static bool isCorrectIP(const QString& ip_address);
    static bool isCorrectRangeIP(const QString& ip_address1, const QString& ip_address2);

public:
    IPAddressGenerator() = delete;
    static std::vector<QString> generateListIP(const QString& ip_address1, const QString& ip_address2);

};


#endif //QTSCANNER_NETWORK_1_IPADDRESSGENERATOR_H
