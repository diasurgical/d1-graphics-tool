#include "d1palhits.h"

D1PalHits::D1PalHits(D1Gfx *g, D1Min *m, D1Til *t)
    : gfx(g)
    , min(m)
    , til(t)
{
    this->update();
}

void D1PalHits::update()
{
    this->buildPalHits();
    this->buildSubtilePalHits();
    this->buildTilePalHits();
}

D1PALHITS_MODE D1PalHits::getMode() const
{
    return this->mode;
}

void D1PalHits::setMode(D1PALHITS_MODE m)
{
    this->mode = m;
}

void D1PalHits::buildPalHits()
{
    // Go through all frames
    this->framePalHits.clear();
    this->allFramesPalHits.clear();
    for (int i = 0; i < this->gfx->getFrameCount(); i++) {
        QMap<quint8, int> frameHits;

        // Get frame pointer
        D1GfxFrame *frame = this->gfx->getFrame(i);

        // Go through every pixels of the frame
        for (int x = 0; x < frame->getWidth(); x++) {
            for (int y = 0; y < frame->getHeight(); y++) {
                // Retrieve the color of the pixel
                D1GfxPixel pixel = frame->getPixel(x, y);
                if (pixel.isTransparent())
                    continue;
                quint8 paletteIndex = pixel.getPaletteIndex();

                // Add one hit to the frameHits and allFramesPalHits maps
                frameHits.insert(paletteIndex, frameHits.value(paletteIndex) + 1);

                this->allFramesPalHits.insert(paletteIndex, frameHits.value(paletteIndex) + 1);
            }
        }

        this->framePalHits[i] = frameHits;
    }
}

void D1PalHits::buildSubtilePalHits()
{
    this->subtilePalHits.clear();

    if (this->min == nullptr) {
        return;
    }
    // Go through all sub-tiles
    for (int i = 0; i < this->min->getSubtileCount(); i++) {
        QMap<quint8, int> subtileHits;

        // Retrieve the CEL frame indices of the current sub-tile
        QList<quint16> &celFrameIndices = this->min->getCelFrameIndices(i);

        // Go through the CEL frames
        for (quint16 frameIndex : celFrameIndices) {
            frameIndex--;

            // Go through the hits of the CEL frame and add them to the subtile hits
            QMapIterator<quint8, int> it2(this->framePalHits.value(frameIndex));
            while (it2.hasNext()) {
                it2.next();
                subtileHits.insert(it2.key(), it2.value());
            }
        }

        this->subtilePalHits[i] = subtileHits;
    }
}

void D1PalHits::buildTilePalHits()
{
    this->tilePalHits.clear();
    if (this->til == nullptr) {
        return;
    }
    // Go through all tiles
    for (int i = 0; i < this->til->getTileCount(); i++) {
        QMap<quint8, int> tileHits;

        // Retrieve the sub-tile indices of the current tile
        QList<quint16> &subtileIndices = this->til->getSubtileIndices(i);

        // Go through the sub-tiles
        for (quint16 subtileIndex : subtileIndices) {
            // Go through the hits of the sub-tile and add them to the tile hits
            QMapIterator<quint8, int> it2(this->subtilePalHits.value(subtileIndex));
            while (it2.hasNext()) {
                it2.next();
                tileHits.insert(it2.key(), it2.value());
            }
        }

        this->tilePalHits[i] = tileHits;
    }
}

int D1PalHits::getIndexHits(quint8 colorIndex, int itemIndex) const
{
    switch (this->mode) {
    case D1PALHITS_MODE::ALL_COLORS:
        return 1;
    case D1PALHITS_MODE::ALL_FRAMES:
        if (this->allFramesPalHits.contains(colorIndex))
            return this->allFramesPalHits[colorIndex];
        break;
    case D1PALHITS_MODE::CURRENT_TILE:
        if (this->tilePalHits.contains(itemIndex) && this->tilePalHits[itemIndex].contains(colorIndex))
            return this->tilePalHits[itemIndex][colorIndex];
        break;
    case D1PALHITS_MODE::CURRENT_SUBTILE:
        if (this->subtilePalHits.contains(itemIndex) && this->subtilePalHits[itemIndex].contains(colorIndex))
            return this->subtilePalHits[itemIndex][colorIndex];
        break;
    case D1PALHITS_MODE::CURRENT_FRAME:
        if (this->framePalHits.contains(itemIndex) && this->framePalHits[itemIndex].contains(colorIndex))
            return this->framePalHits[itemIndex][colorIndex];
        break;
    }

    return 0;
}
