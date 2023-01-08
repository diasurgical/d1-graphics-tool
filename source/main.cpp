#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyleSheet("QLineEdit[readOnly=\"true\"] { color: #808080; background-color: #F0F0F0; }");
    //          "border: 1px solid #B0B0B0;"
    //          "border-radius: 2px;}");

    MainWindow w;
    w.setWindowTitle("Diablo 1 Graphics Tool");
    w.show();

    return a.exec();
}
