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
    Redis
};

class DatabaseFingerprinter {
public:
    static DatabaseType identify(SocketHandle sock, quint16 port);

private:
    static DatabaseType MySQL_MariaBD(SocketHandle sock);
    //static DatabaseType PostgreSQL(SocketHandle sock);
    static DatabaseType Redis(SocketHandle sock);
    //DatabaseType MongoDB(SocketHandle sock);
    //DatabaseType MSSQL(SocketHandle sock);
};

#endif //QTSCANNER_NETWORK_1_DATABASEFINGERPRINTER_H
