#ifndef D1SOL_H
#define D1SOL_H

#include <QBuffer>
#include <QFile>
#include <QList>

class D1Sol : public QObject {
    Q_OBJECT

public:
    D1Sol();
    D1Sol(QString path);
    ~D1Sol();

    bool load(QString);

    QString getFilePath();
    bool isFileOpen();
    quint16 getTileCount();
    quint8 getSubtileProperties(quint16);

private:
    QFile file;
    quint16 tileCount;
    QList<quint8> subProperties;
};

#endif // D1SOL_H
