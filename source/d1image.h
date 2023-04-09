#pragma once

#include <QImage>
#include <QString>

#include "d1gfx.h"
#include "d1pal.h"

// alpha value under which the color is considered as transparent
#define COLOR_ALPHA_LIMIT 128

class D1ImageFrame {
public:
    static bool load(D1GfxFrame &frame, const QImage &image, D1Pal *pal);
};
