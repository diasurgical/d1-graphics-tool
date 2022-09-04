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
    void setTileType(quint16 tileIndex, quint8 value);
    void setTileProperties(quint16 tileIndex, quint8 value);

private:
    QFile file;
    QList<quint8> types;
    QList<quint8> properties;
};
