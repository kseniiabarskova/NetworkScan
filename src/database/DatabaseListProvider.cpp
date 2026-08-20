
#include "DatabaseListProvider.h"

#include <qlist.h>
#include <sql.h>
#include <iostream>
#include <sqlucode.h>

std::wstring DatabaseListProvider::getMSSQL() {
    std::wstring query = L"SELECT name FROM sys.databases;";
    return query;
}

std::wstring DatabaseListProvider::getMySQL_MariaDB() {
    std::wstring query = L"SELECT schema_name FROM information_schema.schemata;";
    return query;
}

std::wstring DatabaseListProvider::getPostgre() {
    std::wstring query = L"SELECT datname FROM pg_database;";
    return query;
}

std::wstring DatabaseListProvider::getMongoDB() {
    return L"SHOW DATABASES;";
}

QString DatabaseListProvider::getRedis(SQLHDBC hdbc) {
    QString result;
    SQLHSTMT hstmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (!SQL_SUCCEEDED(ret)) {
        std::cout << "Failed to allocate statement handle" << std::endl;
        return result;
    }

    ret = SQLTables(hstmt, (SQLWCHAR*)SQL_ALL_CATALOGS, SQL_NTS, (SQLWCHAR*)L"", 0, (SQLWCHAR*)L"", 0, (SQLWCHAR*)L"", 0);
    if (SQL_SUCCEEDED(ret)) {
        SQLWCHAR dbName[256];
        SQLLEN nameLen;
        SQLBindCol(hstmt, 1, SQL_C_WCHAR, dbName, sizeof(dbName), &nameLen);

        std::cout << "Database list: " << std::endl;

        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            if (nameLen != SQL_NULL_DATA) {
                std::wcout << L"Database: " << dbName << std::endl;
                result.append(QString::fromWCharArray(dbName));
            }

        }

    }else {
        std::cout << "SQLExecDirect error" << std::endl;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return result;

}

std::wstring DatabaseListProvider::getOracle() {
    return L"SELECT username FROM all_users;";
}

std::wstring DatabaseListProvider::getIBMdb2() {
    return L"SELECT DISTINCT schemaname FROM syscat.schemata;";
}






QStringList DatabaseListProvider::getDatabases(SQLHDBC hdbc, DatabaseType type) {
    QStringList result;

    if (type == DatabaseType::Redis) {
        getRedis(hdbc);
    }

    SQLHSTMT hstmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (!SQL_SUCCEEDED(ret)) {
        std::cout << "Failed to allocate statement handle" << std::endl;
        return result;
    }
    std::wstring query;

    switch (type) {
        case DatabaseType::MSSQL:
            query = getMSSQL();
            break;
        case DatabaseType::MariaDB:
        case DatabaseType::MySQL:
            query = getMySQL_MariaDB();
            break;
        case DatabaseType::PostgreSQL:
            query = getPostgre();
            break;
        case DatabaseType::MongoDB:
            query = getMongoDB();
            break;
        case DatabaseType::Oracle:
            query = getOracle();
            break;
        case DatabaseType::IBM_DB2:
            query = getIBMdb2();
            break;

        default:
            std::cout << "Unknown database type" << std::endl;
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            return result;
    }

    ret = SQLExecDirect(hstmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    if (SQL_SUCCEEDED(ret)) {
        SQLWCHAR dbName[256];
        SQLLEN nameLen;

        SQLBindCol(hstmt, 1, SQL_C_WCHAR, dbName, sizeof(dbName), &nameLen);

        std::cout << "Database list: " << std::endl;

        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            if (nameLen != SQL_NULL_DATA) {
                std::wcout << L"Database: " << dbName << std::endl;
                result.append(QString::fromWCharArray(dbName));
            }

        }

    }else {
        std::cout << "SQLExecDirect error" << std::endl;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return result;
}