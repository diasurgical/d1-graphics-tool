#pragma once

#include "d1celbase.h"

#define MICRO_WIDTH 32
#define MICRO_HEIGHT 32

enum class D1CEL_FRAME_TYPE {
    Square,
    TransparentSquare,
    LeftTriangle,
    RightTriangle,
    LeftTrapezoid,
    RightTrapezoid,
    Unknown = -1,
};

class D1CelTilesetFrame : public D1CelFrameBase {
public:
    D1CelTilesetFrame() = default;
    D1CelTilesetFrame(D1CEL_FRAME_TYPE type);

    bool load(QByteArray rawData, OpenAsParam *params = nullptr);

    quint8 *writeFrameData(quint8 *pBuf);

private:
    void LoadSquare(QByteArray &rawData);
    void LoadTransparentSquare(QByteArray &rawData);
    void LoadBottomLeftTriangle(QByteArray &rawData);
    void LoadBottomRightTriangle(QByteArray &rawData);
    void LoadLeftTriangle(QByteArray &rawData);
    void LoadRightTriangle(QByteArray &rawData);
    void LoadTopHalfSquare(QByteArray &rawData);
    void LoadLeftTrapezoid(QByteArray &rawData);
    void LoadRightTrapezoid(QByteArray &rawData);

    quint8 *WriteSquare(quint8 *pBuf);
    quint8 *WriteTransparentSquare(quint8 *pBuf);
    quint8 *WriteLeftTriangle(quint8 *pBuf);
    quint8 *WriteRightTriangle(quint8 *pBuf);
    quint8 *WriteLeftTrapezoid(quint8 *pBuf);
    quint8 *WriteRightTrapezoid(quint8 *pBuf);

    D1CEL_FRAME_TYPE frameType = D1CEL_FRAME_TYPE::Square;
};
