#include <QApplication>
#include <QFile>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    const char *qssName = ":/D1GraphicsTool.qss";
    QFile file(qssName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open " << qssName;
        return -1;
    }
    QString styleSheet = QTextStream(&file).readAll();
    a.setStyleSheet(styleSheet);

    MainWindow w;
    w.setWindowTitle("Diablo 1 Graphics Tool");
    w.show();

    return a.exec();
}
