#pragma once

#include <QImage>
#include <QString>

#include "d1gfx.h"
#include "d1pal.h"

class D1ImageFrame {
public:
    static bool load(D1GfxFrame &frame, QImage &image, bool clipped, D1Pal *pal);
};
