#ifndef D1CL2
#define D1CL2

#include "d1celbase.h"

class D1Cl2Frame : public D1CelFrameBase
{
public:
    D1Cl2Frame();
    D1Cl2Frame( QByteArray );

    quint16 computeWidthFromHeader( QByteArray & );
    bool load( QByteArray );
};

class D1Cl2 : public D1CelBase
{
public:
    D1Cl2();
    D1Cl2( QString, D1Pal* );

    bool load( QString );
};

#endif // D1CL2

