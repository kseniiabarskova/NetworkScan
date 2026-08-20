#ifndef QTSCANNER_NETWORK_1_DATABASEFINGERPRINTER_H
#define QTSCANNER_NETWORK_1_DATABASEFINGERPRINTER_H

#include <QString>
#include "SocketUtils.h"
enum class DatabaseType {
    Unknown,
    PostgreSQL,
    MySQL,
    MariaDB,
    MSSQL,
    MongoDB,
    Redis,
    Oracle,
    IBM_DB2
};

class DatabaseFingerprinter {
public:
    static DatabaseType identify(SocketHandle sock, quint16 port);
    static DatabaseType MySQL_MariaBD(SocketHandle sock);
    static DatabaseType PostgreSQL(SocketHandle sock);
    static DatabaseType Redis(SocketHandle sock);
   static DatabaseType DB2(SocketHandle sock);
    static DatabaseType MSSQL(SocketHandle sock);
    static DatabaseType Oracle(SocketHandle sock);
    static DatabaseType MongoDB(SocketHandle sock);
    static DatabaseType identifyUnknown(const QString& ip, quint16 port);

private:

};

#endif //QTSCANNER_NETWORK_1_DATABASEFINGERPRINTER_H
