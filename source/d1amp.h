#pragma once

#include <QList>
#include <QString>

#include "openasdialog.h"

class D1Amp : public QObject {
    Q_OBJECT

public:
    D1Amp() = default;
    ~D1Amp() = default;

    bool load(QString filePath, int tileCount, const OpenAsParam &params);
    bool save(const QString &gfxPath);

    QString getFilePath();
    quint8 getTileType(quint16);
    quint8 getTileProperties(quint16);
    void setTileType(quint16 tileIndex, quint8 value);
    void setTileProperties(quint16 tileIndex, quint8 value);
    void createTile();
    void removeTile(int tileIndex);

private:
    QString ampFilePath;
    QList<quint8> types;
    QList<quint8> properties;
};
