#pragma once

#include <map>

#include <QImage>
#include <QList>
#include <QMap>

#include "d1celtilesetframe.h"
#include "d1gfx.h"
#include "d1sol.h"
#include "saveasdialog.h"

class D1Min : public QObject {
    Q_OBJECT

public:
    D1Min() = default;
    ~D1Min() = default;

    bool load(QString minFilePath, D1Gfx *gfx, D1Sol *sol, std::map<unsigned, D1CEL_FRAME_TYPE> &celFrameTypes, const OpenAsParam &params);
    bool save(const SaveAsParam &params);

    QImage getSubtileImage(int subtileIndex);

    void insertSubtile(int subtileIndex, const QList<quint16> &frameIndicesList);
    void createSubtile();
    void removeSubtile(int subtileIndex);
    void remapSubtiles(const QMap<unsigned, unsigned> &remap);

    QString getFilePath();
    int getSubtileCount();
    quint16 getSubtileWidth();
    void setSubtileWidth(int width);
    quint16 getSubtileHeight();
    void setSubtileHeight(int height);
    QList<quint16> &getCelFrameIndices(int subtileIndex);

private:
    QString minFilePath;
    D1Gfx *gfx = nullptr;
    quint8 subtileWidth;
    quint8 subtileHeight;
    QList<QList<quint16>> celFrameIndices;
};
