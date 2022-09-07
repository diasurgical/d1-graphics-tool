#include "d1celtilesetframe.h"

D1CelTilesetFrame::D1CelTilesetFrame(D1CEL_FRAME_TYPE type)
    : frameType(type)
{
    this->width = 32;
    this->height = 32;
}

bool D1CelTilesetFrame::load(QByteArray rawData)
{
    if (rawData.size() == 0)
        return false;

    switch (this->frameType) {
    case D1CEL_FRAME_TYPE::Square:
        this->LoadSquare(rawData);
        break;
    case D1CEL_FRAME_TYPE::TransparentSquare:
        this->LoadTransparentSquare(rawData);
        break;
    case D1CEL_FRAME_TYPE::LeftTriangle:
        this->LoadLeftTriangle(rawData);
        break;
    case D1CEL_FRAME_TYPE::RightTriangle:
        this->LoadRightTriangle(rawData);
        break;
    case D1CEL_FRAME_TYPE::LeftTrapezoid:
        this->LoadLeftTrapezoid(rawData);
        break;
    case D1CEL_FRAME_TYPE::RightTrapezoid:
        this->LoadRightTrapezoid(rawData);
        break;
    }

    return true;
}

void D1CelTilesetFrame::LoadSquare(QByteArray rawData)
{
    for (int i = 0; i < this->height; i++) {
        QList<D1CelPixel> pixelLine;
        for (int j = 0; j < this->width; j++) {
            pixelLine.append(D1CelPixel(false, rawData[i * this->width + j]));
        }
        this->pixels.insert(0, pixelLine);
    }
}

void D1CelTilesetFrame::LoadTransparentSquare(QByteArray &rawData)
{
    int offset = 0;
    for (int i = 0; i < this->height; i++) {
        QList<D1CelPixel> pixelLine;
        int width = this->width;
        while (width > 0) {
            qint8 readByte = rawData[offset++];
            if (readByte > 0) {
                for (int j = 0; j < readByte; j++) {
                    pixelLine.append(D1CelPixel(false, rawData[offset++]));
                }
            } else {
                readByte = -readByte;
                for (int j = 0; j < readByte; j++)
                    pixelLine.append(D1CelPixel(true, 0));
            }
            width -= readByte;
        }
        this->pixels.insert(0, pixelLine);
    }
}

void D1CelTilesetFrame::LoadLeftTriangle(QByteArray &rawData)
{
    int offset = 0;
    for (int i = 1; i <= this->height / 2; i++) {
        offset += 2 * (i % 2);
        QList<D1CelPixel> pixelLine;
        for (int j = 0; j < this->width - 2 * i; j++) {
            pixelLine.append(D1CelPixel(true, 0));
        }
        for (int j = 0; j < 2 * i; j++) {
            pixelLine.append(D1CelPixel(false, rawData[offset++]));
        }
        this->pixels.insert(0, pixelLine);
    }
    for (int i = 1; i <= this->height / 2; i++) {
        offset += 2 * (i % 2);
        QList<D1CelPixel> pixelLine;
        for (int j = 0; j < 2 * i; j++) {
            pixelLine.append(D1CelPixel(true, 0));
        }
        for (int j = 0; j < this->width - 2 * i; j++) {
            pixelLine.append(D1CelPixel(false, rawData[offset++]));
        }
        this->pixels.insert(0, pixelLine);
    }
}

void D1CelTilesetFrame::LoadRightTriangle(QByteArray &rawData)
{
    int offset = 0;
    for (int i = 1; i <= this->height / 2; i++) {
        QList<D1CelPixel> pixelLine;
        for (int j = 0; j < 2 * i; j++) {
            pixelLine.append(D1CelPixel(false, rawData[offset++]));
        }
        for (int j = 0; j < this->width - 2 * i; j++) {
            pixelLine.append(D1CelPixel(true, 0));
        }
        offset += 2 * (i % 2);
        this->pixels.insert(0, pixelLine);
    }
    for (int i = 1; i <= this->height / 2; i++) {
        QList<D1CelPixel> pixelLine;
        for (int j = 0; j < this->width - 2 * i; j++) {
            pixelLine.append(D1CelPixel(false, rawData[offset++]));
        }
        for (int j = 0; j < 2 * i; j++) {
            pixelLine.append(D1CelPixel(true, 0));
        }
        offset += 2 * (i % 2);
        this->pixels.insert(0, pixelLine);
    }
}

void D1CelTilesetFrame::LoadLeftTrapezoid(QByteArray &rawData)
{
    int offset = 0;
    for (int i = 1; i <= this->height / 2; i++) {
        offset += 2 * (i % 2);
        QList<D1CelPixel> pixelLine;
        for (int j = 0; j < this->width - 2 * i; j++) {
            pixelLine.append(D1CelPixel(true, 0));
        }
        for (int j = 0; j < 2 * i; j++) {
            pixelLine.append(D1CelPixel(false, rawData[offset++]));
        }
        this->pixels.insert(0, pixelLine);
    }
    for (int i = 1; i <= this->height / 2; i++) {
        QList<D1CelPixel> pixelLine;
        for (int j = 0; j < this->width; j++) {
            pixelLine.append(D1CelPixel(false, rawData[offset++]));
        }
        this->pixels.insert(0, pixelLine);
    }
}

void D1CelTilesetFrame::LoadRightTrapezoid(QByteArray &rawData)
{
    int offset = 0;
    for (int i = 1; i <= this->height / 2; i++) {
        QList<D1CelPixel> pixelLine;
        for (int j = 0; j < 2 * i; j++) {
            pixelLine.append(D1CelPixel(false, rawData[offset++]));
        }
        for (int j = 0; j < this->width - 2 * i; j++) {
            pixelLine.append(D1CelPixel(true, 0));
        }
        offset += 2 * (i % 2);
        this->pixels.insert(0, pixelLine);
    }
    for (int i = 1; i <= this->height / 2; i++) {
        QList<D1CelPixel> pixelLine;
        for (int j = 0; j < this->width; j++) {
            pixelLine.append(D1CelPixel(false, rawData[offset++]));
        }
        this->pixels.insert(0, pixelLine);
    }
}
