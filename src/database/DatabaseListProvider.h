#ifndef QTSCANNER_NETWORK_1_DATABASELISTPROVIDER_H
#define QTSCANNER_NETWORK_1_DATABASELISTPROVIDER_H
#include <windows.h>
#include <qcontainerfwd.h>
#include <sqltypes.h>

#include "../scanner/DatabaseFingerprinter.h"


class DatabaseListProvider {
private:
    static std::wstring getMSSQL();
    static std::wstring getMySQL_MariaDB();
    static std::wstring getPostgre();
    static std::wstring getMongoDB();
    static QString getRedis(SQLHDBC hdbc);
    static std::wstring getOracle();
    static std::wstring getIBMdb2();
public:
    static QStringList getDatabases(SQLHDBC hdbc, DatabaseType type);


};


#endif //QTSCANNER_NETWORK_1_DATABASELISTPROVIDER_H
