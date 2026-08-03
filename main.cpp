#include <QApplication>
#include "src/gui/MainWindow.h"
#include "src/gui/ui_mainwindow.h"



int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
