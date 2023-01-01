#pragma once

#include <QMap>

#include "d1gfx.h"
#include "d1min.h"
#include "d1sol.h"
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
    D1PalHits(D1Gfx *);
    D1PalHits(D1Gfx *g, D1Min *m, D1Til *t, D1Sol *s);

    D1PALHITS_MODE getMode();
    void setMode(D1PALHITS_MODE m);

    void buildPalHits();
    void buildSubtilePalHits();
    void buildTilePalHits();

    // Returns the number of hits for a specific index
    quint32 getIndexHits(quint8);
    quint32 getIndexHits(quint8, quint32);

private:
    bool isLevelCel = false;

    D1PALHITS_MODE mode = D1PALHITS_MODE::ALL_COLORS;

    D1Gfx *gfx;
    D1Min *min;
    D1Til *til;
    D1Sol *sol;

    // Palette hits are stored with a palette index key and a hit count value
    QMap<quint8, quint32> allFramesPalHits;
    QMap<quint32, QMap<quint8, quint32>> framePalHits;
    QMap<quint32, QMap<quint8, quint32>> tilePalHits;
    QMap<quint32, QMap<quint8, quint32>> subtilePalHits;
};
