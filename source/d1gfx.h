#pragma once

#include <QImage>
#include <QtEndian>

#include "d1celtilesetframe.h"
#include "d1pal.h"

// TODO: move these to some persistency class?
#define SUB_HEADER_SIZE 0x0A
#define CEL_BLOCK_HEIGHT 32

#define SwapLE16(X) qToLittleEndian((quint16)(X))
#define SwapLE32(X) qToLittleEndian((quint32)(X))

class D1GfxPixel {
public:
    D1GfxPixel() = default;
    D1GfxPixel(bool, quint8);
    ~D1GfxPixel() = default;

    bool isTransparent() const;
    quint8 getPaletteIndex() const;

private:
    bool transparent = false;
    quint8 paletteIndex = 0;
};

class D1GfxFrame {
    friend class D1Cel;
    friend class D1CelFrame;
    friend class D1Cl2;
    friend class D1Cl2Frame;
    friend class D1CelTileset;
    friend class D1CelTilesetFrame;
    friend class D1ImageFrame;

public:
    D1GfxFrame() = default;
    ~D1GfxFrame() = default;

    quint16 getWidth() const;
    quint16 getHeight() const;
    D1GfxPixel getPixel(quint16, quint16) const;
    bool isClipped() const;
    D1CEL_FRAME_TYPE getFrameType() const;
    void setFrameType(D1CEL_FRAME_TYPE type);

protected:
    quint16 width = 0;
    quint16 height = 0;
    QList<QList<D1GfxPixel>> pixels;
    bool clipped = false;
    D1CEL_FRAME_TYPE frameType = D1CEL_FRAME_TYPE::Unknown;
};

enum class D1CEL_TYPE {
    V1_REGULAR,
    V1_COMPILATION,
    V1_LEVEL,
    V2_MONO_GROUP,
    V2_MULTIPLE_GROUPS,
    UNKNOWN = -1,
};

class D1Gfx : public QObject {
    Q_OBJECT

    friend class D1Cel;
    friend class D1Cl2;
    friend class D1CelTileset;

public:
    D1Gfx() = default;
    ~D1Gfx() = default;

    bool isFrameSizeConstant();
    QImage getFrameImage(quint16 frameIndex);
    D1GfxFrame *insertFrame(quint16 frameIndex, QString imageFilePath);
    D1GfxFrame *replaceFrame(quint16 frameIndex, QString imagefilePath);
    void removeFrame(quint16 frameIndex);

    D1CEL_TYPE getType();
    void setType(D1CEL_TYPE type);
    QString getFilePath();
    D1Pal *getPalette();
    void setPalette(D1Pal *pal);
    int getGroupCount();
    QPair<quint16, quint16> getGroupFrameIndices(int groupIndex);
    int getFrameCount();
    D1GfxFrame *getFrame(int frameIndex);
    quint16 getFrameWidth(int frameIndex);
    quint16 getFrameHeight(int frameIndex);

protected:
    D1CEL_TYPE type = D1CEL_TYPE::UNKNOWN;
    QString gfxFilePath;
    D1Pal *palette = nullptr;
    QList<QPair<quint16, quint16>> groupFrameIndices;
    QList<D1GfxFrame> frames;
};
