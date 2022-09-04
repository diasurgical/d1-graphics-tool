#pragma once

#include <QFile>
#include <QList>

class D1Amp : public QObject {
    Q_OBJECT

public:
    D1Amp() = default;
    D1Amp(QString path);
    ~D1Amp();

    bool load(QString);

    QString getFilePath();
    quint8 getTileType(quint16);
    quint8 getTileProperties(quint16);

private:
    QFile file;
    quint16 tileCount = 0;
    QList<quint8> types;
    QList<quint8> properties;
};
