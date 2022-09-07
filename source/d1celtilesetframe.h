#pragma once

#include "d1celbase.h"

enum class D1CEL_FRAME_TYPE {
    Square,
    TransparentSquare,
    LeftTriangle,
    RightTriangle,
    LeftTrapezoid,
    RightTrapezoid,
};

class D1CelTilesetFrame : public D1CelFrameBase {
public:
    D1CelTilesetFrame() = default;
    D1CelTilesetFrame(D1CEL_FRAME_TYPE);

    bool load(QByteArray rawData);

private:
    void LoadSquare(QByteArray rawData);
    void LoadTransparentSquare(QByteArray &rawData);
    void LoadLeftTriangle(QByteArray &rawData);
    void LoadRightTriangle(QByteArray &rawData);
    void LoadLeftTrapezoid(QByteArray &rawData);
    void LoadRightTrapezoid(QByteArray &rawData);

    D1CEL_FRAME_TYPE frameType = D1CEL_FRAME_TYPE::Square;
};
