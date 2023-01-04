#pragma once

#include <QImage>
#include <map>

#include "d1celtilesetframe.h"
#include "d1gfx.h"
#include "saveasdialog.h"

class D1Min : public QObject {
    Q_OBJECT

public:
    D1Min() = default;
    ~D1Min() = default;

    bool load(QString minFilePath, quint16 subtileCount, std::map<unsigned, D1CEL_FRAME_TYPE> &celFrameTypes, const OpenAsParam &params);
    bool save(D1Gfx *gfx, const SaveAsParam &params);

    QImage getSubtileImage(quint16);

    QString getFilePath();
    void setCel(D1Gfx *c);
    quint16 getSubtileCount();
    quint16 getSubtileWidth();
    quint16 getSubtileHeight();
    QList<quint16> &getCelFrameIndices(quint16 subTileIndex);

private:
    QString minFilePath;
    D1Gfx *cel = nullptr;
    quint8 subtileWidth;
    quint8 subtileHeight;
    QList<QList<quint16>> celFrameIndices;
};
