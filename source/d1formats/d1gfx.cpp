#include "d1gfx.h"

#include <QPainter>

#include "d1image.h"

namespace {

QImage EmptyFramePlaceholder(QString text)
{
    QFont font("", 16);
    QFontMetrics metrics = QFontMetrics(font);
    QSize size = metrics.size(0, text);

    QImage placeholder = QImage(
        size.width(),
        size.height(),
        QImage::Format_ARGB32);

    placeholder.fill(qRgba(0, 0, 0, 0));

    QPainter painter = QPainter(&placeholder);
    painter.setPen(Qt::gray);
    painter.setFont(font);
    painter.drawText(0, metrics.ascent(), text);
    return placeholder;
}

} // namespace

D1GfxPixel D1GfxPixel::transparentPixel()
{
    D1GfxPixel pixel;
    pixel.transparent = true;
    pixel.paletteIndex = 0;
    return pixel;
}

D1GfxPixel D1GfxPixel::colorPixel(quint8 color)
{
    D1GfxPixel pixel;
    pixel.transparent = false;
    pixel.paletteIndex = color;
    return pixel;
}

bool D1GfxPixel::isTransparent() const
{
    return this->transparent;
}

quint8 D1GfxPixel::getPaletteIndex() const
{
    return this->paletteIndex;
}

bool operator==(const D1GfxPixel &lhs, const D1GfxPixel &rhs)
{
    return lhs.transparent == rhs.transparent && lhs.paletteIndex == rhs.paletteIndex;
}

int D1GfxFrame::getWidth() const
{
    return this->width;
}

int D1GfxFrame::getHeight() const
{
    return this->height;
}

D1GfxPixel D1GfxFrame::getPixel(int x, int y) const
{
    if (x >= 0 && x < this->width && y >= 0 && y < this->height)
        return this->pixels[y][x];

    return D1GfxPixel::transparentPixel();
}

D1CEL_FRAME_TYPE D1GfxFrame::getFrameType() const
{
    return this->frameType;
}

void D1GfxFrame::setFrameType(D1CEL_FRAME_TYPE type)
{
    this->frameType = type;
}

// builds QImage from a D1CelFrame of given index
QImage D1Gfx::getFrameImage(quint16 frameIndex)
{
    if (this->palette == nullptr)
        return EmptyFramePlaceholder("No palette");

    if (frameIndex >= this->frames.count())
        return EmptyFramePlaceholder("Out of bounds");

    D1GfxFrame &frame = this->frames[frameIndex];

    if (frame.getWidth() == 0 || frame.getHeight() == 0)
        return EmptyFramePlaceholder("No frame data");

    QImage image = QImage(
        frame.getWidth(),
        frame.getHeight(),
        QImage::Format_ARGB32);

    for (int y = 0; y < frame.getHeight(); y++) {
        for (int x = 0; x < frame.getWidth(); x++) {
            D1GfxPixel d1pix = frame.getPixel(x, y);

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

void D1Gfx::insertGroup(int groupIdx, int frameIdx, const QImage &image)
{
    D1GfxFrame frame;
    D1ImageFrame::load(frame, image, this->palette);
    this->frames.insert(frameIdx, frame);
    this->groupFrameIndices.insert(groupIdx, QPair<int, int>(frameIdx, frameIdx));

    // We have to increment first frame index in the group that follows the one that we have deleted,
    // but only if the current group isn't the last one
    for (int i = groupIdx + 1; i < this->groupFrameIndices.count(); i++) {
        this->groupFrameIndices[i].first++;
        this->groupFrameIndices[i].second++;
    }
}

D1GfxFrame *D1Gfx::insertFrame(int frameIdx, const QImage &image)
{
    D1GfxFrame frame;
    D1ImageFrame::load(frame, image, this->palette);
    this->frames.insert(frameIdx, frame);

    if (this->groupFrameIndices.isEmpty()) {
        // create new group if this is the first frame
        this->groupFrameIndices.append(qMakePair(0, 0));
    } else if (this->frames.count() == frameIdx + 1) {
        // extend the last group if appending a frame
        this->groupFrameIndices.last().second = frameIdx;
    } else {
        // extend the current group and adjust every group after it
        for (int i = 0; i < this->groupFrameIndices.count(); i++) {
            if (this->groupFrameIndices[i].second < frameIdx)
                continue;
            if (this->groupFrameIndices[i].first > frameIdx) {
                this->groupFrameIndices[i].first++;
            }
            this->groupFrameIndices[i].second++;
        }
    }

    this->modified = true;
    return &this->frames[frameIdx];
}

void D1Gfx::insertFrameInGroup(int frameIdx, int groupIdx, const QImage &image)
{
    D1GfxFrame frame;
    D1ImageFrame::load(frame, image, this->palette);
    this->frames.insert(frameIdx, frame);

    this->groupFrameIndices[groupIdx].second++;

    // shift all indices in groups AFTER group in which frame has been added
    for (int i = groupIdx + 1; i < this->groupFrameIndices.count(); i++) {
        this->groupFrameIndices[i].first++;
        this->groupFrameIndices[i].second++;
    }

    this->modified = true;
}

D1GfxFrame *D1Gfx::replaceFrame(int idx, const QImage &image)
{
    D1GfxFrame frame;
    D1ImageFrame::load(frame, image, this->palette);
    this->frames[idx] = frame;

    this->modified = true;
    return &this->frames[idx];
}

std::optional<int> D1Gfx::removeFrame(quint16 idx)
{
    this->frames.removeAt(idx);
    std::optional<int> removedGroupIdx;

    for (int i = 0; i < this->groupFrameIndices.count(); i++) {
        if (this->groupFrameIndices[i].second < idx)
            continue;
        if (this->groupFrameIndices[i].second == idx && this->groupFrameIndices[i].first == idx) {
            removedGroupIdx.emplace(i);
            this->groupFrameIndices.removeAt(i);
            i--;
            continue;
        }
        if (this->groupFrameIndices[i].first > idx) {
            this->groupFrameIndices[i].first--;
        }
        this->groupFrameIndices[i].second--;
    }
    this->modified = true;

    return removedGroupIdx;
}

void D1Gfx::regroupFrames(int numGroups)
{
    const int numFrames = this->frames.count();

    // update group indices
    this->groupFrameIndices.clear();
    for (int i = 0; i < numGroups; i++) {
        int ni = numFrames / numGroups;
        this->groupFrameIndices.append(qMakePair(i * ni, i * ni + ni - 1));
    }
    this->modified = true;
}

void D1Gfx::remapFrames(const QMap<unsigned, unsigned> &remap)
{
    QList<D1GfxFrame> newFrames;
    // assert(this->groupFrameIndices.count() == 1);
    for (auto iter = remap.cbegin(); iter != remap.cend(); ++iter) {
        newFrames.append(this->frames.at(iter.value() - 1));
    }
    this->frames.swap(newFrames);
    this->modified = true;
}

bool D1Gfx::isModified() const
{
    return this->modified;
}

void D1Gfx::setModified(bool isModified)
{
    this->modified = isModified;
}

bool D1Gfx::isTileset() const
{
    return this->isTileset_;
}

bool D1Gfx::hasHeader() const
{
    return this->hasHeader_;
}

void D1Gfx::setHasHeader(bool hasHeader)
{
    this->hasHeader_ = hasHeader;
}

QString D1Gfx::getFilePath()
{
    return this->gfxFilePath;
}

D1Pal *D1Gfx::getPalette()
{
    return this->palette;
}

void D1Gfx::setPalette(D1Pal *pal)
{
    this->palette = pal;
}

int D1Gfx::getGroupCount()
{
    return this->groupFrameIndices.count();
}

QPair<quint16, quint16> D1Gfx::getGroupFrameIndices(int groupIndex)
{
    if (groupIndex < 0 || groupIndex >= this->groupFrameIndices.count())
        return qMakePair(0, 0);

    return this->groupFrameIndices[groupIndex];
}

int D1Gfx::getFrameCount()
{
    return this->frames.count();
}

D1GfxFrame *D1Gfx::getFrame(int frameIndex)
{
    if (frameIndex < 0 || frameIndex >= this->frames.count())
        return nullptr;

    return &this->frames[frameIndex];
}

int D1Gfx::getFrameWidth(int frameIndex)
{
    if (frameIndex < 0 || frameIndex >= this->frames.count())
        return 0;

    return this->frames[frameIndex].getWidth();
}

int D1Gfx::getFrameHeight(int frameIndex)
{
    if (frameIndex < 0 || frameIndex >= this->frames.count())
        return 0;

    return this->frames[frameIndex].getHeight();
}
