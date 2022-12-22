#include "d1cel.h"

#include <memory>

bool D1Cel::load(QString celFilePath, OpenAsParam *params)
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

    if (fileBuffer.size() == fileSizeDword) {
        // Going through all frames of the CEL
        for (unsigned int i = 1; i <= firstDword; i++) {
            fileBuffer.seek(i * 4);
            quint32 celFrameStartOffset;
            in >> celFrameStartOffset;
            quint32 celFrameEndOffset;
            in >> celFrameEndOffset;

            this->frameOffsets.append(qMakePair(celFrameStartOffset, celFrameEndOffset));
        }
        this->type = D1CEL_TYPE::V1_REGULAR;
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

        this->type = D1CEL_TYPE::V1_COMPILATION;
        this->groupCount = firstDword / 4;

        // Going through all CELs
        for (unsigned int i = 0; i * 4 < firstDword; i++) {
            fileBuffer.seek(i * 4);
            quint32 celOffset;
            in >> celOffset;

            fileBuffer.seek(celOffset);
            quint32 celFrameCount;
            in >> celFrameCount;

            this->groupFrameIndices.append(
                qMakePair(this->frameOffsets.size(),
                    this->frameOffsets.size() + celFrameCount - 1));

            // Going through all frames of the CEL
            for (unsigned int j = 1; j <= celFrameCount; j++) {
                quint32 celFrameStartOffset = 0;
                quint32 celFrameEndOffset = 0;

                fileBuffer.seek(celOffset + j * 4);
                in >> celFrameStartOffset;
                in >> celFrameEndOffset;

                this->frameOffsets.append(
                    qMakePair(celOffset + celFrameStartOffset,
                        celOffset + celFrameEndOffset));
            }
        }
    }

    // CEL FRAMES OFFSETS CALCULATION

    if (this->frameOffsets.empty())
        return false;

    this->frameCount = this->frameOffsets.size();

    // BUILDING {CEL FRAMES}

    qDeleteAll(this->frames);
    this->frames.clear();
    for (const auto &offset : this->frameOffsets) {
        fileBuffer.seek(offset.first);
        QByteArray celFrameRawData = fileBuffer.read(offset.second - offset.first);

        std::unique_ptr<D1CelFrameBase> frame { new D1CelFrame };
        frame->load(celFrameRawData, params);
        this->frames.append(frame.release());
    }

    return true;
}
