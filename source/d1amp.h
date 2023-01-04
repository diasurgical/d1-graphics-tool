#pragma once

#include <QList>

#include "saveasdialog.h"

class D1Amp : public QObject {
    Q_OBJECT

public:
    D1Amp() = default;
    ~D1Amp() = default;

    bool load(QString filePath, int allocate = 0);
    bool save(const SaveAsParam &params);

    QString getFilePath();
    quint8 getTileType(quint16);
    quint8 getTileProperties(quint16);
    void setTileType(quint16 tileIndex, quint8 value);
    void setTileProperties(quint16 tileIndex, quint8 value);
    void removeTile(int tileIndex);

private:
    QString ampFilePath;
    QList<quint8> types;
    QList<quint8> properties;

    bool clear(int allocate);
};
