#include <windows.h>

#include <QApplication>

#include "src/database/ConnectionManager.h"
#include "src/database/DatabaseListProvider.h"
#include "src/gui/MainWindow.h"
#include "src/gui/ui_mainwindow.h"

#include <sql.h>
#include <sqlext.h>
#include <iostream>
#include <string_view>



int main(int argc, char *argv[]) {
  //  QApplication app(argc, argv);
    //MainWindow window;
    //window.show();
    ConnectionManager cm;
    cm.connect(DatabaseType::MSSQL, "localhost" ,0, "", "", Mode::WinInt);
    DatabaseListProvider::getDatabases(cm.getHDBC(), DatabaseType::MSSQL);
    //cm.connect(DatabaseType::MSSQL, "127.0.0.1", 1433, "test", "1234", Mode::LogPass);

    //cm.connect(DatabaseType::MSSQL, "127.0.0.1", 1434, "test", "1234", Mode::LogPass);
  //  return app.exec();
}

