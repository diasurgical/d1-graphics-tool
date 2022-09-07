#include "d1celtileset.h"
#include "d1celtilesetframe.h"

#include <memory>

D1CelTileset::D1CelTileset(D1Min *min)
    : min(min)
{
    this->type = D1CEL_TYPE::V1_LEVEL;
}

bool D1CelTileset::load(QString celFilePath)
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
        std::unique_ptr<D1CelFrameBase> frame { new D1CelTilesetFrame(frameType) };
        frame->load(celFrameRawData);
        this->frames.append(frame.release());
    }

    return true;
}
