#pragma once

#include "d1celbase.h"

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

    D1CEL_FRAME_TYPE frameType = D1CEL_FRAME_TYPE::Square;
};
