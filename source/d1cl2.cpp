#include "d1cl2.h"

#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QList>
#include <QMessageBox>

quint16 D1Cl2Frame::computeWidthFromHeader(QByteArray &rawFrameData, bool isClx)
{
    QDataStream in(rawFrameData);
    in.setByteOrder(QDataStream::LittleEndian);

    quint16 celFrameHeaderSize;
    in >> celFrameHeaderSize;

    if (celFrameHeaderSize & 1)
        return 0; // invalid header

    quint16 celFrameWidth = 0;
    if (isClx) {
        in >> celFrameWidth;
        return celFrameWidth;
    }

    // Decode the 32 pixel-lines blocks to calculate the image width
    quint16 lastFrameOffset = celFrameHeaderSize;
    for (int i = 0; i < (celFrameHeaderSize / 2) - 1; i++) {
        quint16 pixelCount = 0;
        quint16 nextFrameOffset;
        in >> nextFrameOffset;
        if (nextFrameOffset == 0)
            break;

        for (int j = lastFrameOffset; j < nextFrameOffset; j++) {
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

        quint16 width = pixelCount / CEL_BLOCK_HEIGHT;
        // The calculated width has to be the identical for each 32 pixel-line block
        // If it's not the case, 0 is returned
        if (celFrameWidth != 0 && celFrameWidth != width)
            return 0;

        celFrameWidth = width;
        lastFrameOffset = nextFrameOffset;
    }

    return celFrameWidth;
}

bool D1Cl2Frame::load(D1GfxFrame &frame, QByteArray rawData, bool isClx, const OpenAsParam &params)
{
    if (rawData.size() == 0)
        return false;

    quint32 frameDataStartOffset = 0;

    quint16 width = 0;
    if (params.clipped != OPEN_CLIPPED_TYPE::FALSE) {
        QDataStream in(rawData);
        in.setByteOrder(QDataStream::LittleEndian);
        quint16 offset;
        in >> offset;
        frameDataStartOffset += offset;
        // If header is present, try to compute frame width from frame header
        width = D1Cl2Frame::computeWidthFromHeader(rawData, isClx);
    }
    frame.width = params.celWidth == 0 ? width : params.celWidth;

    if (frame.width == 0)
        return false;

    // READ {CL2 FRAME DATA}

    QList<D1GfxPixel> pixelLine;
    for (int o = frameDataStartOffset; o < rawData.size(); o++) {
        quint8 readByte = rawData[o];

        // Transparent pixels
        if (readByte > 0x00 && readByte < 0x80) {
            for (int i = 0; i < readByte; i++) {
                // Add transparent pixel
                pixelLine.append(D1GfxPixel::transparentPixel());

                if (pixelLine.size() == frame.width) {
                    frame.pixels.insert(0, pixelLine);
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
                pixelLine.append(D1GfxPixel::colorPixel(rawData[o]));

                if (pixelLine.size() == frame.width) {
                    frame.pixels.insert(0, pixelLine);
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
                pixelLine.append(D1GfxPixel::colorPixel(rawData[o]));

                if (pixelLine.size() == frame.width) {
                    frame.pixels.insert(0, pixelLine);
                    pixelLine.clear();
                }
            }
        } else if (readByte == 0x00) {
            qDebug("0x00 found");
        }
    }

    frame.height = frame.pixels.size();

    return true;
}

bool D1Cl2::load(D1Gfx &gfx, QString filePath, bool isClx, const OpenAsParam &params)
{
    // Opening CL2 file with a QBuffer to load it in RAM
    if (!QFile::exists(filePath))
        return false;

    QFile file = QFile(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray fileData = file.readAll();
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
    bool isMultiGroup = fileBuffer.size() != fileSizeDword;
    if (isMultiGroup) {
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

        if (fileBuffer.size() != fileSizeDword) {
            return false;
        }
    }

    // CL2 FRAMES OFFSETS CALCULATION
    gfx.groupFrameIndices.clear();
    QList<QPair<quint32, quint32>> frameOffsets;

    if (isMultiGroup) {
        // Going through all groups
        for (unsigned i = 0; i * 4 < firstDword; i++) {
            fileBuffer.seek(i * 4);
            quint32 cl2GroupOffset;
            in >> cl2GroupOffset;

            fileBuffer.seek(cl2GroupOffset);
            quint32 cl2GroupFrameCount;
            in >> cl2GroupFrameCount;

            gfx.groupFrameIndices.append(
                qMakePair(frameOffsets.size(),
                    frameOffsets.size() + cl2GroupFrameCount - 1));

            // Going through all frames of the group
            for (unsigned j = 1; j <= cl2GroupFrameCount; j++) {
                fileBuffer.seek(cl2GroupOffset + j * 4);
                quint32 cl2FrameStartOffset;
                in >> cl2FrameStartOffset;
                quint32 cl2FrameEndOffset;
                in >> cl2FrameEndOffset;

                frameOffsets.append(
                    qMakePair(cl2GroupOffset + cl2FrameStartOffset,
                        cl2GroupOffset + cl2FrameEndOffset));
            }
        }
    } else {
        // Going through all frames of the only group
        gfx.groupFrameIndices.append(qMakePair(0, firstDword - 1));
        for (unsigned i = 1; i <= firstDword; i++) {
            fileBuffer.seek(i * 4);
            quint32 cl2FrameStartOffset;
            in >> cl2FrameStartOffset;
            quint32 cl2FrameEndOffset;
            in >> cl2FrameEndOffset;

            frameOffsets.append(
                qMakePair(cl2FrameStartOffset, cl2FrameEndOffset));
        }
    }

    // BUILDING {CL2 FRAMES}

    gfx.frames.clear();
    for (const auto &offset : frameOffsets) {
        quint32 cl2FrameSize = offset.second - offset.first;
        fileBuffer.seek(offset.first);

        QByteArray cl2FrameRawData = fileBuffer.read(cl2FrameSize);

        D1GfxFrame frame;
        if (!D1Cl2Frame::load(frame, cl2FrameRawData, isClx, params)) {
            // TODO: log + add placeholder?
            continue;
        }
        gfx.frames.append(frame);
    }

    gfx.gfxFilePath = filePath;
    return true;
}

namespace {

quint8 *AppendClxFillRun(uint8_t color, unsigned width, quint8 *pBuf)
{
    while (width >= 0x3F) {
        *pBuf = 0x80;
        pBuf++;
        *pBuf = color;
        pBuf++;
        width -= 0x3F;
    }
    if (width == 0)
        return pBuf;
    *pBuf = 0xBF - width;
    pBuf++;
    *pBuf = color;
    pBuf++;
    return pBuf;
}

quint8 *AppendClxPixelsRun(D1GfxFrame *frame, int x, int y, unsigned width, quint8 *pBuf)
{
    while (width >= 0x41) {
        *pBuf = 0xBF;
        pBuf++;
        for (size_t i = 0; i < 0x41; ++i) {
            *pBuf = frame->getPixel(x + i, y).getPaletteIndex();
            pBuf++;
        }
        width -= 0x41;
        x += 0x41;
    }
    if (width == 0)
        return pBuf;
    *pBuf = 256 - width;
    pBuf++;
    for (size_t i = 0; i < width; ++i) {
        *pBuf = frame->getPixel(x + i, y).getPaletteIndex();
        pBuf++;
    }

    return pBuf;
}

quint8 *AppendClxTransparentRun(unsigned width, quint8 *pBuf)
{
    while (width >= 0x7F) {
        *pBuf = 0x7F;
        pBuf++;
        width -= 0x7F;
    }
    if (width == 0)
        return pBuf;
    *pBuf = width;
    pBuf++;
    return pBuf;
}

quint8 *AppendClxPixelsOrFillRun(D1GfxFrame *frame, int x, int y, unsigned length, quint8 *pBuf)
{
    int beginX = x;
    int prevColorX = x;
    unsigned prevColorRunLength = 1;
    uint8_t prevColor = frame->getPixel(x, y).getPaletteIndex();
    x++;
    while (--length > 0) {
        const uint8_t color = frame->getPixel(x, y).getPaletteIndex();
        if (prevColor == color) {
            ++prevColorRunLength;
        } else {
            // A tunable parameter that decides at which minimum length we encode a fill run.
            // 3 appears to be optimal for most of our data (much better than 2, rarely very slightly worse than 4).
            constexpr unsigned MinFillRunLength = 3;
            if (prevColorRunLength >= MinFillRunLength) {
                pBuf = AppendClxPixelsRun(frame, beginX, y, prevColorX - beginX, pBuf);
                pBuf = AppendClxFillRun(prevColor, prevColorRunLength, pBuf);
                beginX = x;
            }
            prevColorX = x;
            prevColorRunLength = 1;
            prevColor = color;
        }
        x++;
    }

    // Here we use 2 instead of `MinFillRunLength` because we know that this run
    // is followed by transparent pixels.
    // Width=2 Fill command takes 2 bytes, while the Pixels command is 3 bytes.
    if (prevColorRunLength >= 2) {
        pBuf = AppendClxPixelsRun(frame, beginX, y, prevColorX - beginX, pBuf);
        pBuf = AppendClxFillRun(prevColor, prevColorRunLength, pBuf);
    } else {
        pBuf = AppendClxPixelsRun(frame, beginX, y, prevColorX - beginX + prevColorRunLength, pBuf);
    }
    return pBuf;
}

} // namespace

static quint8 *writeFrameData(D1GfxFrame *frame, quint8 *pBuf, bool isClx, int subHeaderSize)
{
    // convert one image to cl2-data
    quint16 *pHeader = reinterpret_cast<quint16 *>(pBuf);
    // add CL2 FRAME HEADER
    pHeader[0] = SwapLE16(subHeaderSize); // SUB_HEADER_SIZE
    pHeader[1] = 0; // row  -32
    pHeader[2] = 0; // row  -64
    pHeader[3] = 0; // row  -96
    pHeader[4] = 0; // row -128
    pBuf += subHeaderSize;

    unsigned transparentRunWidth = 0;
    for (int i = 1; i <= frame->getHeight(); i++) {
        if ((i % CEL_BLOCK_HEIGHT) == 1) {
            if (transparentRunWidth != 0) {
                pBuf = AppendClxTransparentRun(transparentRunWidth, pBuf);
                transparentRunWidth = 0;
            }
            pHeader[i / CEL_BLOCK_HEIGHT] = SwapLE16(pBuf - reinterpret_cast<quint8 *>(pHeader)); // buf - SUB_HEADER_SIZE;
        }
        int y = frame->getHeight() - i;
        // Process line:
        unsigned solidRunWidth = 0;
        for (int x = 0; x < frame->getWidth(); x++) {
            D1GfxPixel pixel = frame->getPixel(x, y);
            if (pixel.isTransparent()) {
                if (solidRunWidth != 0) {
                    pBuf = AppendClxPixelsOrFillRun(frame, x - transparentRunWidth - solidRunWidth, y, solidRunWidth, pBuf);
                    solidRunWidth = 0;
                }
                ++transparentRunWidth;
            } else {
                pBuf = AppendClxTransparentRun(transparentRunWidth, pBuf);
                transparentRunWidth = 0;
                ++solidRunWidth;
            }
        }
        if (solidRunWidth != 0) {
            pBuf = AppendClxPixelsOrFillRun(frame, frame->getWidth() - solidRunWidth, y, solidRunWidth, pBuf);
        }
    }
    pBuf = AppendClxTransparentRun(transparentRunWidth, pBuf);
    if (isClx) {
        pHeader[1] = SwapLE16(frame->getWidth());
        pHeader[2] = SwapLE16(frame->getHeight());
    }
    return pBuf;
}

bool D1Cl2::writeFileData(D1Gfx &gfx, QFile &outFile, bool isClx, const QString &gfxPath)
{
    const int numFrames = gfx.frames.count();

    // calculate header size
    int headerSize = 0;
    int numGroups = gfx.getGroupCount();
    bool groupped = numGroups > 1;
    for (int i = 0; i < numGroups; i++) {
        QPair<quint16, quint16> gfi = gfx.getGroupFrameIndices(i);
        int ni = gfi.second - gfi.first + 1;
        headerSize += 4 + 4 * (ni + 1);
    }
    if (groupped) {
        headerSize += sizeof(quint32) * numGroups;
    }

    // calculate sub header size
    int subHeaderSize = SUB_HEADER_SIZE;
    for (int n = 0; n < numFrames; n++) {
        D1GfxFrame *frame = gfx.getFrame(n);
        int hs = (frame->getHeight() - 1) / CEL_BLOCK_HEIGHT;
        hs = (hs + 1) * sizeof(quint16);
        subHeaderSize = std::max(subHeaderSize, hs);
    }
    // estimate data size
    int maxSize = headerSize;
    for (int n = 0; n < numFrames; n++) {
        D1GfxFrame *frame = gfx.getFrame(n);
        maxSize += subHeaderSize; // SUB_HEADER_SIZE
        maxSize += frame->getHeight() * (2 * frame->getWidth());
    }

    QByteArray fileData;
    fileData.append(maxSize, 0);

    quint8 *buf = (quint8 *)fileData.data();
    quint8 *hdr = buf;
    if (groupped) {
        // add optional {CL2 GROUP HEADER}
        int offset = numGroups * 4;
        for (int i = 0; i < numGroups; i++, hdr += 4) {
            *(quint32 *)&hdr[0] = offset;
            QPair<quint16, quint16> gfi = gfx.getGroupFrameIndices(i);
            int ni = gfi.second - gfi.first + 1;
            offset += 4 + 4 * (ni + 1);
        }
    }

    quint8 *pBuf = &buf[headerSize];
    int idx = 0;
    for (int ii = 0; ii < numGroups; ii++) {
        QPair<quint16, quint16> gfi = gfx.getGroupFrameIndices(ii);
        int ni = gfi.second - gfi.first + 1;
        *(quint32 *)&hdr[0] = SwapLE32(ni);
        *(quint32 *)&hdr[4] = SwapLE32(pBuf - hdr);

        for (int n = 0; n < ni; n++, idx++) {
            D1GfxFrame *frame = gfx.getFrame(idx); // TODO: what if the groups are not continuous?
            pBuf = writeFrameData(frame, pBuf, isClx, subHeaderSize);
            *(quint32 *)&hdr[4 + 4 * (n + 1)] = SwapLE32(pBuf - hdr);
        }
        hdr += 4 + 4 * (ni + 1);
    }

    // write to file
    QDataStream out(&outFile);
    out.writeRawData((char *)buf, pBuf - buf);

    return true;
}

bool D1Cl2::save(D1Gfx &gfx, bool isClx, const QString &gfxPath)
{
    QFile outFile = QFile(gfxPath);
    if (!outFile.open(QIODevice::WriteOnly | QFile::Truncate)) {
        QMessageBox::critical(nullptr, "Error", "Failed open file: " + gfxPath);
        return false;
    }

    bool result = D1Cl2::writeFileData(gfx, outFile, isClx, gfxPath);

    if (result) {
        gfx.gfxFilePath = gfxPath;
    }
    return result;
}
