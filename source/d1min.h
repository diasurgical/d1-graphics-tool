#pragma once

#include <QImage>
#include <map>

#include "d1celtilesetframe.h"
#include "d1gfx.h"
#include "saveasdialog.h"

enum class D1MIN_TYPE {
    REGULAR_HEIGHT,
    EXTENDED_HEIGHT
};

class D1Min : public QObject {
    Q_OBJECT

public:
    D1Min() = default;
    ~D1Min() = default;

    bool load(QString minFilePath, quint16 subtileCount);
    bool save(SaveAsParam *params = nullptr);

    QImage getSubtileImage(quint16);

    D1MIN_TYPE getType();
    QString getFilePath();
    void setCel(D1Gfx *c);
    quint16 getSubtileCount();
    quint16 getSubtileWidth();
    quint16 getSubtileHeight();
    D1CEL_FRAME_TYPE getFrameType(quint16 id);
    QList<quint16> getCelFrameIndices(quint16);

private:
    D1MIN_TYPE type = D1MIN_TYPE::REGULAR_HEIGHT;
    QString minFilePath;
    D1Gfx *cel = nullptr;
    quint8 subtileWidth = 2;
    quint8 subtileHeight = 5;
    QList<QList<quint16>> celFrameIndices;
    std::map<unsigned, D1CEL_FRAME_TYPE> celFrameTypes;
};
