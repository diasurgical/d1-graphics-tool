#ifndef D1MIN_H
#define D1MIN_H

#include <QFile>
#include <QImage>
#include <QPainter>

#include "d1cel.h"

enum class D1MIN_TYPE {
    REGULAR_HEIGHT,
    EXTENDED_HEIGHT
};

class D1Min : public QObject {
    Q_OBJECT

public:
    D1Min() = default;
    D1Min(QString, D1Cel *);
    ~D1Min();

    bool load(QString);
    QImage getSubtileImage(quint16);

    D1MIN_TYPE getType();
    QString getFilePath();
    D1CelBase *getCel();
    void setCel(D1CelBase *c);
    quint16 getSubtileWidth();
    quint16 getSubtileHeight();
    quint16 getSubtileCount();
    QList<quint16> getCelFrameIndices(quint16);

private:
    D1MIN_TYPE type = D1MIN_TYPE::REGULAR_HEIGHT;
    QFile file;
    D1CelBase *cel = nullptr;
    quint8 subtileWidth = 2;
    quint8 subtileHeight = 5;
    quint16 subtileCount = 0;
    QList<QList<quint16>> celFrameIndices;
    QList<QList<quint8>> celFrameTypes;
};

#endif // D1MIN_H
