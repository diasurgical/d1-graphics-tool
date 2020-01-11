#ifndef D1CELCORE_H
#define D1CELCORE_H

#include <QPointer>
#include <QFile>
#include <QBuffer>
#include <QImage>
#include <QMap>
#include "d1pal.h"

class D1CelPixel
{
public:
    D1CelPixel();
    D1CelPixel( bool, quint8 );
    ~D1CelPixel();

    bool isTransparent();
    quint8 getPaletteIndex();

private:
    bool transparent;
    quint8 paletteIndex;
};

class D1CelFrameBase : public QObject
{
    Q_OBJECT

public:
    D1CelFrameBase();
    ~D1CelFrameBase();

    virtual quint16 computeWidthFromHeader( QByteArray & ) = 0;
    virtual bool load( QByteArray ) = 0;

    quint16 getWidth();
    quint16 getHeight();
    D1CelPixel getPixel( quint16, quint16 );

protected:
    quint16 width;
    quint16 height;
    QList< QList<D1CelPixel> > pixels;
};

enum class D1CEL_TYPE
{
    NONE,
    V1_REGULAR,
    V1_COMPILATION,
    V1_LEVEL,
    V2_MONO_GROUP,
    V2_MULTIPLE_GROUPS
};

class D1CelBase : public QObject
{
    Q_OBJECT

public:
    D1CelBase();
    D1CelBase( D1Pal* );
    ~D1CelBase();

    virtual bool load( QString ) = 0;
    bool isFrameSizeConstant();
    QImage getFrameImage( quint16 );

    D1CEL_TYPE getType();
    QString getFilePath();
    bool isFileOpen();
    D1Pal* getPalette();
    void setPalette( D1Pal* );
    quint16 getGroupCount();
    QPair<quint16,quint16> getGroupFrameIndices( quint16 );
    quint32 getFrameCount();
    D1CelFrameBase* getFrame( quint16 );
    quint16 getFrameWidth( quint16 );
    quint16 getFrameHeight( quint16 );

protected:
    D1CEL_TYPE type;
    QFile file;
    D1Pal* palette;
    quint16 groupCount;
    QList< QPair<quint16,quint16> > groupFrameIndices;
    quint32 frameCount;
    QList< QPair<quint32,quint32> > frameOffsets;
    QList< QPointer<D1CelFrameBase> > frames;
};

#endif // D1CELCORE_H
