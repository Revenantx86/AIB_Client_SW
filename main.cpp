#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // Prefereces
    w.setWindowTitle("AIB Software Interface V1");
    w.show();

    return a.exec();
}
