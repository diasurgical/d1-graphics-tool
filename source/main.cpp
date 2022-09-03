#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.setWindowTitle("Diablo 1 Graphics Tool");
    w.show();

    return a.exec();
}
