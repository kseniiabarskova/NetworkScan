#include "ConnectionManager.h"
#include <iostream>

ConnectionManager::ConnectionManager() {
    henv1 = SQL_NULL_HENV;
    hdbc1 = SQL_NULL_HDBC;
    isConnected = false;
    error = "";
}

ConnectionManager::~ConnectionManager() {
    disconnect();
}

QString ConnectionManager::buildConnect(DatabaseType type, const QString &ip, int port, const QString &user, const QString &password,Mode mode) {
    switch (type) {
        case DatabaseType::MSSQL: {
            QString con = QString("Driver={ODBC Driver 18 for SQL Server};Server=%1;TrustServerCertificate=yes;Encrypt=no;")
            .arg(ip);
            switch (mode) {
                case Mode::LogPass:
                    std::cout << "Login: " << user.toStdString() << std::endl;
                    std::cout << "Password: " << password.toStdString() << std::endl;
                    con += QString("Uid=%1;Pwd=%2;").arg(user).arg(password);
                    break;

                case Mode::Log:
                    std::cout << "Login: " << user.toStdString() << std::endl;

                    con +=QString("Uid=%1;").arg(user);
                    break;

                case Mode::WinInt:
                    con += "Trusted_Connection=yes;";
                    break;
            }
            std::cout << "Connection string: " << con.toStdString() << std::endl;
            return con;
        }
        case DatabaseType::MySQL:
            return {};

        default:
            return {};
    }
}

bool ConnectionManager::connect(DatabaseType type, const QString &ip, int port, const QString &user, const QString &password, Mode mode) {
    if (isConnected) {
        disconnect();
    }
    error = "";
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1);
    if (!SQL_SUCCEEDED(ret)) {
        std::cout << "SQLAllocHandle() error" << std::endl;
        return false;
    }

    ret = SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (!SQL_SUCCEEDED(ret)) {
        std::cout << "SQLSetEnvAttr() error" << std::endl;
        disconnect();
        return false;
    }

    ret = SQLAllocHandle(SQL_HANDLE_DBC,henv1, &hdbc1);
    if (!SQL_SUCCEEDED(ret)) {
        std::cout << "SQLAllocHandle hdbc error" << std::endl;
        disconnect();
        return false;
    }

    std::wstring strConnect = (buildConnect(type, ip, port, user, password, mode)).toStdWString();

    SQLWCHAR outConnect[1024];
    SQLSMALLINT outLen;

    ret = SQLDriverConnect(hdbc1, NULL, (SQLWCHAR*)strConnect.c_str(), SQL_NTS, outConnect, sizeof(outConnect)/sizeof(SQLWCHAR), &outLen, SQL_DRIVER_NOPROMPT);

    if (SQL_SUCCEEDED(ret)) {
        isConnected = true;
        std::cout << "Successfully connected to the database" << std::endl;
        return true;
    }
    readError(SQL_HANDLE_DBC, hdbc1);
    std::cout << "Connection failed. Reason: " << getError().toStdString() << std::endl;

    disconnect();
    return false;
}


void ConnectionManager::disconnect() {
    if (hdbc1 != SQL_NULL_HDBC) {
        if (isConnected) {
            SQLDisconnect(hdbc1);
            isConnected = false;
        }
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc1);
        hdbc1 = SQL_NULL_HDBC;
    }
    if (henv1 != SQL_NULL_HENV) {
        SQLFreeHandle(SQL_HANDLE_ENV, henv1);
        henv1 = SQL_NULL_HENV;
    }
}


QString ConnectionManager::getError() const {
    return error;
}

void ConnectionManager::readError(SQLSMALLINT type, SQLHANDLE handle) {
    SQLWCHAR state[6];
    SQLINTEGER native;
    SQLWCHAR text[2048];
    SQLSMALLINT len;
    SQLRETURN ret;
    int i = 1;

    error = "";

    while (true) {
        ret = SQLGetDiagRec(type, handle, i, state, &native, text, sizeof(text) / sizeof(SQLWCHAR), &len);
        if (!SQL_SUCCEEDED(ret)) break;

        if (!error.isEmpty()) error += "; ";
        error += QString::fromWCharArray((wchar_t*)text);
        i++;
    }

    if (error.isEmpty()) {
        error = "Unknown ODBC error occurred.";
    }
}


SQLHDBC ConnectionManager::getHDBC() const {
    return hdbc1;
}

