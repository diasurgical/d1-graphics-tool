#include "d1palhits.h"

D1PalHits::D1PalHits( D1CelBase* c ) :
    isLevelCel( false ),
    mode( D1PALHITS_MODE::ALL_COLORS ),
    cel( c )
{
    this->buildPalHits();
}

D1PalHits::D1PalHits( D1CelBase* c, D1Min* m, D1Til* t ) :
    isLevelCel( true ),
    mode( D1PALHITS_MODE::ALL_COLORS ),
    cel( c ),
    min( m ),
    til( t )
{
    this->buildPalHits();
    this->buildSubtilePalHits();
    this->buildTilePalHits();
}

D1PALHITS_MODE D1PalHits::getMode()
{
    return this->mode;
}

void D1PalHits::setMode( D1PALHITS_MODE m )
{
    this->mode = m;
}

void D1PalHits::buildPalHits()
{
    D1CelFrameBase* frame = NULL;
    quint8 paletteIndex = 0;

    // Go through all frames
    for( quint32 i = 0; i < this->cel->getFrameCount(); i++ )
    {
        QMap<quint8,quint32> frameHits;

        // Get frame pointer
        frame = this->cel->getFrame(i);

        // Go through every pixels of the frame
        for( int jx = 0; jx < frame->getWidth(); jx++ )
        {
            for( int jy = 0; jy < frame->getHeight(); jy++ )
            {
                // Retrieve the color of the pixel
                paletteIndex = frame->getPixel(jx,jy).getPaletteIndex();

                // Add one hit to the frameHits and allFramesPalHits maps
                if( frameHits.contains(paletteIndex) )
                    frameHits.insert(paletteIndex,frameHits.value(paletteIndex)+1);
                else
                    frameHits.insert(paletteIndex,1);

                if( frameHits.contains(paletteIndex) )
                    this->allFramesPalHits.insert(paletteIndex,frameHits.value(paletteIndex)+1);
                else
                    this->allFramesPalHits.insert(paletteIndex,1);
            }
        }

        this->framePalHits[i] = frameHits;
    }
}

void D1PalHits::buildSubtilePalHits()
{
    QList<quint16> celFrameIndices;
    quint16 frameIndex;

    // Go through all sub-tiles
    for( int i = 0; i < this->min->getSubtileCount(); i++ )
    {
        QMap<quint8,quint32> subtileHits;

        // Retrieve the CEL frame indices of the current sub-tile
        celFrameIndices = this->min->getCelFrameIndices( i );

        // Go through the CEL frames
        QListIterator<quint16> it1( celFrameIndices );
        while( it1.hasNext() )
        {
            frameIndex = it1.next() - 1;

            // Go through the hits of the CEL frame and add them to the subtile hits
            QMapIterator<quint8,quint32> it2( this->framePalHits.value(frameIndex) );
            while( it2.hasNext() )
            {
                it2.next();
                subtileHits.insert( it2.key(), it2.value() );
            }
        }

        this->subtilePalHits[i] = subtileHits;
    }
}

void D1PalHits::buildTilePalHits()
{
    QList<quint16> subtileIndices;
    quint16 subtileIndex;

    // Go through all tiles
    for( int i = 0; i < this->til->getTileCount(); i++ )
    {
        QMap<quint8,quint32> tileHits;

        // Retrieve the sub-tile indices of the current tile
        subtileIndices = this->til->getSubtileIndices( i );

        // Go through the sub-tiles
        QListIterator<quint16> it1( subtileIndices );
        while( it1.hasNext() )
        {
            subtileIndex = it1.next();

            // Go through the hits of the sub-tile and add them to the tile hits
            QMapIterator<quint8,quint32> it2( this->subtilePalHits.value(subtileIndex) );
            while( it2.hasNext() )
            {
                it2.next();
                tileHits.insert( it2.key(), it2.value() );
            }
        }

        this->tilePalHits[i] = tileHits;
    }
}

quint32 D1PalHits::getIndexHits( quint8 colorIndex )
{
    if( this->mode == D1PALHITS_MODE::ALL_COLORS )
    {
        return 1;
    }
    else if( this->mode == D1PALHITS_MODE::ALL_FRAMES )
    {
        if( !this->allFramesPalHits.contains(colorIndex) )
            return 0;
        else
            return this->allFramesPalHits[colorIndex];
    }
    else
    {
        return 0;
    }
}

quint32 D1PalHits::getIndexHits( quint8 colorIndex, quint32 itemIndex )
{
    if( this->mode == D1PALHITS_MODE::CURRENT_TILE )
    {
        if( !this->tilePalHits[itemIndex].contains(colorIndex) )
            return 0;
        else
            return this->tilePalHits[itemIndex][colorIndex];
    }
    else if( this->mode == D1PALHITS_MODE::CURRENT_SUBTILE )
    {
        if( !this->subtilePalHits[itemIndex].contains(colorIndex) )
            return 0;
        else
            return this->subtilePalHits[itemIndex][colorIndex];
    }
    else if( this->mode == D1PALHITS_MODE::CURRENT_FRAME )
    {
        if( !this->framePalHits[itemIndex].contains(colorIndex) )
            return 0;
        else
            return this->framePalHits[itemIndex][colorIndex];
    }
    else
    {
        return 0;
    }
}
