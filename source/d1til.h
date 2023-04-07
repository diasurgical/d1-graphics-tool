#pragma once

#include <QImage>
#include <QList>
#include <QString>

#include "d1min.h"

#define TILE_WIDTH 2
#define TILE_HEIGHT 2

class D1Til : public QObject {
    Q_OBJECT

public:
    D1Til() = default;
    ~D1Til() = default;

    bool load(QString filePath, D1Min *min);
    bool save(const QString &gfxPath);

    QImage getTileImage(int tileIndex);
    QImage getFlatTileImage(int tileIndex);
    void insertTile(int tileIndex, const QList<quint16> &subtileIndices);
    void createTile();
    void removeTile(int tileIndex);

    QString getFilePath();
    int getTileCount();
    QList<quint16> &getSubtileIndices(int tileIndex);

private:
    QString tilFilePath;
    D1Min *min = nullptr;
    QList<QList<quint16>> subtileIndices;
};
