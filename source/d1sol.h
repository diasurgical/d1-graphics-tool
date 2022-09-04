#pragma once

#include <QFile>
#include <QList>

class D1Sol : public QObject {
    Q_OBJECT

public:
    D1Sol() = default;
    D1Sol(QString path);
    ~D1Sol();

    bool load(QString, int allocate = 0);

    QString getFilePath();
    quint8 getSubtileProperties(quint16);
    void setSubtileProperties(quint16 tileIndex, quint8 value);

private:
    QFile file;
    QList<quint8> subProperties;
};
