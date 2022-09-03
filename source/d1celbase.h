#pragma once

#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QPointer>

#include "d1pal.h"

class D1CelPixel {
public:
    D1CelPixel() = default;
    D1CelPixel(bool, quint8);
    ~D1CelPixel() = default;

    bool isTransparent() const;
    quint8 getPaletteIndex();

private:
    bool transparent = false;
    quint8 paletteIndex = 0;
};

class D1CelFrameBase : public QObject {
    Q_OBJECT

public:
    D1CelFrameBase() = default;
    ~D1CelFrameBase() = default;

    virtual quint16 computeWidthFromHeader(QByteArray &) = 0;
    virtual bool load(QByteArray) = 0;

    quint16 getWidth();
    quint16 getHeight();
    D1CelPixel getPixel(quint16, quint16);

protected:
    quint16 width = 0;
    quint16 height = 0;
    QList<QList<D1CelPixel>> pixels;
};

enum class D1CEL_TYPE {
    NONE,
    V1_REGULAR,
    V1_COMPILATION,
    V1_LEVEL,
    V2_MONO_GROUP,
    V2_MULTIPLE_GROUPS
};

class D1CelBase : public QObject {
    Q_OBJECT

public:
    D1CelBase() = default;
    D1CelBase(D1Pal *pal);
    ~D1CelBase();

    virtual bool load(QString) = 0;
    bool isFrameSizeConstant();
    QImage getFrameImage(quint16);

    D1CEL_TYPE getType();
    QString getFilePath();
    D1Pal *getPalette();
    void setPalette(D1Pal *pal);
    quint16 getGroupCount();
    QPair<quint16, quint16> getGroupFrameIndices(quint16);
    quint32 getFrameCount();
    D1CelFrameBase *getFrame(quint16);
    quint16 getFrameWidth(quint16);
    quint16 getFrameHeight(quint16);

protected:
    D1CEL_TYPE type = D1CEL_TYPE::NONE;
    QFile file;
    D1Pal *palette = nullptr;
    quint16 groupCount = 0;
    QList<QPair<quint16, quint16>> groupFrameIndices;
    quint32 frameCount = 0;
    QList<QPair<quint32, quint32>> frameOffsets;
    QList<QPointer<D1CelFrameBase>> frames;
};
