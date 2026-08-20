

#ifndef QTSCANNER_NETWORK_1_CONNECTIONMANAGER_H
#define QTSCANNER_NETWORK_1_CONNECTIONMANAGER_H
#include <QString>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include "../scanner/DatabaseFingerprinter.h"

enum class Mode {
    LogPass,
    Log,
    WinInt
};
class ConnectionManager {
private:


    SQLHENV henv1 = SQL_NULL_HENV; //дескриптор среды
    SQLHDBC hdbc1 = SQL_NULL_HDBC; //дескриптор соединения
    bool isConnected = false;
    QString error;



    QString buildConnect(DatabaseType type, const QString& ip, int port, const QString& user, const QString& password, Mode mode);

    void disconnect();


    void readError(SQLSMALLINT type, SQLHANDLE handle);

public:
    SQLHDBC getHDBC() const;
    ConnectionManager();
    ~ConnectionManager();

    QString getError() const;

    bool connect(DatabaseType type, const QString& ip, int port, const QString& user, const QString& password, Mode mode);


};


#endif //QTSCANNER_NETWORK_1_CONNECTIONMANAGER_H
