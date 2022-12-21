#include "d1celframe.h"

D1CelPixelGroup::D1CelPixelGroup(bool t, quint16 c)
    : transparent(t)
    , pixelCount(c)
{
}

bool D1CelPixelGroup::isTransparent() const
{
    return this->transparent;
}

quint16 D1CelPixelGroup::getPixelCount()
{
    return this->pixelCount;
}

bool D1CelFrame::load(QByteArray rawData, OpenAsParam *params)
{
    if (rawData.size() == 0)
        return false;

    quint32 frameDataStartOffset = 0;
    quint16 width = 0;
    if (params == nullptr || params->clipped == OPEN_CLIPPING_TYPE::CLIPPED_AUTODETECT) {
        // Checking the presence of the {CEL FRAME HEADER}
        if ((quint8)rawData[0] == 0x0A && (quint8)rawData[1] == 0x00) {
            frameDataStartOffset += 0x0A;
            // If header is present, try to compute frame width from frame header
            width = this->computeWidthFromHeader(rawData);
        }
    } else {
        if (params->clipped == OPEN_CLIPPING_TYPE::CLIPPED_TRUE) {
            QDataStream in(rawData);
            in.setByteOrder(QDataStream::LittleEndian);
            quint16 offset;
            in >> offset;
            frameDataStartOffset += offset;
        }
    }
    this->width = (params == nullptr || params->width == 0) ? width : params->width;

    // If width could not be calculated with frame header,
    // attempt to calculate it from the frame data (by identifying pixel groups line wraps)
    if (this->width == 0)
        this->width = this->computeWidthFromData(rawData);

    // if CEL width was not found, return false
    if (this->width == 0)
        return false;

    // READ {CEL FRAME DATA}
    QList<D1CelPixel> pixelLine;
    for (int o = frameDataStartOffset; o < rawData.size(); o++) {
        quint8 readByte = rawData[o];

        // Transparent pixels group
        if (readByte > 0x7F) {
            // A pixel line can't exceed the image width
            if ((pixelLine.size() + (256 - readByte)) > this->width)
                return false;

            for (int i = 0; i < (256 - readByte); i++)
                pixelLine.append(D1CelPixel(true, 0));
        } else {
            // Palette indices group
            // A pixel line can't exceed the image width
            if ((pixelLine.size() + readByte) > this->width)
                return false;

            for (int i = 0; i < readByte; i++) {
                o++;
                pixelLine.append(D1CelPixel(false, rawData[o]));
            }
        }

        if (pixelLine.size() == this->width) {
            pixels.insert(0, pixelLine);
            pixelLine.clear();
        }
    }

    if (this->height == 0)
        this->height = pixels.size();

    return true;
}

quint16 D1CelFrame::computeWidthFromHeader(QByteArray &rawFrameData)
{
    // Reading the frame header
    QDataStream in(rawFrameData);
    in.setByteOrder(QDataStream::LittleEndian);

    quint16 celFrameHeader[5];
    quint16 celFrameWidth[4] = { 0, 0, 0, 0 };

    // Read the {CEL FRAME HEADER}
    for (quint16 &header : celFrameHeader)
        in >> header;

    // Read the five 32 pixel-lines block to calculate the image width
    for (int i = 0; i < 4; i++) {
        if (celFrameHeader[i + 1] == 0)
            break;

        quint16 pixelCount = 0;
        for (int j = celFrameHeader[i]; j < celFrameHeader[i + 1]; j++) {
            quint8 readByte = rawFrameData[j];

            if (readByte > 0x7F) {
                pixelCount += (256 - readByte);
            } else {
                pixelCount += readByte;
                j += readByte;
            }
        }

        celFrameWidth[i] = pixelCount / 32;
    }

    // The calculated width has to be the identical for each 32 pixel-line block
    // If it's not the case, 0 is returned
    for (int i = 0; i < 3; i++) {
        if (celFrameWidth[i + 1] != 0 && celFrameWidth[i] != celFrameWidth[i + 1])
            return 0;
    }

    return celFrameWidth[0];
}

quint16 D1CelFrame::computeWidthFromData(QByteArray &rawFrameData)
{
    quint16 biggestGroupPixelCount = 0;
    quint16 pixelCount = 0;
    quint16 width = 0;
    QList<D1CelPixelGroup *> pixelGroups;

    // Checking the presence of the {CEL FRAME HEADER}
    quint32 frameDataStartOffset = 0;
    if ((quint8)rawFrameData[0] == 0x0A && (quint8)rawFrameData[1] == 0x00)
        frameDataStartOffset = 0x0A;

    // Going through the frame data to find pixel groups
    quint32 globalPixelCount = 0;
    for (int o = frameDataStartOffset; o < rawFrameData.size(); o++) {
        quint8 readByte = rawFrameData[o];

        // Transparent pixels group
        if (readByte > 0x80) {
            pixelCount += (256 - readByte);
            pixelGroups.append(new D1CelPixelGroup(true, pixelCount));
            globalPixelCount += pixelCount;
            if (pixelCount > biggestGroupPixelCount)
                biggestGroupPixelCount = pixelCount;
            pixelCount = 0;
        } else if (readByte == 0x80) {
            pixelCount += 0x80;
        }
        // Palette indices pixel group
        else if (readByte == 0x7F) {
            pixelCount += 0x7F;
            o += 0x7F;
        } else {
            pixelCount += readByte;
            pixelGroups.append(new D1CelPixelGroup(false, pixelCount));
            globalPixelCount += pixelCount;
            if (pixelCount > biggestGroupPixelCount)
                biggestGroupPixelCount = pixelCount;
            pixelCount = 0;
            o += readByte;
        }
    }

    // Going through pixel groups to find pixel-lines wraps
    pixelCount = 0;
    for (int i = 1; i < pixelGroups.size(); i++) {
        pixelCount += pixelGroups[i - 1]->getPixelCount();

        if (pixelGroups[i - 1]->isTransparent() == pixelGroups[i]->isTransparent()) {
            // If width == 0 then it's the first pixel-line wrap and width needs to be set
            // If pixelCount is less than width then the width has to be set to the new value
            if (width == 0 || pixelCount < width)
                width = pixelCount;

            // If the pixelCount of the last group is less than the current pixel group
            // then width is equal to this last pixel group's pixel count.
            // Mostly useful for small frames like the "J" frame in smaltext.cel
            if (i == pixelGroups.size() - 1 && pixelGroups[i]->getPixelCount() < pixelCount)
                width = pixelGroups[i]->getPixelCount();

            pixelCount = 0;
        }

        // If last pixel group is being processed and width is still unknown
        // then set the width to the pixelCount of the last two pixel groups
        if (i == pixelGroups.size() - 1 && width == 0) {
            width = pixelGroups[i - 1]->getPixelCount() + pixelGroups[i]->getPixelCount();
        }
    }

    // If width wasnt found return 0
    if (width == 0) {
        qDeleteAll(pixelGroups);
        return 0;
    }

    // If width is consistent
    if (globalPixelCount % width == 0) {
        qDeleteAll(pixelGroups);
        return width;
    }

    // Try to find  relevant width by adding pixel groups' pixel counts iteratively
    pixelCount = 0;
    for (int i = 0; i < pixelGroups.size(); i++) {
        pixelCount += pixelGroups[i]->getPixelCount();
        if (pixelCount > 1
            && globalPixelCount % pixelCount == 0
            && pixelCount >= biggestGroupPixelCount) {
            qDeleteAll(pixelGroups);
            return pixelCount;
        }
    }

    qDeleteAll(pixelGroups);
    // If still no width found return 0
    return 0;
}
