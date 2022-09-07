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
    quint16 getSubtileCount();
    quint8 getSubtileProperties(quint16);
    void setSubtileProperties(quint16 tileIndex, quint8 value);

private:
    QFile file;
    QList<quint8> subProperties;
};
