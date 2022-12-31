#include "d1celbase.h"

D1CelPixel::D1CelPixel(bool t, quint8 pi)
    : transparent(t)
    , paletteIndex(pi)
{
}

bool D1CelPixel::isTransparent() const
{
    return this->transparent;
}

quint8 D1CelPixel::getPaletteIndex()
{
    return this->paletteIndex;
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

    return {};
}

bool D1CelFrameBase::isClipped()
{
    return this->clipped;
}

D1CelBase::~D1CelBase()
{
    qDeleteAll(this->frames);
}

bool D1CelBase::isFrameSizeConstant()
{
    if (this->frames.isEmpty()) {
        return false;
    }

    quint16 frameWidth = this->frames[0]->getWidth();
    quint16 frameHeight = this->frames[0]->getHeight();

    for (int i = 1; i < this->frames.count(); i++) {
        if (this->frames[i]->getWidth() != frameWidth
            || this->frames[i]->getHeight() != frameHeight)
            return false;
    }

    return true;
}

// builds QImage from a D1CelFrame of given index
QImage D1CelBase::getFrameImage(quint16 frameIndex)
{
    if (this->palette == nullptr || frameIndex >= this->frames.size())
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
    return this->celFilePath;
}

D1Pal *D1CelBase::getPalette()
{
    return this->palette;
}

void D1CelBase::setPalette(D1Pal *pal)
{
    this->palette = pal;
}

int D1CelBase::getGroupCount()
{
    return this->groupFrameIndices.count();
}

QPair<quint16, quint16> D1CelBase::getGroupFrameIndices(quint16 groupIndex)
{
    if (!this->groupFrameIndices.empty() && groupIndex < this->groupFrameIndices.count())
        return this->groupFrameIndices[groupIndex];

    return qMakePair(0, 0);
}

int D1CelBase::getFrameCount()
{
    return this->frames.count();
}

D1CelFrameBase *D1CelBase::getFrame(quint16 frameIndex)
{
    if (frameIndex >= this->frames.count())
        return nullptr;

    return this->frames[frameIndex];
}

quint16 D1CelBase::getFrameWidth(quint16 frameIndex)
{
    if (frameIndex >= this->frames.count())
        return 0;

    return this->frames[frameIndex]->getWidth();
}

quint16 D1CelBase::getFrameHeight(quint16 frameIndex)
{
    if (frameIndex >= this->frames.count())
        return 0;

    return this->frames[frameIndex]->getHeight();
}
