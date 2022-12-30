#include "d1celtilesetframe.h"

#include <QMessageBox>

D1CelTilesetFrame::D1CelTilesetFrame(D1CEL_FRAME_TYPE type)
    : frameType(type)
{
    this->width = MICRO_WIDTH;
    this->height = MICRO_HEIGHT;
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

quint8 *D1CelTilesetFrame::writeFrameData(quint8 *pDst)
{
    switch (this->frameType) {
    case D1CEL_FRAME_TYPE::LeftTriangle:
        pDst = WriteLeftTriangle(pDst);
        break;
    case D1CEL_FRAME_TYPE::RightTriangle:
        pDst = WriteRightTriangle(pDst);
        break;
    case D1CEL_FRAME_TYPE::LeftTrapezoid:
        pDst = WriteLeftTrapezoid(pDst);
        break;
    case D1CEL_FRAME_TYPE::RightTrapezoid:
        pDst = WriteRightTrapezoid(pDst);
        break;
    case D1CEL_FRAME_TYPE::Square:
        pDst = WriteSquare(pDst);
        break;
    case D1CEL_FRAME_TYPE::TransparentSquare:
        pDst = WriteTransparentSquare(pDst);
        break;
    default:
        // case D1CEL_FRAME_TYPE::Unknown:
        QMessageBox::critical(nullptr, "Error", "Unknown frame type.");
        break;
    }
    return pDst;
}

quint8 *D1CelTilesetFrame::WriteSquare(quint8 *pDst)
{
    D1CelPixel pixel;
    int x, y;
    // int length = MICRO_WIDTH * MICRO_HEIGHT;

    // add opaque pixels
    for (y = MICRO_HEIGHT - 1; y >= 0; y--) {
        for (x = 0; x < MICRO_WIDTH; ++x) {
            pixel = this->getPixel(x, y);
            if (pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid transparent pixel in a Square frame I.");
                return pDst;
            }
            *pDst = pixel.getPaletteIndex();
            ++pDst;
        }
    }
    return pDst;
}

quint8 *D1CelTilesetFrame::WriteTransparentSquare(quint8 *pDst)
{
    D1CelPixel pixel;
    int x, y;
    // int length = MICRO_WIDTH * MICRO_HEIGHT;
    bool hasColor = false;
    quint8 *pHead = pDst;
    pDst++;
    for (y = MICRO_HEIGHT - 1; y >= 0; y--) {
        bool alpha = false;
        for (x = 0; x < MICRO_WIDTH; x++) {
            pixel = this->getPixel(x, y);
            if (pixel.isTransparent()) {
                // add transparent pixel
                if ((char)(*pHead) > 0) {
                    pHead = pDst;
                    pDst++;
                }
                --*pHead;
                alpha = true;
            } else {
                // add opaque pixel
                if (alpha) {
                    alpha = false;
                    pHead = pDst;
                    pDst++;
                }
                *pDst = pixel.getPaletteIndex();
                pDst++;
                ++*pHead;
                hasColor = true;
            }
        }
        pHead = pDst;
        pDst++;
    }
    // if (!hasColor) {
    //     qDebug() << "Empty transparent frame"; -- TODO: log empty frame?
    // }
    // pHead = (quint8 *)(((qsizetype)pHead + 3) & (~(qsizetype)3));
    return pHead;
}

quint8 *D1CelTilesetFrame::WriteLeftTriangle(quint8 *pDst)
{
    D1CelPixel pixel;
    int i, x, y;
    // int length = MICRO_WIDTH * MICRO_HEIGHT / 2 + MICRO_HEIGHT;

    // memset(pDst, 0, length); -- unnecessary for the game, and the current user did this anyway
    y = MICRO_HEIGHT - 1;
    for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, y--) {
        // check transparent pixels
        for (x = 0; x < i; x++) {
            pixel = this->getPixel(x, y);
            if (!pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid non-transparent pixel in a Left Triangle frame I.");
                return pDst;
            }
        }
        pDst += i & 2;
        // add opaque pixels
        for (x = i; x < MICRO_WIDTH; x++) {
            pixel = this->getPixel(x, y);
            if (pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid transparent pixel in a Left Triangle frame I.");
                return pDst;
            }
            *pDst = pixel.getPaletteIndex();
            ++pDst;
        }
    }

    for (i = 2; i != MICRO_HEIGHT; i += 2, y--) {
        // check transparent pixels
        for (x = 0; x < i; x++) {
            pixel = this->getPixel(x, y);
            if (!pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid non-transparent pixel in a Left Triangle frame II.");
                return pDst;
            }
        }
        pDst += i & 2;
        // add opaque pixels
        for (x = i; x < MICRO_WIDTH; ++x) {
            pixel = this->getPixel(x, y);
            if (pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid transparent pixel in a Left Triangle frame II.");
                return pDst;
            }
            *pDst = pixel.getPaletteIndex();
            ++pDst;
        }
    }
    return pDst;
}

quint8 *D1CelTilesetFrame::WriteRightTriangle(quint8 *pDst)
{
    D1CelPixel pixel;
    int i, x, y;
    // int length = MICRO_WIDTH * MICRO_HEIGHT / 2 + MICRO_HEIGHT;

    // memset(pDst, 0, length); -- unnecessary for the game, and the current user did this anyway
    y = MICRO_HEIGHT - 1;
    for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, y--) {
        // add opaque pixels
        for (x = 0; x < (MICRO_WIDTH - i); x++) {
            pixel = this->getPixel(x, y);
            if (pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid transparent pixel in a Right Triangle frame I.");
                return pDst;
            }
            *pDst = pixel.getPaletteIndex();
            ++pDst;
        }
        pDst += i & 2;
        // check transparent pixels
        for (x = MICRO_WIDTH - i; x < MICRO_WIDTH; x++) {
            pixel = this->getPixel(x, y);
            if (!pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid non-transparent pixel in a Right Triangle frame I.");
                return pDst;
            }
        }
    }

    for (i = 2; i != MICRO_HEIGHT; i += 2, y--) {
        // add opaque pixels
        for (x = 0; x < (MICRO_WIDTH - i); x++) {
            pixel = this->getPixel(x, y);
            if (pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid transparent pixel in a Right Triangle frame II.");
                return pDst;
            }
            *pDst = pixel.getPaletteIndex();
            ++pDst;
        }
        pDst += i & 2;
        // check transparent pixels
        for (x = MICRO_WIDTH - i; x < MICRO_WIDTH; x++) {
            pixel = this->getPixel(x, y);
            if (!pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid non-transparent pixel in a Right Triangle frame II.");
                return pDst;
            }
        }
    }
    return pDst;
}

quint8 *D1CelTilesetFrame::WriteLeftTrapezoid(quint8 *pDst)
{
    D1CelPixel pixel;
    int i, x, y;
    // int length = (MICRO_WIDTH * MICRO_HEIGHT) / 2 + MICRO_HEIGHT * (2 + MICRO_HEIGHT) / 4 + MICRO_HEIGHT / 2;

    // memset(pDst, 0, length); -- unnecessary for the game, and the current user did this anyway
    y = MICRO_HEIGHT - 1;
    for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, y--) {
        // check transparent pixels
        for (x = 0; x < i; x++) {
            pixel = this->getPixel(x, y);
            if (!pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid non-transparent pixel in a Left Trapezoid frame I.");
                return pDst;
            }
        }
        pDst += i & 2;
        // add opaque pixels
        for (x = i; x < MICRO_WIDTH; x++) {
            pixel = this->getPixel(x, y);
            if (pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid transparent pixel in a Left Trapezoid frame I.");
                return pDst;
            }
            *pDst = pixel.getPaletteIndex();
            ++pDst;
        }
    }
    // add opaque pixels
    for (i = MICRO_HEIGHT / 2; i != 0; i--, y--) {
        for (x = 0; x < MICRO_WIDTH; x++) {
            pixel = this->getPixel(x, y);
            if (pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid transparent pixel in a Left Trapezoid frame II.");
                return pDst;
            }
            *pDst = pixel.getPaletteIndex();
            ++pDst;
        }
    }
    return pDst;
}

quint8 *D1CelTilesetFrame::WriteRightTrapezoid(quint8 *pDst)
{
    D1CelPixel pixel;
    int i, x, y;
    // int length = (MICRO_WIDTH * MICRO_HEIGHT) / 2 + MICRO_HEIGHT * (2 + MICRO_HEIGHT) / 4 + MICRO_HEIGHT / 2;

    // memset(pDst, 0, length); -- unnecessary for the game, and the current user did this anyway
    y = MICRO_HEIGHT - 1;
    for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, y--) {
        // add opaque pixels
        for (x = 0; x < (MICRO_WIDTH - i); x++) {
            pixel = this->getPixel(x, y);
            if (pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid transparent pixel in a Right Trapezoid frame I.");
                return pDst;
            }
            *pDst = pixel.getPaletteIndex();
            ++pDst;
        }
        pDst += i & 2;
        // check transparent pixels
        for (x = MICRO_WIDTH - i; x < MICRO_WIDTH; x++) {
            pixel = this->getPixel(x, y);
            if (!pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid non-transparent pixel in a Right Trapezoid frame I.");
                return pDst;
            }
        }
    }
    // add opaque pixels
    for (i = MICRO_HEIGHT / 2; i != 0; i--, y--) {
        for (x = 0; x < MICRO_WIDTH; x++) {
            pixel = this->getPixel(x, y);
            if (pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid transparent pixel in a Right Trapezoid frame II.");
                return pDst;
            }
            *pDst = pixel.getPaletteIndex();
            ++pDst;
        }
    }
    return pDst;
}
