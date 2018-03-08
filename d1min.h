#ifndef D1MIN_H
#define D1MIN_H

#include <QPointer>
#include <QFile>
#include <QBuffer>
#include <QImage>
#include <QPainter>
#include <QMap>
#include "d1cel.h"

enum class D1MIN_TYPE
{
    REGULAR_HEIGHT,
    EXTENDED_HEIGHT
};

class D1Min : public QObject
{
    Q_OBJECT

public:
    D1Min();
    D1Min( QString, D1Cel* );
    ~D1Min();

    bool load( QString );
    QImage getSubtileImage( quint16 );

    D1MIN_TYPE getType();
    QString getFilePath();
    bool isFileOpen();
    D1CelBase* getCel();
    void setCel( D1CelBase* );
    quint16 getSubtileWidth();
    quint16 getSubtileHeight();
    quint16 getSubtileCount();

private:
    D1MIN_TYPE type;
    QFile file;
    D1CelBase* cel;
    quint8 subtileWidth;
    quint8 subtileHeight;
    quint16 subtileCount;
    QList< QList<quint16> > celFrameIndices;
    QList< QList<quint8> > celFrameTypes;
};

#endif // D1MIN_H
