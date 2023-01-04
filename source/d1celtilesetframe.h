#pragma once

#include <QByteArray>

#include "openasdialog.h"

#define MICRO_WIDTH 32
#define MICRO_HEIGHT 32

class D1GfxFrame;

enum class D1CEL_FRAME_TYPE {
    Square,
    TransparentSquare,
    LeftTriangle,
    RightTriangle,
    LeftTrapezoid,
    RightTrapezoid,
    Unknown = -1,
};

class D1CelTilesetFrame {
public:
    static bool load(D1GfxFrame &frame, D1CEL_FRAME_TYPE frameType, QByteArray rawData, const OpenAsParam &params);

    static quint8 *writeFrameData(D1GfxFrame &frame, quint8 *pBuf);

private:
    static void LoadSquare(D1GfxFrame &frame, QByteArray &rawData);
    static void LoadTransparentSquare(D1GfxFrame &frame, QByteArray &rawData);
    static void LoadBottomLeftTriangle(D1GfxFrame &frame, QByteArray &rawData);
    static void LoadBottomRightTriangle(D1GfxFrame &frame, QByteArray &rawData);
    static void LoadLeftTriangle(D1GfxFrame &frame, QByteArray &rawData);
    static void LoadRightTriangle(D1GfxFrame &frame, QByteArray &rawData);
    static void LoadTopHalfSquare(D1GfxFrame &frame, QByteArray &rawData);
    static void LoadLeftTrapezoid(D1GfxFrame &frame, QByteArray &rawData);
    static void LoadRightTrapezoid(D1GfxFrame &frame, QByteArray &rawData);

    static quint8 *WriteSquare(D1GfxFrame &frame, quint8 *pBuf);
    static quint8 *WriteTransparentSquare(D1GfxFrame &frame, quint8 *pBuf);
    static quint8 *WriteLeftTriangle(D1GfxFrame &frame, quint8 *pBuf);
    static quint8 *WriteRightTriangle(D1GfxFrame &frame, quint8 *pBuf);
    static quint8 *WriteLeftTrapezoid(D1GfxFrame &frame, quint8 *pBuf);
    static quint8 *WriteRightTrapezoid(D1GfxFrame &frame, quint8 *pBuf);
};
