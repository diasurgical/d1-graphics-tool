#include "d1celtilesetframe.h"

D1CelTilesetFrame::D1CelTilesetFrame(D1CEL_FRAME_TYPE type)
    : frameType(type)
{
    this->width = 32;
    this->height = 32;
}

bool D1CelTilesetFrame::load(QByteArray rawData, OpenAsParam *params)
{
    (void)params; // unused

    if (rawData.size() == 0)
        return false;

    this->clipped = false;
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

void D1CelTilesetFrame::LoadSquare(QByteArray &rawData)
{
    int offset = 0;
    for (int i = 0; i < this->height; i++) {
        QList<D1CelPixel> pixelLine;
        for (int j = 0; j < this->width; j++) {
            pixelLine.append(D1CelPixel(false, rawData[offset++]));
        }
        this->pixels.insert(0, pixelLine);
    }
}

void D1CelTilesetFrame::LoadTransparentSquare(QByteArray &rawData)
{
    int offset = 0;
    for (int i = 0; i < this->height; i++) {
        QList<D1CelPixel> pixelLine;
        int width = 0;
        for (int j = 0; j < this->width; j += width) {
            qint8 readByte = rawData[offset++];
            width = std::abs(readByte);
            bool isTransparent = readByte < 0;
            for (int j = 0; j < width; j++) {
                pixelLine.append(D1CelPixel(isTransparent, isTransparent ? 0 : rawData[offset++]));
            }
        }
        this->pixels.insert(0, pixelLine);
    }
}

void D1CelTilesetFrame::LoadBottomLeftTriangle(QByteArray &rawData)
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
}

void D1CelTilesetFrame::LoadBottomRightTriangle(QByteArray &rawData)
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
}

void D1CelTilesetFrame::LoadLeftTriangle(QByteArray &rawData)
{
    this->LoadBottomLeftTriangle(rawData);
    int offset = 288;
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
    this->LoadBottomRightTriangle(rawData);
    int offset = 288;
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

void D1CelTilesetFrame::LoadTopHalfSquare(QByteArray &rawData)
{
    int offset = 288;
    for (int i = 1; i <= this->height / 2; i++) {
        QList<D1CelPixel> pixelLine;
        for (int j = 0; j < this->width; j++) {
            pixelLine.append(D1CelPixel(false, rawData[offset++]));
        }
        this->pixels.insert(0, pixelLine);
    }
}

void D1CelTilesetFrame::LoadLeftTrapezoid(QByteArray &rawData)
{
    this->LoadBottomLeftTriangle(rawData);
    this->LoadTopHalfSquare(rawData);
}

void D1CelTilesetFrame::LoadRightTrapezoid(QByteArray &rawData)
{
    this->LoadBottomRightTriangle(rawData);
    this->LoadTopHalfSquare(rawData);
}
