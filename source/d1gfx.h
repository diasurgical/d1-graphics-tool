#pragma once

#include <QImage>
#include <QMap>
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
    static D1GfxPixel transparentPixel();
    static D1GfxPixel colorPixel(quint8 color);

    ~D1GfxPixel() = default;

    bool isTransparent() const;
    quint8 getPaletteIndex() const;

    friend bool operator==(const D1GfxPixel &lhs, const D1GfxPixel &rhs);

private:
    D1GfxPixel() = default;

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

    int getWidth() const;
    int getHeight() const;
    D1GfxPixel getPixel(int x, int y) const;
    D1CEL_FRAME_TYPE getFrameType() const;
    void setFrameType(D1CEL_FRAME_TYPE type);

protected:
    int width = 0;
    int height = 0;
    QList<QList<D1GfxPixel>> pixels;
    // fields of tileset-frames
    D1CEL_FRAME_TYPE frameType = D1CEL_FRAME_TYPE::TransparentSquare;
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
    D1GfxFrame *insertFrame(int frameIndex, const QImage &image);
    D1GfxFrame *replaceFrame(int frameIndex, const QImage &image);
    void removeFrame(quint16 frameIndex);
    void remapFrames(const QMap<unsigned, unsigned> &remap);

    bool isTileset() const;
    bool hasHeader() const;
    void setHasHeader(bool hasHeader);
    QString getFilePath();
    D1Pal *getPalette();
    void setPalette(D1Pal *pal);
    int getGroupCount();
    QPair<quint16, quint16> getGroupFrameIndices(int groupIndex);
    int getFrameCount();
    D1GfxFrame *getFrame(int frameIndex);
    int getFrameWidth(int frameIndex);
    int getFrameHeight(int frameIndex);

protected:
    bool isTileset_ = false;
    bool hasHeader_ = true;
    QString gfxFilePath;
    D1Pal *palette = nullptr;
    QList<QPair<quint16, quint16>> groupFrameIndices;
    QList<D1GfxFrame> frames;
};
