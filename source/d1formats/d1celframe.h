#pragma once

#include <QByteArray>

#include "d1gfx.h"
#include "dialogs/openasdialog.h"

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

class D1CelFrame {
public:
    static bool load(D1GfxFrame &frame, QByteArray rawData, const OpenAsParam &params);

private:
    static quint16 computeWidthFromHeader(QByteArray &);
    static quint16 computeWidthFromData(QByteArray &);
};
