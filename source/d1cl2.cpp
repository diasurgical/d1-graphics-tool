#include "d1cl2.h"

#include <memory>

#include <QMessageBox>

quint16 D1Cl2Frame::computeWidthFromHeader(QByteArray &rawFrameData)
{
    QDataStream in(rawFrameData);
    in.setByteOrder(QDataStream::LittleEndian);

    quint16 celFrameHeaderSize;
    in >> celFrameHeaderSize;

    if (celFrameHeaderSize & 1)
        return 0; // invalid header

    // Decode the 32 pixel-lines blocks to calculate the image width
    quint16 celFrameWidth = 0;
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

        quint16 width = pixelCount / 32;
        // The calculated width has to be the identical for each 32 pixel-line block
        // If it's not the case, 0 is returned
        if (celFrameWidth != 0 && celFrameWidth != width)
            return 0;

        celFrameWidth = width;
        lastFrameOffset = nextFrameOffset;
    }

    return celFrameWidth;
}

bool D1Cl2Frame::load(QByteArray rawData, OpenAsParam *params)
{
    if (rawData.size() == 0)
        return false;

    quint32 frameDataStartOffset = 0;

    this->clipped = false;
    quint16 width = 0;
    if (params == nullptr || params->clipped == OPEN_CLIPPING_TYPE::CLIPPED_AUTODETECT) {
        // Assume the presence of the {CEL FRAME HEADER}
        QDataStream in(rawData);
        in.setByteOrder(QDataStream::LittleEndian);
        quint16 offset;
        in >> offset;
        frameDataStartOffset += offset;
        // If header is present, try to compute frame width from frame header
        width = this->computeWidthFromHeader(rawData);
        this->clipped = true;
    } else {
        if (params->clipped == OPEN_CLIPPING_TYPE::CLIPPED_TRUE) {
            QDataStream in(rawData);
            in.setByteOrder(QDataStream::LittleEndian);
            quint16 offset;
            in >> offset;
            frameDataStartOffset += offset;
            // If header is present, try to compute frame width from frame header
            width = this->computeWidthFromHeader(rawData);
            this->clipped = true;
        }
    }
    this->width = (params == nullptr || params->width == 0) ? width : params->width;

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

bool D1Cl2::load(QString filePath, OpenAsParam *params)
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
        } else {
            return false;
        }
    }

    // CL2 FRAMES OFFSETS CALCULATION
    this->groupFrameIndices.clear();
    QList<QPair<quint32, quint32>> frameOffsets;
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
        this->groupFrameIndices.append(qMakePair(0, firstDword - 1));
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

    if (frameOffsets.empty()) {
        return false;
    }

    // BUILDING {CL2 FRAMES}

    qDeleteAll(this->frames);
    this->frames.clear();
    for (const auto &offset : frameOffsets) {
        quint32 cl2FrameSize = offset.second - offset.first;
        fileBuffer.seek(offset.first);

        QByteArray cl2FrameRawData = fileBuffer.read(cl2FrameSize);

        std::unique_ptr<D1CelFrameBase> frame { createFrame() };
        frame->load(cl2FrameRawData, params);
        this->frames.append(frame.release());
    }

    this->celFilePath = filePath;
    return true;
}

static quint8 *writeFrameData(D1CelFrameBase *frame, quint8 *pBuf, int subHeaderSize, bool clipped)
{
    const int RLE_LEN = 4; // number of matching colors to switch from bmp encoding to RLE

    // convert one image to cl2-data
    quint8 *pHeader = pBuf;
    if (clipped) {
        // add CL2 FRAME HEADER
        *(quint16 *)&pBuf[0] = SwapLE16(subHeaderSize); // SUB_HEADER_SIZE
        *(quint32 *)&pBuf[2] = 0;
        *(quint32 *)&pBuf[6] = 0;
        pBuf += subHeaderSize;
    }

    quint8 *pHead = pBuf;
    quint8 col, lastCol;
    quint8 colMatches = 0;
    bool alpha = false;
    bool first = true;
    for (int i = 1; i <= frame->getHeight(); i++) {
        if (clipped && (i % CEL_BLOCK_HEIGHT) == 1 /*&& (i / CEL_BLOCK_HEIGHT) * 2 < SUB_HEADER_SIZE*/) {
            pHead = pBuf;
            *(quint16 *)(&pHeader[(i / CEL_BLOCK_HEIGHT) * 2]) = SwapLE16(pHead - pHeader); // pHead - buf - SUB_HEADER_SIZE;

            colMatches = 0;
            alpha = false;
            // first = true;
        }
        first = true;
        for (int j = 0; j < frame->getWidth(); j++) {
            D1CelPixel pixel = frame->getPixel(j, frame->getHeight() - i);
            if (!pixel.isTransparent()) {
                // add opaque pixel
                col = pixel.getPaletteIndex();
                if (alpha || first || col != lastCol)
                    colMatches = 1;
                else
                    colMatches++;
                if (colMatches < RLE_LEN || (char)*pHead <= -127) {
                    // bmp encoding
                    if (alpha || (char)*pHead <= -65 || first) {
                        pHead = pBuf;
                        pBuf++;
                        colMatches = 1;
                    }
                    *pBuf = col;
                    pBuf++;
                } else {
                    // RLE encoding
                    if (colMatches == RLE_LEN) {
                        memset(pBuf - (RLE_LEN - 1), 0, RLE_LEN - 1);
                        *pHead += RLE_LEN - 1;
                        if (*pHead != 0) {
                            pHead = pBuf - (RLE_LEN - 1);
                        }
                        *pHead = -65 - (RLE_LEN - 1);
                        pBuf = pHead + 1;
                        *pBuf = col;
                        pBuf++;
                    }
                }
                --*pHead;

                lastCol = col;
                alpha = false;
            } else {
                // add transparent pixel
                if (!alpha || (char)*pHead >= 127) {
                    pHead = pBuf;
                    pBuf++;
                }
                ++*pHead;
                alpha = true;
            }
            first = false;
        }
    }
    return pBuf;
}

bool D1Cl2::writeFileData(QFile &outFile, SaveAsParam *params)
{
    const int numFrames = this->frames.count();

    // prepare clipping info
    bool clippedForced = params != nullptr && params->clipped != SAVE_CLIPPING_TYPE::CLIPPED_AUTODETECT;
    QList<bool> clipped;
    for (int n = 0; n < numFrames; n++) {
        QPointer<D1CelFrameBase> frame = this->frames[n];
        clipped.append((clippedForced && params->clipped == SAVE_CLIPPING_TYPE::CLIPPED_TRUE) || (!clippedForced && frame->isClipped()));
    }
    // calculate header size
    bool groupped = false;
    int numGroups = 0;
    int headerSize = 0;
    QList<int> groupSizes;
    if (params == nullptr || params->groupNum == 0) {
        numGroups = this->getGroupCount();
        groupped = numGroups > 1;
        for (int i = 0; i < numGroups; i++) {
            QPair<quint16, quint16> gfi = this->getGroupFrameIndices(i);
            int ni = gfi.second - gfi.first + 1;
            groupSizes.append(ni);
            headerSize += 4 + 4 * (ni + 1);
        }
    } else {
        numGroups = params->groupNum;
        if (numFrames % numGroups != 0) {
            QMessageBox::critical(nullptr, "Error", "Frames can not be split to equal groups.");
            return false;
        }
        groupped = true;
        for (int i = 0; i < numGroups; i++) {
            int ni = numFrames / numGroups;
            groupSizes.append(ni);
            headerSize += 4 + 4 * (ni + 1);
        }
    }
    if (groupped) {
        headerSize += sizeof(quint32) * numGroups;
    }
    // calculate sub header size
    int subHeaderSize = SUB_HEADER_SIZE;
    for (int n = 0; n < numFrames; n++) {
        QPointer<D1CelFrameBase> frame = this->frames[n];
        if (clipped[n]) {
            int hs = (frame->getHeight() - 1) / CEL_BLOCK_HEIGHT;
            hs = (hs + 1) * sizeof(quint16);
            subHeaderSize = std::max(subHeaderSize, hs);
        }
    }
    // estimate data size
    int maxSize = headerSize;
    for (int n = 0; n < numFrames; n++) {
        QPointer<D1CelFrameBase> frame = this->frames[n];
        if (clipped[n]) {
            maxSize += subHeaderSize; // SUB_HEADER_SIZE
        }
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
            quint32 ni = groupSizes[i];
            offset += 4 + 4 * (ni + 1);
        }
    }

    quint8 *pBuf = &buf[headerSize];
    int idx = 0;
    for (int ii = 0; ii < numGroups; ii++) {
        int ni = groupSizes[ii];
        *(quint32 *)&hdr[0] = SwapLE32(ni);
        *(quint32 *)&hdr[4] = SwapLE32(pBuf - hdr);

        for (int n = 0; n < ni; n++, idx++) {
            pBuf = writeFrameData(this->getFrame(idx), pBuf, subHeaderSize, clipped[idx]); // TODO: what if the groups are not continuous?
            *(quint32 *)&hdr[4 + 4 * (n + 1)] = SwapLE32(pBuf - hdr);
        }
        hdr += 4 + 4 * (ni + 1);
    }

    // write to file
    QDataStream out(&outFile);
    out.writeRawData((char *)buf, pBuf - buf);

    return true;
}

bool D1Cl2::save(SaveAsParam *params)
{
    QString filePath = this->getFilePath();
    if (params != nullptr && !params->celFilePath.isEmpty()) {
        filePath = params->celFilePath;
        /*if (QFile::exists(filePath)) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(nullptr, "Confirmation", "Are you sure you want to overwrite the CL2 file?", QMessageBox::Yes | QMessageBox::No);
            if (reply != QMessageBox::Yes) {
                return false;
            }
        }*/
    }

    QFile outFile = QFile(filePath);
    if (!outFile.open(QIODevice::WriteOnly | QFile::Truncate)) {
        QMessageBox::critical(nullptr, "Error", "Failed open file: " + filePath);
        return false;
    }

    bool result = this->writeFileData(outFile, params);

    outFile.close();

    if (result) {
        this->load(filePath);
    }
    return result;
}

D1Cl2Frame *D1Cl2::createFrame()
{
    return new D1Cl2Frame();
}
