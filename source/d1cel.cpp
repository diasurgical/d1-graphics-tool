#include "d1cel.h"

#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QList>
#include <QMessageBox>

#include "d1celframe.h"

bool D1Cel::load(D1Gfx &gfx, QString filePath, const OpenAsParam &params)
{
    // Opening CEL file with a QBuffer to load it in RAM
    if (!QFile::exists(filePath))
        return false;

    QFile file = QFile(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray fileData = file.readAll();
    QBuffer fileBuffer(&fileData);

    if (!fileBuffer.open(QIODevice::ReadOnly))
        return false;

    // Read CEL binary data
    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    // CEL HEADER CHECKS

    // Read first DWORD
    quint32 firstDword;
    in >> firstDword;

    // Trying to find file size in CEL header
    if (fileBuffer.size() < (4 + firstDword * 4 + 4))
        return false;

    fileBuffer.seek(firstDword * 4 + 4);
    quint32 fileSizeDword;
    in >> fileSizeDword;

    QList<QPair<quint32, quint32>> frameOffsets;
    if (fileBuffer.size() == fileSizeDword) {
        // Going through all frames of the CEL
        gfx.groupFrameIndices.clear();
        gfx.groupFrameIndices.append(qMakePair(0, firstDword - 1));
        for (unsigned int i = 1; i <= firstDword; i++) {
            fileBuffer.seek(i * 4);
            quint32 celFrameStartOffset;
            in >> celFrameStartOffset;
            quint32 celFrameEndOffset;
            in >> celFrameEndOffset;

            frameOffsets.append(qMakePair(celFrameStartOffset, celFrameEndOffset));
        }
    } else {
        // Read offset of the last CEL of the CEL compilation
        fileBuffer.seek(firstDword - 4);
        quint32 lastCelOffset;
        in >> lastCelOffset;

        // Go to last CEL of the CEL compilation
        if (fileBuffer.size() < (lastCelOffset + 8))
            return false;

        fileBuffer.seek(lastCelOffset);

        // Read last CEL header
        quint32 lastCelFrameCount;
        in >> lastCelFrameCount;

        // Read the last CEL size
        if (fileBuffer.size() < (lastCelOffset + 4 + lastCelFrameCount * 4 + 4))
            return false;

        fileBuffer.seek(lastCelOffset + 4 + lastCelFrameCount * 4);
        quint32 lastCelSize;
        in >> lastCelSize;

        // If the last CEL size plus the last CEL offset is equal to
        // the file size then it's a CEL compilation
        if (fileBuffer.size() != (lastCelOffset + lastCelSize)) {
            return false;
        }

        // Going through all CELs
        gfx.groupFrameIndices.clear();
        for (unsigned int i = 0; i * 4 < firstDword; i++) {
            fileBuffer.seek(i * 4);
            quint32 celOffset;
            in >> celOffset;

            fileBuffer.seek(celOffset);
            quint32 celFrameCount;
            in >> celFrameCount;

            gfx.groupFrameIndices.append(
                qMakePair(frameOffsets.size(),
                    frameOffsets.size() + celFrameCount - 1));

            // Going through all frames of the CEL
            for (unsigned int j = 1; j <= celFrameCount; j++) {
                quint32 celFrameStartOffset = 0;
                quint32 celFrameEndOffset = 0;

                fileBuffer.seek(celOffset + j * 4);
                in >> celFrameStartOffset;
                in >> celFrameEndOffset;

                frameOffsets.append(
                    qMakePair(celOffset + celFrameStartOffset,
                        celOffset + celFrameEndOffset));
            }
        }
    }

    // CEL FRAMES OFFSETS CALCULATION

    // BUILDING {CEL FRAMES}

    if (params.clipped == OPEN_CLIPPED_TYPE::Auto) {
        fileBuffer.seek(frameOffsets[0].first);
        QDataStream header(fileBuffer.read(2));
        header.setByteOrder(QDataStream::LittleEndian);
        quint16 offset;
        header >> offset;
        gfx.setHasHeader(offset == 0x0A);
    } else {
        gfx.setHasHeader(params.clipped == OPEN_CLIPPED_TYPE::Yes);
    }

    gfx.frames.clear();
    for (const auto &offset : frameOffsets) {
        fileBuffer.seek(offset.first);
        QByteArray celFrameRawData = fileBuffer.read(offset.second - offset.first);

        D1GfxFrame frame;
        if (!D1CelFrame::load(frame, celFrameRawData, params)) {
            // TODO: log + add placeholder?
            continue;
        }
        gfx.frames.append(frame);
    }

    gfx.gfxFilePath = filePath;
    return true;
}

static quint8 *writeFrameData(D1GfxFrame *frame, quint8 *pBuf, int subHeaderSize, bool writeHeader)
{
    // add optional {CEL FRAME HEADER}
    quint8 *pHeader = pBuf;
    if (writeHeader) {
        *(quint16 *)&pBuf[0] = SwapLE16(subHeaderSize); // SUB_HEADER_SIZE
        *(quint32 *)&pBuf[2] = 0;
        *(quint32 *)&pBuf[6] = 0;
        pBuf += subHeaderSize;
    }
    // convert to cel
    quint8 *pHead;
    for (int i = 1; i <= frame->getHeight(); i++) {
        pHead = pBuf;
        pBuf++;
        bool alpha = false;
        if (writeHeader && (i % CEL_BLOCK_HEIGHT) == 1 /*&& (i / CEL_BLOCK_HEIGHT) * 2 < SUB_HEADER_SIZE*/) {
            *(quint16 *)(&pHeader[(i / CEL_BLOCK_HEIGHT) * 2]) = SwapLE16(pHead - pHeader); // pHead - buf - SUB_HEADER_SIZE;
        }
        for (int j = 0; j < frame->getWidth(); j++) {
            D1GfxPixel pixel = frame->getPixel(j, frame->getHeight() - i);
            if (!pixel.isTransparent()) {
                // add opaque pixel
                if (alpha || *pHead > 126) {
                    pHead = pBuf;
                    pBuf++;
                }
                ++*pHead;
                *pBuf = pixel.getPaletteIndex();
                pBuf++;
                alpha = false;
            } else {
                // add transparent pixel
                if (j != 0 && (!alpha || (char)*pHead == -128)) {
                    pHead = pBuf;
                    pBuf++;
                }
                --*pHead;
                alpha = true;
            }
        }
    }
    return pBuf;
}

bool D1Cel::writeFileData(D1Gfx &gfx, QFile &outFile)
{
    bool writeHeader = gfx.hasHeader();
    const int numFrames = gfx.frames.count();

    // calculate header size
    int HEADER_SIZE = 4 + 4 + numFrames * 4;
    // calculate sub header size
    int subHeaderSize = SUB_HEADER_SIZE;
    for (int n = 0; n < numFrames; n++) {
        D1GfxFrame *frame = gfx.getFrame(n);
        if (writeHeader) {
            int hs = (frame->getHeight() - 1) / CEL_BLOCK_HEIGHT;
            hs = (hs + 1) * sizeof(quint16);
            subHeaderSize = std::max(subHeaderSize, hs);
        }
    }
    // estimate data size
    int maxSize = HEADER_SIZE;
    for (int n = 0; n < numFrames; n++) {
        D1GfxFrame *frame = gfx.getFrame(n);
        if (writeHeader) {
            maxSize += subHeaderSize; // SUB_HEADER_SIZE
        }
        maxSize += frame->getHeight() * (2 * frame->getWidth());
    }

    QByteArray fileData;
    fileData.append(maxSize, 0);

    quint8 *buf = (quint8 *)fileData.data();
    *(quint32 *)&buf[0] = SwapLE32(numFrames);
    *(quint32 *)&buf[4] = SwapLE32(HEADER_SIZE);
    quint8 *pBuf = &buf[HEADER_SIZE];
    for (int n = 0; n < numFrames; n++) {
        D1GfxFrame *frame = gfx.getFrame(n);
        pBuf = writeFrameData(frame, pBuf, subHeaderSize, writeHeader);
        *(quint32 *)&buf[4 + 4 * (n + 1)] = SwapLE32(pBuf - buf);
    }

    // write to file
    QDataStream out(&outFile);
    out.writeRawData((char *)buf, pBuf - buf);

    return true;
}

bool D1Cel::writeCompFileData(D1Gfx &gfx, QFile &outFile)
{
    bool writeHeader = gfx.hasHeader();
    const int numFrames = gfx.frames.count();

    // calculate header size
    int headerSize = 0;
    int numGroups = gfx.getGroupCount();
    for (int i = 0; i < numGroups; i++) {
        QPair<quint16, quint16> gfi = gfx.getGroupFrameIndices(i);
        int ni = gfi.second - gfi.first + 1;
        headerSize += 4 + 4 * (ni + 1);
    }

    headerSize += sizeof(quint32) * numGroups;

    // calculate sub header size
    int subHeaderSize = SUB_HEADER_SIZE;
    for (int n = 0; n < numFrames; n++) {
        D1GfxFrame *frame = gfx.getFrame(n);
        if (writeHeader) {
            int hs = (frame->getHeight() - 1) / CEL_BLOCK_HEIGHT;
            hs = (hs + 1) * sizeof(quint16);
            subHeaderSize = std::max(subHeaderSize, hs);
        }
    }
    // estimate data size
    int maxSize = headerSize;
    for (int n = 0; n < numFrames; n++) {
        D1GfxFrame *frame = gfx.getFrame(n);
        if (writeHeader) {
            maxSize += subHeaderSize; // SUB_HEADER_SIZE
        }
        maxSize += frame->getHeight() * (2 * frame->getWidth());
    }

    QByteArray fileData;
    fileData.append(maxSize, 0);

    quint8 *buf = (quint8 *)fileData.data();
    quint8 *pBuf = &buf[sizeof(quint32) * numGroups];
    int idx = 0;
    for (int ii = 0; ii < numGroups; ii++) {
        QPair<quint16, quint16> gfi = gfx.getGroupFrameIndices(ii);
        int ni = gfi.second - gfi.first + 1;
        *(quint32 *)&buf[ii * sizeof(quint32)] = SwapLE32(pBuf - buf);

        quint8 *hdr = pBuf;
        *(quint32 *)&hdr[0] = SwapLE32(ni);
        *(quint32 *)&hdr[4] = SwapLE32(4 + 4 * (ni + 1));

        pBuf += 4 + 4 * (ni + 1);
        for (int n = 0; n < ni; n++, idx++) {
            D1GfxFrame *frame = gfx.getFrame(idx); // TODO: what if the groups are not continuous?
            pBuf = writeFrameData(frame, pBuf, subHeaderSize, writeHeader);
            *(quint32 *)&hdr[4 + 4 * (n + 1)] = SwapLE32(pBuf - hdr);
        }
    }
    // write to file
    QDataStream out(&outFile);
    out.writeRawData((char *)buf, pBuf - buf);

    return true;
}

bool D1Cel::save(D1Gfx &gfx, const QString &gfxPath)
{
    QFile outFile = QFile(gfxPath);
    if (!outFile.open(QIODevice::WriteOnly | QFile::Truncate)) {
        QMessageBox::critical(nullptr, "Error", "Failed open file: " + gfxPath);
        return false;
    }

    bool success;
    if (gfx.getGroupCount() > 1) {
        success = D1Cel::writeCompFileData(gfx, outFile);
    } else {
        success = D1Cel::writeFileData(gfx, outFile);
    }

    if (success) {
        gfx.modified = false;
        gfx.gfxFilePath = gfxPath;
    }
    return success;
}
