#pragma once

#include <QMap>

#include "d1gfx.h"
#include "d1min.h"
#include "d1til.h"

enum class D1PALHITS_MODE {
    ALL_COLORS,
    ALL_FRAMES,
    CURRENT_TILE,
    CURRENT_SUBTILE,
    CURRENT_FRAME
};

class D1PalHits : public QObject {
    Q_OBJECT

public:
    D1PalHits(D1Gfx *g, D1Min *m = nullptr, D1Til *t = nullptr);

    void update();

    D1PALHITS_MODE getMode() const;
    void setMode(D1PALHITS_MODE m);

    // Returns the number of hits for a specific index
    int getIndexHits(quint8 colorIndex, int itemIndex) const;

private:
    void buildPalHits();
    void buildSubtilePalHits();
    void buildTilePalHits();

private:
    D1PALHITS_MODE mode = D1PALHITS_MODE::ALL_COLORS;

    D1Gfx *gfx;
    D1Min *min;
    D1Til *til;

    // Palette hits are stored with a palette index key and a hit count value
    QMap<quint8, int> allFramesPalHits;
    QMap<int, QMap<quint8, int>> framePalHits;
    QMap<int, QMap<quint8, int>> tilePalHits;
    QMap<int, QMap<quint8, int>> subtilePalHits;
};
