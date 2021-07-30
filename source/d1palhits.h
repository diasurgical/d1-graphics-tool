#ifndef D1PALHITS_H
#define D1PALHITS_H

#include <QMap>

#include "d1pal.h"
#include "d1cel.h"
#include "d1cl2.h"
#include "d1min.h"
#include "d1til.h"

enum class D1PALHITS_MODE
{
    ALL_COLORS,
    ALL_FRAMES,
    CURRENT_TILE,
    CURRENT_SUBTILE,
    CURRENT_FRAME
};


class D1PalHits : public QObject
{
    Q_OBJECT

public:
    D1PalHits( D1Pal*, D1CelBase* );
    D1PalHits( D1Pal*, D1CelBase*, D1Min*, D1Til* );

    D1PALHITS_MODE getMode();
    void setMode( D1PALHITS_MODE );

    void buildPalHits();
    void buildSubtilePalHits();
    void buildTilePalHits();

    // Returns the number of hits for a specific index
    quint32 getIndexHits( quint8 );
    quint32 getIndexHits( quint8, quint32 );

private:

    bool isLevelCel;

    D1PALHITS_MODE mode;

    D1Pal *pal;
    D1CelBase *cel;
    D1Min *min;
    D1Til *til;

    // Palette hits are stored with a palette index key and a hit count value
    QMap<quint8,quint32> allFramesPalHits;
    QMap<quint32,QMap<quint8,quint32>> framePalHits;
    QMap<quint32,QMap<quint8,quint32>> tilePalHits;
    QMap<quint32,QMap<quint8,quint32>> subtilePalHits;
};

#endif // D1PALHITS_H
