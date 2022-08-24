#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // Prefereces
    w.setWindowTitle("EGSE Client Sofware Version-2");
    w.show();

    return a.exec();
}
