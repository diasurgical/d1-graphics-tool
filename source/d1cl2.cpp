#include "d1cl2.h"

#include <memory>

quint16 D1Cl2Frame::computeWidthFromHeader(QByteArray &rawFrameData)
{
    QDataStream in(rawFrameData);
    in.setByteOrder(QDataStream::LittleEndian);

    // Read the {CEL FRAME HEADER}
    quint16 celFrameHeader[5];
    for (quint16 &header : celFrameHeader)
        in >> header;

    quint16 celFrameWidth = 0;

    // Decode the 32 pixel-lines blocks to calculate the image width
    for (int i = 0; i < 4; i++) {
        quint16 pixelCount = 0;
        if (celFrameHeader[i + 1] == 0)
            break;

        for (int j = celFrameHeader[i]; j < celFrameHeader[i + 1]; j++) {
            quint8 readByte = rawFrameData[j];

            if (readByte > 0x00 && readByte < 0x80) {
                pixelCount += readByte;
            } else if (readByte >= 0x80 && readByte < 0xBF) {
                pixelCount += (0xBF - readByte);
                j++;
            } else if (readByte >= 0xBF) {
                pixelCount += (256 - readByte);
                j += (256 - readByte);
            }
        }

        quint16 width = pixelCount / 32;
        if (celFrameWidth != 0 && celFrameWidth != width)
            return 0;

        celFrameWidth = width;
    }

    return celFrameWidth;
}

bool D1Cl2Frame::load(QByteArray rawData)
{
    if (rawData.size() == 0)
        return false;

    quint32 frameDataStartOffset = 0;

    this->width = 0;
    if ((quint8)rawData[0] == 0x0A && (quint8)rawData[1] == 0x00) {
        frameDataStartOffset += 0x0A;
        this->width = this->computeWidthFromHeader(rawData);
    }

    if (this->width == 0)
        return false;

    // READ {CL2 FRAME DATA}

    QList<D1CelPixel> pixelLine;
    for (int o = frameDataStartOffset; o < rawData.size(); o++) {
        quint8 readByte = rawData[o];

        // Transparent pixels
        if (readByte > 0x00 && readByte < 0x80) {
            for (int i = 0; i < readByte; i++) {
                // Add transparent pixel
                pixelLine.append(D1CelPixel(true, 0));

                if (pixelLine.size() == this->width) {
                    pixels.insert(0, pixelLine);
                    pixelLine.clear();
                }
            }
        }
        // Repeat palette index
        else if (readByte >= 0x80 && readByte < 0xBF) {
            // Go to the palette index offset
            o++;

            for (int i = 0; i < (0xBF - readByte); i++) {
                // Add opaque pixel
                pixelLine.append(D1CelPixel(false, rawData[o]));

                if (pixelLine.size() == this->width) {
                    pixels.insert(0, pixelLine);
                    pixelLine.clear();
                }
            }
        }
        // Palette indices
        else if (readByte >= 0xBF) {
            for (int i = 0; i < (256 - readByte); i++) {
                // Go to the next palette index offset
                o++;
                // Add opaque pixel
                pixelLine.append(D1CelPixel(false, rawData[o]));

                if (pixelLine.size() == this->width) {
                    pixels.insert(0, pixelLine);
                    pixelLine.clear();
                }
            }
        } else if (readByte == 0x00) {
            qDebug("0x00 found");
        }
    }

    if (this->height == 0)
        this->height = pixels.size();

    return true;
}

D1Cl2::D1Cl2()
    : D1CelBase()
{
    this->type = D1CEL_TYPE::V2_MULTIPLE_GROUPS;
}

D1Cl2::D1Cl2(QString path, D1Pal *pal)
    : D1CelBase(pal)
{
    this->type = D1CEL_TYPE::V2_MULTIPLE_GROUPS;
    this->load(path);
}

bool D1Cl2::load(QString cl2FilePath)
{
    // Opening CL2 file with a QBuffer to load it in RAM
    if (!QFile::exists(cl2FilePath))
        return false;

    if (this->file.isOpen())
        this->file.close();

    this->file.setFileName(cl2FilePath);

    if (!this->file.open(QIODevice::ReadOnly))
        return false;

    QByteArray fileData = this->file.readAll();
    QBuffer fileBuffer(&fileData);

    if (!fileBuffer.open(QIODevice::ReadOnly))
        return false;

    // Read CL2 binary data
    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    // CL2 HEADER CHECKS

    quint32 firstDword;
    in >> firstDword;

    // Trying to find file size in CL2 header
    if (fileBuffer.size() < (firstDword * 4 + 4 + 4))
        return false;

    fileBuffer.seek(firstDword * 4 + 4);
    quint32 fileSizeDword;
    in >> fileSizeDword;

    // If the dword is not equal to the file size then
    // check if it's a CL2 with multiple groups
    if (fileBuffer.size() == fileSizeDword) {
        this->type = D1CEL_TYPE::V2_MONO_GROUP;
    } else {
        // Read offset of the last CL2 group header
        fileBuffer.seek(firstDword - 4);
        quint32 lastCl2GroupHeaderOffset;
        in >> lastCl2GroupHeaderOffset;

        // Read the number of frames of the last CL2 group
        if (fileBuffer.size() < lastCl2GroupHeaderOffset)
            return false;

        fileBuffer.seek(lastCl2GroupHeaderOffset);
        quint32 lastCl2GroupFrameCount;
        in >> lastCl2GroupFrameCount;

        // Read the last frame offset corresponding to the file size
        if (fileBuffer.size()
            < lastCl2GroupHeaderOffset + lastCl2GroupFrameCount * 4 + 4 + 4)
            return false;

        fileBuffer.seek(lastCl2GroupHeaderOffset + lastCl2GroupFrameCount * 4 + 4);
        in >> fileSizeDword;
        // The offset is from the beginning of the last group header
        // so we need to add the offset of the lasr group header
        // to have an offset from the beginning of the file
        fileSizeDword += lastCl2GroupHeaderOffset;

        if (fileBuffer.size() == fileSizeDword) {
            this->type = D1CEL_TYPE::V2_MULTIPLE_GROUPS;
            this->groupCount = firstDword / 4;
        } else
            return false;
    }

    // CL2 FRAMES OFFSETS CALCULATION

    this->frameOffsets.clear();
    if (this->type == D1CEL_TYPE::V2_MULTIPLE_GROUPS) {
        // Going through all groups
        for (unsigned i = 0; i * 4 < firstDword; i++) {
            fileBuffer.seek(i * 4);
            quint32 cl2GroupOffset;
            in >> cl2GroupOffset;

            fileBuffer.seek(cl2GroupOffset);
            quint32 cl2GroupFrameCount;
            in >> cl2GroupFrameCount;

            this->groupFrameIndices.append(
                qMakePair(this->frameOffsets.size(),
                    this->frameOffsets.size() + cl2GroupFrameCount - 1));

            // Going through all frames of the group
            for (unsigned j = 1; j <= cl2GroupFrameCount; j++) {
                fileBuffer.seek(cl2GroupOffset + j * 4);
                quint32 cl2FrameStartOffset;
                in >> cl2FrameStartOffset;
                quint32 cl2FrameEndOffset;
                in >> cl2FrameEndOffset;

                this->frameOffsets.append(
                    qMakePair(cl2GroupOffset + cl2FrameStartOffset,
                        cl2GroupOffset + cl2FrameEndOffset));
            }
        }
    } else {
        // Going through all frames of the only group
        for (unsigned i = 1; i <= firstDword; i++) {
            fileBuffer.seek(i * 4);
            quint32 cl2FrameStartOffset;
            in >> cl2FrameStartOffset;
            quint32 cl2FrameEndOffset;
            in >> cl2FrameEndOffset;

            this->frameOffsets.append(
                qMakePair(cl2FrameStartOffset, cl2FrameEndOffset));
        }
    }

    if (!this->frameOffsets.empty())
        this->frameCount = this->frameOffsets.size();
    else
        return false;

    // BUILDING {CL2 FRAMES}

    qDeleteAll(this->frames);
    this->frames.clear();
    for (const auto &offset : this->frameOffsets) {
        quint32 cl2FrameSize = offset.second - offset.first;
        fileBuffer.seek(offset.first);

        QByteArray cl2FrameRawData = fileBuffer.read(cl2FrameSize);

        std::unique_ptr<D1CelFrameBase> frame { createFrame() };
        frame->load(cl2FrameRawData);
        this->frames.append(frame.release());
    }

    return true;
}

D1Cl2Frame *D1Cl2::createFrame()
{
    return new D1Cl2Frame;
}
