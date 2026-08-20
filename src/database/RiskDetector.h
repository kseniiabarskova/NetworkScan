

#ifndef QTSCANNER_NETWORK_1_RISKDETECTOR_H
#define QTSCANNER_NETWORK_1_RISKDETECTOR_H
#include <qlist.h>

#include "../scanner/DatabaseFingerprinter.h"
#include "ConnectionManager.h"


struct Risk{
    bool isNoPassword = false;
    bool isDefaultLog = false;
    QString defaultLogin;
    QStringList warnings;
    QString status;
};

class RiskDetector {
private:

public:
    void loadConfig();
    bool isDefaultLogin(DatabaseType type, QString login);
    Risk detect(DatabaseType type, QString login, QString pass, Mode mode);




};


#endif //QTSCANNER_NETWORK_1_RISKDETECTOR_H
