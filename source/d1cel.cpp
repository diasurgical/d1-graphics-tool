#include "d1cel.h"

const bool D1CEL_LEVEL_FRAME_TYPE_2[512] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const quint16 D1CEL_LEVEL_FRAME_TYPE_2_ZEROED_BYTES[32] =
{
    0, 1, 8, 9, 24, 25, 48, 49, 80, 81, 120, 121, 168, 169, 224, 225,
    288, 289, 348, 349, 400, 401, 444, 445, 480, 481, 508, 509, 528, 529, 540, 541
};

const bool D1CEL_LEVEL_FRAME_TYPE_3[512] =
{
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const quint16 D1CEL_LEVEL_FRAME_TYPE_3_ZEROED_BYTES[32] =
{
    2, 3, 14, 15, 34, 35, 62, 63, 98, 99, 142, 143, 194, 195, 254, 255,
    318, 319, 374, 375, 422, 423, 462, 463, 494, 495, 518, 519, 534, 535, 542, 543
};

const bool D1CEL_LEVEL_FRAME_TYPE_4[512] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

const quint16 D1CEL_LEVEL_FRAME_TYPE_4_ZEROED_BYTES[16] =
{
    0, 1, 8, 9, 24, 25, 48, 49, 80, 81, 120, 121, 168, 169, 224, 225
};

const bool D1CEL_LEVEL_FRAME_TYPE_5[512] =
{
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

const quint16 D1CEL_LEVEL_FRAME_TYPE_5_ZEROED_BYTES[16] =
{
    2, 3, 14, 15, 34, 35, 62, 63, 98, 99, 142, 143, 194, 195, 254, 255
};

D1CelPixelGroup::D1CelPixelGroup() :
    transparent( false ),
    pixelCount( 0 )
{}

D1CelPixelGroup::D1CelPixelGroup( bool t, quint16 c ) :
    transparent( t ),
    pixelCount( c )
{}

bool D1CelPixelGroup::isTransparent()
{
    return this->transparent;
}

quint16 D1CelPixelGroup::getPixelCount()
{
    return this->pixelCount;
}


D1CelFrame::D1CelFrame() :
    D1CelFrameBase(),
    frameType( D1CEL_FRAME_TYPE::REGULAR )
{}

D1CelFrame::D1CelFrame( QByteArray rawData, quint16 w, quint16 h, D1CEL_FRAME_TYPE type ) :
    D1CelFrameBase(),
    frameType( type )
{
    this->width = w;
    this->height = h;
    this->load( rawData );
}

D1CEL_FRAME_TYPE D1CelFrame::getLevelFrame220Type( QByteArray &rawFrameData )
{
    D1CEL_FRAME_TYPE type = D1CEL_FRAME_TYPE::REGULAR;
    quint8 readByte = 0;

    for( int i = 0; i < 32; i++ )
    {
        readByte = rawFrameData[ D1CEL_LEVEL_FRAME_TYPE_2_ZEROED_BYTES[i] ];
        if( readByte != 0 )
            break;
        if( i == 31 )
            type = D1CEL_FRAME_TYPE::LEVEL_TYPE_2;
    }

    for( int i = 0; i < 32; i++ )
    {
        readByte = rawFrameData[ D1CEL_LEVEL_FRAME_TYPE_3_ZEROED_BYTES[i] ];
        if( readByte != 0 )
            break;
        if( i == 31 )
            type = D1CEL_FRAME_TYPE::LEVEL_TYPE_3;
    }

    return type;
}

D1CEL_FRAME_TYPE D1CelFrame::getLevelFrame320Type( QByteArray &rawFrameData )
{
    D1CEL_FRAME_TYPE type = D1CEL_FRAME_TYPE::REGULAR;
    quint8 readByte = 0;

    for( int i = 0; i < 16; i++ )
    {
        readByte = rawFrameData[ D1CEL_LEVEL_FRAME_TYPE_4_ZEROED_BYTES[i] ];
        if( readByte != 0 )
            break;
        if( i == 15 )
            type = D1CEL_FRAME_TYPE::LEVEL_TYPE_4;
    }

    for( int i = 0; i < 16; i++ )
    {
        readByte = rawFrameData[ D1CEL_LEVEL_FRAME_TYPE_5_ZEROED_BYTES[i] ];
        if( readByte != 0 )
            break;
        if( i == 15 )
            type = D1CEL_FRAME_TYPE::LEVEL_TYPE_5;
    }

    return type;
}

D1CEL_FRAME_TYPE D1CelFrame::getLevelFrame400Type( QByteArray &rawFrameData )
{
    quint8 readByte = 0;
    quint32 globalPixelCount = 0;
    quint16 pixelCount = 0;

    // Going through the frame data to find pixel groups
    for( int o = 0; o < rawFrameData.size(); o++ )
    {
        readByte = rawFrameData[o];

        // Transparent pixels group
        if( readByte > 0x80 )
        {
            pixelCount += ( 256 - readByte );
            globalPixelCount += pixelCount;
            if( pixelCount > 32 )
                return D1CEL_FRAME_TYPE::LEVEL_TYPE_0;
            pixelCount = 0;
        }
        else if( readByte == 0x80 )
        {
            pixelCount += 0x80;
            if( pixelCount > 32 )
                return D1CEL_FRAME_TYPE::LEVEL_TYPE_0;
        }
        // Palette indices pixel group
        else if( readByte == 0x7F )
        {
            pixelCount += 0x7F;
            if( pixelCount > 32 || o + 0x7F >= rawFrameData.size() )
                return D1CEL_FRAME_TYPE::LEVEL_TYPE_0;
            o += 0x7F;
        }
        else
        {
            pixelCount += readByte;
            globalPixelCount += pixelCount;
            if( pixelCount > 32 || o + readByte >= rawFrameData.size() )
                return D1CEL_FRAME_TYPE::LEVEL_TYPE_0;
            pixelCount = 0;
            o += readByte;
        }
    }

    if( globalPixelCount == 32*32 )
        return D1CEL_FRAME_TYPE::REGULAR;
    else
        return D1CEL_FRAME_TYPE::LEVEL_TYPE_0;
}

quint16 D1CelFrame::computeWidthFromHeader( QByteArray &rawFrameData )
{
    // Reading the frame header
    QDataStream in( rawFrameData );
    in.setByteOrder( QDataStream::LittleEndian );

    quint16 celFrameHeader[5];
    quint16 celFrameWidth[4] = { 0, 0, 0, 0 };
    quint16 pixelCount = 0;
    quint8 readByte = 0;

    // Read the {CEL FRAME HEADER}
    for( int i = 0; i < 5; i++ )
        in >> celFrameHeader[i];

    // Read the five 32 pixel-lines block to calculate the image width
    for( int i = 0; i < 4; i++ )
    {
        if( celFrameHeader[i+1] == 0 )
            break;

        for( int j = celFrameHeader[i]; j < celFrameHeader[i+1]; j++ )
        {
            readByte = rawFrameData[j];

            if( readByte > 0x7F )
            {
                pixelCount += ( 256 - readByte );
            }
            else
            {
                pixelCount += readByte;
                j += readByte;
            }
        }

        celFrameWidth[i] = pixelCount / 32;
        pixelCount = 0;
    }

    // The calculated width has to be the identical for each 32 pixel-line block
    // If it's not the case, 0 is returned
    for( int i = 0; i < 3; i++ )
    {
        if( celFrameWidth[i+1] != 0 && celFrameWidth[i] != celFrameWidth[i+1] )
            return 0;
    }

    return celFrameWidth[0];
}

quint16 D1CelFrame::computeWidthFromData( QByteArray &rawFrameData )
{
    quint32 frameDataStartOffset = 0;
    quint8 readByte = 0;
    quint32 globalPixelCount = 0;
    quint16 biggestGroupPixelCount = 0;
    quint16 pixelCount = 0;
    quint16 width = 0;
    QList<D1CelPixelGroup*> pixelGroups;

    // Checking the presence of the {CEL FRAME HEADER}
    if( (quint8)rawFrameData[0] == 0x0A && (quint8)rawFrameData[1] == 0x00 )
        frameDataStartOffset = 0x0A;

    // Going through the frame data to find pixel groups
    for( int o = frameDataStartOffset; o < rawFrameData.size(); o++ )
    {
        readByte = rawFrameData[o];

        // Transparent pixels group
        if( readByte > 0x80 )
        {
            pixelCount += ( 256 - readByte );
            pixelGroups.append( new D1CelPixelGroup(true,pixelCount) );
            globalPixelCount += pixelCount;
            if( pixelCount > biggestGroupPixelCount )
                biggestGroupPixelCount = pixelCount;
            pixelCount = 0;
        }
        else if( readByte == 0x80 )
        {
            pixelCount += 0x80;
        }
        // Palette indices pixel group
        else if( readByte == 0x7F )
        {
            pixelCount += 0x7F;
            o += 0x7F;
        }
        else
        {
            pixelCount += readByte;
            pixelGroups.append( new D1CelPixelGroup(false,pixelCount) );
            globalPixelCount += pixelCount;
            if( pixelCount > biggestGroupPixelCount )
                biggestGroupPixelCount = pixelCount;
            pixelCount = 0;
            o += readByte;
        }
    }

    // Going through pixel groups to find pixel-lines wraps
    pixelCount = 0;
    for( int i = 1; i < pixelGroups.size(); i++ )
    {
        pixelCount += pixelGroups[i-1]->getPixelCount();

        if( pixelGroups[i-1]->isTransparent() == pixelGroups[i]->isTransparent() )
        {
            // If width == 0 then it's the first pixel-line wrap and width needs to be set
            // If pixelCount is less than width then the width has to be set to the new value
            if( width == 0 || pixelCount < width )
                width = pixelCount;

            // If the pixelCount of the last group is less than the current pixel group
            // then width is equal to this last pixel group's pixel count.
            // Mostly useful for small frames like the "J" frame in smaltext.cel
            if( i == pixelGroups.size()-1 && pixelGroups[i]->getPixelCount() < pixelCount )
                width = pixelGroups[i]->getPixelCount();

            pixelCount = 0;
        }

        // If last pixel group is being processed and width is still unknown
        // then set the width to the pixelCount of the last two pixel groups
        if( i == pixelGroups.size()-1 && width == 0 )
        {
            width = pixelGroups[i-1]->getPixelCount() + pixelGroups[i]->getPixelCount();
        }
    }

    // If width wasnt found return 0
    if( width == 0 )
        return 0;

    // If width is consistent
    if( globalPixelCount % width == 0 )
    {
        return width;
    }
    // If width is inconsistent
    else
    {
        // Try to find  relevant width by adding pixel groups' pixel counts iteratively
        pixelCount = 0;
        for( int i = 0; i < pixelGroups.size(); i++ )
        {
            pixelCount += pixelGroups[i]->getPixelCount();
            if( pixelCount > 1
                && globalPixelCount % pixelCount == 0
                && pixelCount >= biggestGroupPixelCount )
            {
                return pixelCount;
            }
        }

        // If still no width found return 0
        return 0;
    }
}

bool D1CelFrame::load( QByteArray rawData )
{
    quint32 frameDataStartOffset = 0;
    quint8 readByte = 0;
    quint8 secondReadByte = 0;
    QList<D1CelPixel> pixelLine;

    if( rawData.size() == 0 )
        return false;

    // If the frame size wasnt provided then it needs to be calculated
    if( this->width == 0 )
    {
        // Checking the presence of the {CEL FRAME HEADER}
        if( (quint8)rawData[0] == 0x0A && (quint8)rawData[1] == 0x00 )
        {
            frameDataStartOffset += 0x0A;
            // If header is present, try to compute frame width from frame header
            this->width = this->computeWidthFromHeader( rawData );
        }

        // If width could not be calculated with frame header,
        // attempt to calculate it from the frame data (by identifying pixel groups line wraps)
        if( this->width == 0 )
            this->width = this->computeWidthFromData( rawData );

        // if CEL width was not found, return false
        if( this->width == 0 )
            return false;
    }

    // READ {CEL FRAME DATA}
    // if it is a CEL level frame
    if( this->frameType != D1CEL_FRAME_TYPE::REGULAR )
    {
        // 0x400 frame
        if( this->frameType == D1CEL_FRAME_TYPE::LEVEL_TYPE_0 )
        {
            for( int i = 0; i < 32; i++ )
            {
                for( int j = 0; j < 32; j++ )
                {
                    pixelLine.append( D1CelPixel(false,rawData[i*32+j]) );
                }
                this->pixels.insert( 0, pixelLine );
                pixelLine.clear();
            }
        }
        // 0x220 or 0x320 frame
        else
        {
            const bool* dataPattern = NULL;
            const quint16* dataPatternZeroedBytes = NULL;

            switch( this->frameType )
            {
                case D1CEL_FRAME_TYPE::LEVEL_TYPE_2 :
                    dataPattern = D1CEL_LEVEL_FRAME_TYPE_2;
                    dataPatternZeroedBytes = D1CEL_LEVEL_FRAME_TYPE_2_ZEROED_BYTES;
                    break;
                case D1CEL_FRAME_TYPE::LEVEL_TYPE_3 :
                    dataPattern = D1CEL_LEVEL_FRAME_TYPE_3;
                    dataPatternZeroedBytes = D1CEL_LEVEL_FRAME_TYPE_3_ZEROED_BYTES;
                    break;
                case D1CEL_FRAME_TYPE::LEVEL_TYPE_4 :
                    dataPattern = D1CEL_LEVEL_FRAME_TYPE_4;
                    dataPatternZeroedBytes = D1CEL_LEVEL_FRAME_TYPE_4_ZEROED_BYTES;
                    break;
                case D1CEL_FRAME_TYPE::LEVEL_TYPE_5 :
                    dataPattern = D1CEL_LEVEL_FRAME_TYPE_5;
                    dataPatternZeroedBytes = D1CEL_LEVEL_FRAME_TYPE_5_ZEROED_BYTES;
                    break;
                default :
                    return false;
            }

            // Going through the data structure
            int offset = 0;
            int zeroedBytesIndex = 0;
            pixelLine.clear();
            for( int i = 0; i < 512; i++ )
            {
                // if dataPattern[i] is true, then read and add 2 pixels to the line
                if( dataPattern[i] )
                {
                    readByte = rawData[offset];
                    secondReadByte = rawData[offset+1];

                    if( readByte == 0x00 && secondReadByte == 0x00
                        && offset == dataPatternZeroedBytes[zeroedBytesIndex] )
                    {
                        // Skip the 0x00 0x00 bytes
                        offset += 2;
                        // and read the next 2 bytes
                        readByte = rawData[offset];
                        secondReadByte = rawData[offset+1];

                        // move forward in the zeroed bytes structure
                        zeroedBytesIndex += 2;
                    }

                    pixelLine.append( D1CelPixel(false,readByte) );
                    pixelLine.append( D1CelPixel(false,secondReadByte) );

                    offset += 2;
                }
                // else add 2 transparent pixels to the line
                else
                {
                    pixelLine.append( D1CelPixel(true,0) );
                    pixelLine.append( D1CelPixel(true,0) );
                }

                // If it is the end of the pixel line, add the lien to the frame
                if( pixelLine.size() == 32 )
                {
                    this->pixels.insert( 0, pixelLine );
                    pixelLine.clear();
                }
            }
        }
    }
    // if it's a regular CEL frame
    else
    {
        pixelLine.clear();
        for( int o = frameDataStartOffset; o < rawData.size(); o++ )
        {
            readByte = rawData[o];

            // Transparent pixels group
            if( readByte > 0x7F )
            {
                // A pixel line can't exceed the image width
                if( (pixelLine.size()+(256-readByte)) > this->width )
                    return false;

                for( int i = 0; i < (256-readByte); i++ )
                    pixelLine.append( D1CelPixel(true,0) );
            }
            // Palette indices group
            else
            {
                // A pixel line can't exceed the image width
                if( (pixelLine.size()+readByte) > this->width )
                    return false;

                for( int i = 0; i < readByte; i++ )
                {
                    o++;
                    pixelLine.append( D1CelPixel(false,rawData[o]) );
                }
            }

            if( pixelLine.size() == this->width )
            {
                pixels.insert( 0, pixelLine );
                pixelLine.clear();
            }
        }

        if( this->height == 0 )
            this->height = pixels.size();

    }

    return true;
}


D1Cel::D1Cel() :
    D1CelBase()
{
    this->type = D1CEL_TYPE::V1_REGULAR;
}

D1Cel::D1Cel( QString path, D1Pal* pal ) :
    D1CelBase( pal )
{
    this->type = D1CEL_TYPE::V1_REGULAR;
    this->load( path );
}

bool D1Cel::load( QString celFilePath )
{
    quint32 firstDword = 0;
    quint32 fileSizeDword = 0;
    quint32 lastCelOffset = 0;
    quint32 lastCelFrameCount = 0;
    quint32 lastCelSize = 0;

    quint32 celOffset = 0;
    quint32 celFrameCount = 0;
    quint32 celFrameStartOffset = 0;
    quint32 celFrameEndOffset = 0;
    quint32 celFrameSize = 0;

    quint32 level0x400FrameCount = 0;
    quint32 level0x320FrameCount = 0;
    quint32 level0x220FrameCount = 0;

     QByteArray celFrameRawData;
     D1CEL_FRAME_TYPE celFrameType;

    // Opening CEL file with a QBuffer to load it in RAM
    if( !QFile::exists( celFilePath ) )
        return false;

    if( this->file.isOpen() )
        this->file.close();

    this->file.setFileName( celFilePath );

    if( !this->file.open(QIODevice::ReadOnly) )
        return false;

    QByteArray fileData = this->file.readAll();
    QBuffer fileBuffer( &fileData );

    if( !fileBuffer.open(QIODevice::ReadOnly) )
        return false;

    // Read CEL binary data
    QDataStream in( &fileBuffer );
    in.setByteOrder( QDataStream::LittleEndian );

    // CEL HEADER CHECKS

    // Read first DWORD
    in >> firstDword;

    // Trying to find file size in CEL header
    if( fileBuffer.size() < (4 + firstDword*4 + 4) )
        return false;

    fileBuffer.seek( firstDword*4 + 4 );
    in >> fileSizeDword;

    // If the dword is not equal to the file size then
    // check if it's a CEL compilation
    if( fileBuffer.size() != fileSizeDword )
    {
        // Read offset of the last CEL of the CEL compilation
        fileBuffer.seek( firstDword - 4 );
        in >> lastCelOffset;

        // Go to last CEL of the CEL compilation
        if( fileBuffer.size() < (lastCelOffset + 8) )
            return false;

        fileBuffer.seek( lastCelOffset );

        // Read last CEL header
        in >> lastCelFrameCount;

        // Read the last CEL size
        if( fileBuffer.size() < (lastCelOffset + 4 + lastCelFrameCount*4 + 4) )
            return false;

        fileBuffer.seek( lastCelOffset + 4 + lastCelFrameCount*4 );
        in >> lastCelSize;

        // If the last CEL size plus the last CEL offset is equal to
        // the file size then it's a CEL compilation
        if( fileBuffer.size() == (lastCelOffset + lastCelSize) )
        {
            this->type = D1CEL_TYPE::V1_COMPILATION;
            this->groupCount = firstDword/4;
        }
        else
            return false;
    }
    else
    {
        this->type = D1CEL_TYPE::V1_REGULAR;
        this->groupCount = 1;
    }

    // CEL FRAMES OFFSETS CALCULATION

    this->groupFrameIndices.clear();
    this->frameOffsets.clear();
    if( this->type == D1CEL_TYPE::V1_COMPILATION )
    {
        // Going through all CELs
        for( unsigned int i = 0; i*4 < firstDword; i++ )
        {
            fileBuffer.seek( i*4 );
            in >> celOffset;

            fileBuffer.seek( celOffset );
            in >> celFrameCount;

            this->groupFrameIndices.append(
                qMakePair( this->frameOffsets.size(),
                    this->frameOffsets.size() + celFrameCount - 1 ) );

            // Going through all frames of the CEL
            for( unsigned int j = 1; j <= celFrameCount; j++ )
            {
                celFrameStartOffset = 0;
                celFrameEndOffset = 0;

                fileBuffer.seek( celOffset + j*4 );
                in >> celFrameStartOffset;
                in >> celFrameEndOffset;

                this->frameOffsets.append(
                    qMakePair( celOffset + celFrameStartOffset,
                               celOffset + celFrameEndOffset ) );
            }
        }

    }
    else
    {
        // Going through all frames of the CEL
        for( unsigned int i = 1; i <= firstDword; i++ )
        {
            celFrameStartOffset = 0;
            celFrameEndOffset = 0;

            fileBuffer.seek( i*4 );
            in >> celFrameStartOffset;
            in >> celFrameEndOffset;

            this->frameOffsets.append(
                qMakePair( celFrameStartOffset, celFrameEndOffset ) );

            // Level CEL Check
            celFrameSize = celFrameEndOffset - celFrameStartOffset;
            if( celFrameSize == 0x400 )
                level0x400FrameCount++;
            if( celFrameSize == 0x320 )
                level0x320FrameCount++;
            if( celFrameSize == 0x220 )
                level0x220FrameCount++;
        }

        // If there is more than 50 frames of 0x400, 0x320 and 0x220 size, then
        // it's a level frame.
        if( level0x400FrameCount + level0x320FrameCount + level0x220FrameCount > 512 )
            this->type = D1CEL_TYPE::V1_LEVEL;
    }

    if( !this->frameOffsets.empty() )
        this->frameCount = this->frameOffsets.size();
    else
        return false;

    // BUILDING {CEL FRAMES}

    this->frames.clear();
    for( int i = 0; i < this->frameOffsets.size(); i++ )
    {
        celFrameSize = this->frameOffsets[i].second - this->frameOffsets[i].first;
        fileBuffer.seek( this->frameOffsets[i].first );
        celFrameRawData = fileBuffer.read( celFrameSize );

        // If it's not a level CEL
        if( this->type != D1CEL_TYPE::V1_LEVEL )
        {
            this->frames.append(
                new D1CelFrame( celFrameRawData, 0, 0, D1CEL_FRAME_TYPE::REGULAR ) );
        }
        // If it's a level CEL
        else
        {
            celFrameType = D1CEL_FRAME_TYPE::REGULAR;

            switch( celFrameSize )
            {
            case 0x400 :
                celFrameType = D1CelFrame::getLevelFrame400Type( celFrameRawData );
                break;
            case 0x320 :
                celFrameType = D1CelFrame::getLevelFrame320Type( celFrameRawData );
                break;
            case 0x220 :
                celFrameType = D1CelFrame::getLevelFrame220Type( celFrameRawData );
                break;
            default:
                break;
            }

            this->frames.append(
                new D1CelFrame( celFrameRawData, 32, 32, celFrameType ) );
        }
    }

    return true;
}
