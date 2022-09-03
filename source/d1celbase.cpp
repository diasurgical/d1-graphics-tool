#include "d1celbase.h"

D1CelPixel::D1CelPixel()
    : transparent(false)
    , paletteIndex(0)
{
}

D1CelPixel::D1CelPixel(bool t, quint8 pi)
    : transparent(t)
    , paletteIndex(pi)
{
}

D1CelPixel::~D1CelPixel()
{
}

bool D1CelPixel::isTransparent()
{
    return this->transparent;
}

quint8 D1CelPixel::getPaletteIndex()
{
    return this->paletteIndex;
}

D1CelFrameBase::D1CelFrameBase()
    : width(0)
    , height(0)
{
}

D1CelFrameBase::~D1CelFrameBase()
{
}

quint16 D1CelFrameBase::getWidth()
{
    return this->width;
}

quint16 D1CelFrameBase::getHeight()
{
    return this->height;
}

D1CelPixel D1CelFrameBase::getPixel(quint16 x, quint16 y)
{
    if (x < this->width && y < this->height)
        return this->pixels[y][x];
    else
        return D1CelPixel();
}

D1CelBase::D1CelBase()
    : type(D1CEL_TYPE::NONE)
    , file()
    , palette(NULL)
    , groupCount(0)
    , frameCount(0)
{
}

D1CelBase::D1CelBase(D1Pal *pal)
    : type(D1CEL_TYPE::NONE)
    , file()
    , palette(pal)
    , groupCount(0)
    , frameCount(0)
{
}

D1CelBase::~D1CelBase()
{
    qDeleteAll(this->frames);

    if (this->file.isOpen())
        this->file.close();
}

bool D1CelBase::isFrameSizeConstant()
{
    if (this->frameCount == 0)
        return false;

    quint16 frameWidth = this->getFrameWidth(0);
    quint16 frameHeight = this->getFrameHeight(0);

    for (unsigned int i = 1; i < this->frameCount; i++) {
        if (this->getFrameWidth(i) != frameWidth
            || this->getFrameHeight(i) != frameHeight)
            return false;
    }

    return true;
}

// builds QImage from a D1CelFrame of given index
QImage D1CelBase::getFrameImage(quint16 frameIndex)
{
    if (this->palette == NULL || frameIndex >= this->frames.size())
        return QImage();

    QPointer<D1CelFrameBase> frame = this->frames[frameIndex];

    QImage image = QImage(
        frame->getWidth(),
        frame->getHeight(),
        QImage::Format_ARGB32);

    for (int y = 0; y < frame->getHeight(); y++) {
        for (int x = 0; x < frame->getWidth(); x++) {
            D1CelPixel d1pix = frame->getPixel(x, y);

            QColor color;
            if (d1pix.isTransparent())
                color = QColor(Qt::transparent);
            else
                color = this->palette->getColor(d1pix.getPaletteIndex());

            image.setPixel(x, y, color.rgba());
        }
    }

    return image;
}

D1CEL_TYPE D1CelBase::getType()
{
    return this->type;
}

QString D1CelBase::getFilePath()
{
    if (this->file.isOpen())
        return this->file.fileName();
    else
        return QString();
}

bool D1CelBase::isFileOpen()
{
    return this->file.isOpen();
}

D1Pal *D1CelBase::getPalette()
{
    return this->palette;
}

void D1CelBase::setPalette(D1Pal *pal)
{
    this->palette = pal;
}

quint16 D1CelBase::getGroupCount()
{
    return this->groupCount;
}

QPair<quint16, quint16> D1CelBase::getGroupFrameIndices(quint16 groupIndex)
{
    if (!this->groupFrameIndices.empty() && groupIndex < this->groupCount)
        return this->groupFrameIndices[groupIndex];
    else
        return qMakePair(0, 0);
}

quint32 D1CelBase::getFrameCount()
{
    return this->frameCount;
}

D1CelFrameBase *D1CelBase::getFrame(quint16 frameIndex)
{
    if (frameIndex < this->frameCount)
        return this->frames[frameIndex];
    else
        return NULL;
}

quint16 D1CelBase::getFrameWidth(quint16 frameIndex)
{
    if (frameIndex < this->frameCount)
        return this->frames[frameIndex]->getWidth();
    else
        return 0;
}

quint16 D1CelBase::getFrameHeight(quint16 frameIndex)
{
    if (frameIndex < this->frameCount)
        return this->frames[frameIndex]->getHeight();
    else
        return 0;
}
