#pragma once

#include <QFile>
#include <QList>

class D1Sol : public QObject {
    Q_OBJECT

public:
    D1Sol() = default;
    D1Sol(QString path);
    ~D1Sol();

    bool load(QString);

    QString getFilePath();
    quint8 getSubtileProperties(quint16);

private:
    QFile file;
    quint16 tileCount;
    QList<quint8> subProperties;
};
