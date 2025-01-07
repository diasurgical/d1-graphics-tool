#include "pcx.h"
#include "d1gfx.h"
#include <QFile>
#include <QBuffer>
#include <QByteArray>
#include <QDataStream>

bool Pcx::load(D1Gfx &gfx, QString filePath, const OpenAsParam &params)
{
    if (!QFile::exists(filePath))
        return false;

    QFile file = QFile(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray fileData = file.readAll();
    QBuffer fileBuffer(&fileData);

    if (!fileBuffer.open(QIODevice::ReadOnly))
        return false;

    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::BigEndian);

    PCXHeader header;

    char* headerBuf = reinterpret_cast<char*>(&header);

    D1GfxFrame frame;

    in.readRawData(headerBuf, PcxHeaderSize);

    frame.width = header.Xmax;
    frame.height = header.Ymax;

    gfx.gfxFilePath = filePath;

    constexpr uint16_t PaletteSize = 768;
    const uint32_t imgDataSize = fileData.size() - PcxHeaderSize - PaletteSize - 1;
    unsigned char imgData[imgDataSize];
    in.readRawData(reinterpret_cast<char *>(imgData), imgDataSize);

    QList<D1GfxPixel> pixelLine;
    for (int i = 0; i < imgDataSize; i++) {

        if (pixelLine.size() == header.BytesPerLine) {
            frame.pixels.append(pixelLine);
            pixelLine.clear();
        }

        constexpr uint8_t PcxMaxSinglePixel = 0xBF;

        const uint8_t byte = imgData[i];
        if (byte <= PcxMaxSinglePixel) {
            pixelLine.append(D1GfxPixel::colorPixel(byte));
            continue;
        }

        constexpr uint8_t PcxRunLengthMask = 0x3F;
        const uint8_t runLength = (byte & PcxRunLengthMask);
        for (unsigned int repeatedByte = 0; repeatedByte < runLength; repeatedByte++)
        {
            if (pixelLine.size() == header.BytesPerLine) {
                frame.pixels.append(pixelLine);
                pixelLine.clear();
            }
            pixelLine.append(D1GfxPixel::colorPixel(imgData[i+1]));
        }
        i++;
    }

    gfx.frames.append(frame);

    // TBD how to actually handle loading palette, since it is being
    // from the image directly

//    unsigned char paletteBuf[768] = { 0 };
//    in.skipRawData(1);
//    in.readRawData(reinterpret_cast<char *>(paletteBuf), 768);
//    QFile pcxFile("pcxtest.pal");
//    if (!pcxFile.open(QIODevice::WriteOnly)) {
//    }
//
//    QByteArray byteArray(reinterpret_cast<const char*>(paletteBuf), 768);
//    pcxFile.write(byteArray);
//    pcxFile.close();

    return true;
}