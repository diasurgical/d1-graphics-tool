#include "d1celtileset.h"

#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QMessageBox>

#include "d1celtilesetframe.h"

D1CEL_FRAME_TYPE guessFrameType(QByteArray &rawFrameData)
{
    if (rawFrameData.size() == 544 || rawFrameData.size() == 800) {
        const int leftZeros[32] = {
            0, 1, 8, 9, 24, 25, 48, 49, 80, 81, 120, 121, 168, 169, 224, 225,
            288, 289, 348, 349, 400, 401, 444, 445, 480, 481, 508, 509, 528, 529, 540, 541
        };

        for (int i = 0; i < 32; i++) {
            std::uint8_t byte = rawFrameData[leftZeros[i]];
            if (byte != 0)
                break;
            if (i == 15 && rawFrameData.size() == 800)
                return D1CEL_FRAME_TYPE::LeftTrapezoid;
            if (i == 31 && rawFrameData.size() == 544)
                return D1CEL_FRAME_TYPE::LeftTriangle;
        }

        const int rightZeros[32] = {
            2, 3, 14, 15, 34, 35, 62, 63, 98, 99, 142, 143, 194, 195, 254, 255,
            318, 319, 374, 375, 422, 423, 462, 463, 494, 495, 518, 519, 534, 535, 542, 543
        };

        for (int i = 0; i < 32; i++) {
            std::uint8_t byte = rawFrameData[rightZeros[i]];
            if (byte != 0)
                break;
            if (i == 15 && rawFrameData.size() == 800)
                return D1CEL_FRAME_TYPE::RightTrapezoid;
            if (i == 31 && rawFrameData.size() == 544)
                return D1CEL_FRAME_TYPE::RightTriangle;
        }
    }

    if (rawFrameData.size() == 1024) {
        return D1CEL_FRAME_TYPE::Square;
    }

    return D1CEL_FRAME_TYPE::TransparentSquare;
}

bool D1CelTileset::load(D1Gfx &gfx, std::map<unsigned, D1CEL_FRAME_TYPE> &celFrameTypes, QString filePath, const OpenAsParam &params)
{
    // prepare file data source
    QFile file;
    // done by the caller
    // if (!params.celFilePath.isEmpty()) {
    //    filePath = params.celFilePath;
    // }
    if (!filePath.isEmpty()) {
        file.setFileName(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }
    }

    QByteArray fileData = file.readAll();
    QBuffer fileBuffer(&fileData);

    if (!fileBuffer.open(QIODevice::ReadOnly)) {
        return false;
    }

    // Read CEL binary data
    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    // File size check
    int numFrames = 0;
    auto fileSize = fileBuffer.size();
    if (fileSize != 0) {
        // CEL HEADER CHECKS
        if (fileSize < 4) {
            qDebug() << "Level-cel-file is too small.";
            return false;
        }

        // Read first DWORD
        quint32 readDword;
        in >> readDword;

        numFrames = readDword;

        // Trying to find file size in CEL header
        if (fileSize < (4 + numFrames * 4 + 4)) {
            qDebug() << "Header of the level-cel-file is too small.";
            return false;
        }

        fileBuffer.seek(numFrames * 4 + 4);
        quint32 fileSizeDword;
        in >> fileSizeDword;

        if (fileSize != fileSizeDword) {
            qDebug() << "Invalid level-cel-file header.";
            return false;
        }
    }

    gfx.groupFrameIndices.clear();
    gfx.groupFrameIndices.append(qMakePair(0, numFrames - 1));
    gfx.isTileset_ = true;

    // CEL FRAMES OFFSETS CALCULATION
    QList<QPair<quint32, quint32>> frameOffsets;
    for (int i = 1; i <= numFrames; i++) {
        fileBuffer.seek(i * 4);
        quint32 celFrameStartOffset;
        in >> celFrameStartOffset;
        quint32 celFrameEndOffset;
        in >> celFrameEndOffset;

        frameOffsets.append(qMakePair(celFrameStartOffset, celFrameEndOffset));
    }

    // BUILDING {CEL FRAMES}
    gfx.frames.clear();
    for (int i = 0; i < frameOffsets.count(); i++) {
        const auto &offset = frameOffsets[i];
        fileBuffer.seek(offset.first);
        QByteArray celFrameRawData = fileBuffer.read(offset.second - offset.first);
        D1CEL_FRAME_TYPE frameType;
        auto iter = celFrameTypes.find(i + 1);
        if (iter != celFrameTypes.end()) {
            frameType = iter->second;
        } else {
            qDebug() << "Unknown frame type for frame " << i + 1;
            frameType = guessFrameType(celFrameRawData);
        }
        D1GfxFrame frame;
        if (!D1CelTilesetFrame::load(frame, frameType, celFrameRawData, params)) {
            // TODO: log + add placeholder?
            continue;
        }
        gfx.frames.append(frame);
    }
    gfx.gfxFilePath = filePath;
    return true;
}

bool D1CelTileset::writeFileData(D1Gfx &gfx, QFile &outFile)
{
    const int numFrames = gfx.getFrameCount();

    // calculate header size
    int headerSize = 4 + numFrames * 4 + 4;

    // estimate data size
    int maxSize = headerSize;
    maxSize += MICRO_WIDTH * MICRO_HEIGHT * numFrames;

    QByteArray fileData;
    fileData.append(maxSize, 0);

    quint8 *buf = (quint8 *)fileData.data();
    *(quint32 *)&buf[0] = SwapLE32(numFrames);

    quint8 *pBuf = &buf[headerSize];
    for (int ii = 0; ii < numFrames; ii++) {
        *(quint32 *)&buf[(ii + 1) * sizeof(quint32)] = SwapLE32(pBuf - buf);

        pBuf = D1CelTilesetFrame::writeFrameData(*gfx.getFrame(ii), pBuf);
    }

    *(quint32 *)&buf[(numFrames + 1) * sizeof(quint32)] = SwapLE32(pBuf - buf);

    // write to file
    QDataStream out(&outFile);
    out.writeRawData((char *)buf, pBuf - buf);

    return true;
}

bool D1CelTileset::save(D1Gfx &gfx, const QString &gfxPath)
{
    QFile outFile = QFile(gfxPath);
    if (!outFile.open(QIODevice::WriteOnly | QFile::Truncate)) {
        QMessageBox::critical(nullptr, "Error", "Failed open file: " + gfxPath);
        return false;
    }

    bool success = D1CelTileset::writeFileData(gfx, outFile);

    if (success) {
        gfx.modified = false;
        gfx.gfxFilePath = gfxPath;
    }
    return success;
}
