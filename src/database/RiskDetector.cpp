

#include "RiskDetector.h"


bool RiskDetector::isDefaultLogin(DatabaseType type, QString login) {
    if (type == DatabaseType::MSSQL) {
        if (login == "sa") {
            return true;
        }

    }
    if (type == DatabaseType::MySQL) {
        if (login == "root") {
            return true;
        }
    }
    if (type == DatabaseType::Oracle) {
        if (login == "sys" || login == "system") {
            return true;
        }
    }
    if (type == DatabaseType::PostgreSQL) {
        if (login == "postgres") {
            return true;

        }
    }
    if (type == DatabaseType::IBM_DB2) {
        if (login =="db2admin" || login == "admin" ) {
            return true;
        }
    }
    return false;

}

Risk RiskDetector::detect(DatabaseType type, QString login, QString pass, Mode mode) {
    Risk result;
    if (mode != Mode::WinInt)
    {
        if (mode == Mode::Log || pass.isEmpty())
        {
            result.isNoPassword = true;
            result.warnings.append("Подключение без пароля");
        }
    }
    if (isDefaultLogin(type, login)) {
        result.isDefaultLog = true;
        result.defaultLogin = login;
        result.warnings.append("Вход по типовому логину");
    }

    if (result.isNoPassword || result.isDefaultLog) {
        result.status = "risk";

    }else {
        result.status = "ok";
    }
    return result;

}
