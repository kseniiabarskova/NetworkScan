
#include "IPAddressGenerator.h"
#include <QHostAddress>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QWidget>


bool IPAddressGenerator::isCorrectIP(const QString &ip_address) {
    static const QRegularExpression ipRegex(
        "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
    );
    if (!ipRegex.match(ip_address).hasMatch()) {
        return false;
    }

    QHostAddress address;
    if (address.setAddress(ip_address)) {
        return address.protocol() == QAbstractSocket::IPv4Protocol;
    }
    return false;
}



bool IPAddressGenerator::isCorrectRangeIP(const QString &ip_address1, const QString &ip_address2) {
    if (!isCorrectIP(ip_address1) || !isCorrectIP(ip_address2)) {
        return false;
    }
    QHostAddress addr1(ip_address1);
    QHostAddress addr2(ip_address2);

    quint32 ip1 = addr1.toIPv4Address();
    quint32 ip2 = addr2.toIPv4Address();

    return ip1 <= ip2;

}

std::vector<QString> IPAddressGenerator::generateListIP(const QString &ip_address1, const QString &ip_address2) {
    std::vector<QString> ipList;

    if (!isCorrectRangeIP(ip_address1, ip_address2)) {
        return ipList;
    }
    quint32 ip1 = QHostAddress(ip_address1).toIPv4Address();
    quint32 ip2 = QHostAddress(ip_address2).toIPv4Address();

    for (quint64 i = ip1; i <= ip2; ++i) {
        QString ipStr = QHostAddress(static_cast<quint32>(i)).toString();
        qDebug() << ipStr;
        ipList.push_back(ipStr);

    }
    return ipList;
}
