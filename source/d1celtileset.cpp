#include "d1celtileset.h"
#include "d1celtilesetframe.h"

#include <memory>

#include <QMessageBox>

D1CelTileset::D1CelTileset(D1Min *min)
    : min(min)
{
    this->type = D1CEL_TYPE::V1_LEVEL;
}

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

bool D1CelTileset::load(QString filePath, OpenAsParam *params)
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

    this->groupFrameIndices.clear();
    this->frameOffsets.clear();

    if (fileBuffer.size() != fileSizeDword)
        return false;

    // Going through all frames of the CEL
    for (unsigned i = 1; i <= firstDword; i++) {
        fileBuffer.seek(i * 4);
        quint32 celFrameStartOffset;
        in >> celFrameStartOffset;
        quint32 celFrameEndOffset;
        in >> celFrameEndOffset;

        this->frameOffsets.append(qMakePair(celFrameStartOffset, celFrameEndOffset));
    }

    if (this->frameOffsets.empty())
        return false;

    // CEL FRAMES OFFSETS CALCULATION

    this->frameCount = this->frameOffsets.size();

    // BUILDING {CEL FRAMES}

    qDeleteAll(this->frames);
    this->frames.clear();
    for (const auto &offset : this->frameOffsets) {
        fileBuffer.seek(offset.first);
        QByteArray celFrameRawData = fileBuffer.read(offset.second - offset.first);
        D1CEL_FRAME_TYPE frameType = this->min->getFrameType(this->frames.count() + 1);
        if (frameType == D1CEL_FRAME_TYPE::Unknown) {
            qDebug() << "Unknown frame type for frame " << this->frames.count() + 1;
            frameType = guessFrameType(celFrameRawData);
        }
        std::unique_ptr<D1CelFrameBase> frame { new D1CelTilesetFrame(frameType) };
        frame->load(celFrameRawData, params);
        this->frames.append(frame.release());
    }
    this->celFilePath = filePath;
    return true;
}

bool D1CelTileset::writeFileData(QFile &outFile)
{
    const int numFrames = this->getFrameCount();

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

        pBuf = ((D1CelTilesetFrame *)this->getFrame(ii))->writeFrameData(pBuf);
    }

    *(quint32 *)&buf[(numFrames + 1) * sizeof(quint32)] = SwapLE32(pBuf - buf);

    // write to file
    QDataStream out(&outFile);
    out.writeRawData((char *)buf, pBuf - buf);

    return true;
}

bool D1CelTileset::save(SaveAsParam *params)
{
    QString filePath = this->getFilePath();
    if (params != nullptr && !params->celFilePath.isEmpty()) {
        filePath = params->celFilePath;
        /*if (QFile::exists(filePath)) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(nullptr, "Confirmation", "Are you sure you want to overwrite the (level-)CEL file?", QMessageBox::Yes | QMessageBox::No);
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

    bool result = this->writeFileData(outFile);

    outFile.close();

    if (result) {
        this->celFilePath = filePath; // this->load(filePath);
    }
    return result;
}