#include "d1image.h"

#include <QColor>
#include <QImage>
#include <QList>

static quint8 getPalColor(D1Pal *pal, QColor color)
{
    int res = 0;
    /* int bestR = abs(color.red() - 0)
       int bestG = abs(color.green() - 0)
       int bestB = abs(color.blue() - 0); */
    int bestR = (color.red() - 0) * (color.red() - 0);
    int bestG = (color.green() - 0) * (color.green() - 0);
    int bestB = (color.blue() - 0) * (color.blue() - 0);
    int best = bestR + bestG + bestB;

    for (int i = 0; i < D1PAL_COLORS; i++) {
        QColor palColor = pal->getColor(i);
        /* int currR = abs(color.red() - palColor.red())
           int currG = abs(color.green() - palColor.green())
           int currB = abs(color.blue() - palColor.blue());*/
        int currR = (color.red() - palColor.red()) * (color.red() - palColor.red());
        int currG = (color.green() - palColor.green()) * (color.green() - palColor.green());
        int currB = (color.blue() - palColor.blue()) * (color.blue() - palColor.blue());
        int curr = currR + currG + currB;
        if (curr < best) {
            best = curr;
            res = i;
        }
    }

    return res;
}

bool D1ImageFrame::load(D1GfxFrame &frame, QImage &image, bool clipped, D1Pal *pal)
{
    frame.clipped = clipped;
    frame.width = image.width();
    frame.height = image.height();

    frame.pixels.clear();

    for (int y = 0; y < frame.height; y++) {
        QList<D1GfxPixel> pixelLine;
        for (int x = 0; x < frame.width; x++) {
            QColor color = image.pixelColor(x, y);
            // if (color == QColor(Qt::transparent)) {
            if (color.alpha() < 128) {
                pixelLine.append(D1GfxPixel(true, 0));
            } else {
                pixelLine.append(D1GfxPixel(false, getPalColor(pal, color)));
            }
        }
        frame.pixels.append(pixelLine);
    }

    return true;
}
