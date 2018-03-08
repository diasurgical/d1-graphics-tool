#ifndef D1CEL
#define D1CEL

#include "d1celbase.h"

// Special level frames pixel structures
extern const bool D1CEL_LEVEL_FRAME_TYPE_2[512];
extern const quint16 D1CEL_LEVEL_FRAME_TYPE_2_ZEROED_BYTES[32];
extern const bool D1CEL_LEVEL_FRAME_TYPE_3[512];
extern const quint16 D1CEL_LEVEL_FRAME_TYPE_3_ZEROED_BYTES[32];
extern const bool D1CEL_LEVEL_FRAME_TYPE_4[512];
extern const quint16 D1CEL_LEVEL_FRAME_TYPE_4_ZEROED_BYTES[16];
extern const bool D1CEL_LEVEL_FRAME_TYPE_5[512];
extern const quint16 D1CEL_LEVEL_FRAME_TYPE_5_ZEROED_BYTES[16];

// Class used only for CEL frame width calculation
class D1CelPixelGroup
{
public:
    D1CelPixelGroup();
    D1CelPixelGroup( bool, quint16 );

    bool isTransparent();
    quint16 getPixelCount();

private:
    bool transparent;
    quint16 pixelCount;
};

enum class D1CEL_FRAME_TYPE
{
    REGULAR,      // == LEVEL_TYPE_1
    LEVEL_TYPE_0, // 0x400 full opaque
    LEVEL_TYPE_2, // 0x220 left transparency
    LEVEL_TYPE_3, // 0x220 right transparency
    LEVEL_TYPE_4, // 0x320 left transparency
    LEVEL_TYPE_5  // 0x320 right transparency
};

class D1CelFrame : public D1CelFrameBase
{
public:
    D1CelFrame();
    D1CelFrame( QByteArray, quint16, quint16, D1CEL_FRAME_TYPE );

    static D1CEL_FRAME_TYPE getLevelFrame220Type( QByteArray & );
    static D1CEL_FRAME_TYPE getLevelFrame320Type( QByteArray & );
    static D1CEL_FRAME_TYPE getLevelFrame400Type( QByteArray & );
    quint16 computeWidthFromHeader( QByteArray & );
    quint16 computeWidthFromData( QByteArray & );
    bool load( QByteArray );

private:
    D1CEL_FRAME_TYPE frameType;
};

class D1Cel : public D1CelBase
{
public:
    D1Cel();
    D1Cel( QString, D1Pal* );

    bool load( QString );
};

#endif // D1CEL
