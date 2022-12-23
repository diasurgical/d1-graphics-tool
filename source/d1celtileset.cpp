#include "d1celtileset.h"
#include "d1celtilesetframe.h"

#include <memory>

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

bool D1CelTileset::load(QString celFilePath, OpenAsParam *params)
{
    // Opening CEL file with a QBuffer to load it in RAM
    if (!QFile::exists(celFilePath))
        return false;

    if (this->file.isOpen())
        this->file.close();

    this->file.setFileName(celFilePath);

    if (!this->file.open(QIODevice::ReadOnly))
        return false;

    QByteArray fileData = this->file.readAll();
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

    return true;
}
