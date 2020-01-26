#ifndef D1TIL_H
#define D1TIL_H

#include <QPointer>
#include <QFile>
#include <QBuffer>
#include <QImage>
#include <QPainter>
#include <QMap>
#include "d1min.h"

enum class D1TIL_TYPE
{
    REGULAR_HEIGHT,
    EXTENDED_HEIGHT
};

class D1Til : public QObject
{
    Q_OBJECT

public:
    D1Til();
    D1Til( QString, D1Min* );
    ~D1Til();

    bool load( QString );
    QImage getTileImage( quint16 );

    D1TIL_TYPE getType();
    QString getFilePath();
    bool isFileOpen();
    D1Min* getMin();
    void setMin( D1Min* );
    quint16 getTileCount();
    quint16 getTileWidth();
    quint16 getTileHeight();
    quint16 getTilePixelWidth();
    quint16 getTilePixelHeight();
    QList<quint16> getSubtileIndices( quint16 );

private:
    D1TIL_TYPE type;
    QFile file;
    D1Min* min;
    quint16 tileCount;
    quint16 tileWidth;
    quint16 tileHeight;
    quint16 tilePixelWidth;
    quint16 tilePixelHeight;
    QList< QList<quint16> > subtileIndices;
};

#endif // D1TIL_H
