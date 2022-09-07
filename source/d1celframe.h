#pragma once

#include "d1celbase.h"

// Class used only for CEL frame width calculation
class D1CelPixelGroup {
public:
    D1CelPixelGroup() = default;
    D1CelPixelGroup(bool, quint16);

    bool isTransparent() const;
    quint16 getPixelCount();

private:
    bool transparent = false;
    quint16 pixelCount = 0;
};

class D1CelFrame : public D1CelFrameBase {
public:
    D1CelFrame() = default;

    bool load(QByteArray);

private:
    quint16 computeWidthFromHeader(QByteArray &);
    quint16 computeWidthFromData(QByteArray &);
};
