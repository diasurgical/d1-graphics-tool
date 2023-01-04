#include "d1celtilesetframe.h"

#include <QMessageBox>

#include "d1gfx.h"

bool D1CelTilesetFrame::load(D1GfxFrame &frame, D1CEL_FRAME_TYPE type, QByteArray rawData, const OpenAsParam &params)
{
    (void)params; // unused

    if (rawData.size() == 0)
        return false;

    frame.width = MICRO_WIDTH;
    frame.height = MICRO_HEIGHT;
    frame.frameType = type;
    frame.clipped = false;
    switch (type) {
    case D1CEL_FRAME_TYPE::Square:
        D1CelTilesetFrame::LoadSquare(frame, rawData);
        break;
    case D1CEL_FRAME_TYPE::TransparentSquare:
        D1CelTilesetFrame::LoadTransparentSquare(frame, rawData);
        break;
    case D1CEL_FRAME_TYPE::LeftTriangle:
        D1CelTilesetFrame::LoadLeftTriangle(frame, rawData);
        break;
    case D1CEL_FRAME_TYPE::RightTriangle:
        D1CelTilesetFrame::LoadRightTriangle(frame, rawData);
        break;
    case D1CEL_FRAME_TYPE::LeftTrapezoid:
        D1CelTilesetFrame::LoadLeftTrapezoid(frame, rawData);
        break;
    case D1CEL_FRAME_TYPE::RightTrapezoid:
        D1CelTilesetFrame::LoadRightTrapezoid(frame, rawData);
        break;
    }

    return true;
}

void D1CelTilesetFrame::LoadSquare(D1GfxFrame &frame, QByteArray &rawData)
{
    int offset = 0;
    for (int i = 0; i < frame.height; i++) {
        QList<D1GfxPixel> pixelLine;
        for (int j = 0; j < frame.width; j++) {
            pixelLine.append(D1GfxPixel(false, rawData[offset++]));
        }
        frame.pixels.insert(0, pixelLine);
    }
}

void D1CelTilesetFrame::LoadTransparentSquare(D1GfxFrame &frame, QByteArray &rawData)
{
    int offset = 0;
    for (int i = 0; i < frame.height; i++) {
        QList<D1GfxPixel> pixelLine;
        int width = 0;
        for (int j = 0; j < frame.width; j += width) {
            qint8 readByte = rawData[offset++];
            width = std::abs(readByte);
            bool isTransparent = readByte < 0;
            for (int j = 0; j < width; j++) {
                pixelLine.append(D1GfxPixel(isTransparent, isTransparent ? 0 : rawData[offset++]));
            }
        }
        frame.pixels.insert(0, pixelLine);
    }
}

void D1CelTilesetFrame::LoadBottomLeftTriangle(D1GfxFrame &frame, QByteArray &rawData)
{
    int offset = 0;
    for (int i = 1; i <= frame.height / 2; i++) {
        offset += 2 * (i % 2);
        QList<D1GfxPixel> pixelLine;
        for (int j = 0; j < frame.width - 2 * i; j++) {
            pixelLine.append(D1GfxPixel(true, 0));
        }
        for (int j = 0; j < 2 * i; j++) {
            pixelLine.append(D1GfxPixel(false, rawData[offset++]));
        }
        frame.pixels.insert(0, pixelLine);
    }
}

void D1CelTilesetFrame::LoadBottomRightTriangle(D1GfxFrame &frame, QByteArray &rawData)
{
    int offset = 0;
    for (int i = 1; i <= frame.height / 2; i++) {
        QList<D1GfxPixel> pixelLine;
        for (int j = 0; j < 2 * i; j++) {
            pixelLine.append(D1GfxPixel(false, rawData[offset++]));
        }
        for (int j = 0; j < frame.width - 2 * i; j++) {
            pixelLine.append(D1GfxPixel(true, 0));
        }
        offset += 2 * (i % 2);
        frame.pixels.insert(0, pixelLine);
    }
}

void D1CelTilesetFrame::LoadLeftTriangle(D1GfxFrame &frame, QByteArray &rawData)
{
    D1CelTilesetFrame::LoadBottomLeftTriangle(frame, rawData);
    int offset = 288;
    for (int i = 1; i <= frame.height / 2; i++) {
        offset += 2 * (i % 2);
        QList<D1GfxPixel> pixelLine;
        for (int j = 0; j < 2 * i; j++) {
            pixelLine.append(D1GfxPixel(true, 0));
        }
        for (int j = 0; j < frame.width - 2 * i; j++) {
            pixelLine.append(D1GfxPixel(false, rawData[offset++]));
        }
        frame.pixels.insert(0, pixelLine);
    }
}

void D1CelTilesetFrame::LoadRightTriangle(D1GfxFrame &frame, QByteArray &rawData)
{
    D1CelTilesetFrame::LoadBottomRightTriangle(frame, rawData);
    int offset = 288;
    for (int i = 1; i <= frame.height / 2; i++) {
        QList<D1GfxPixel> pixelLine;
        for (int j = 0; j < frame.width - 2 * i; j++) {
            pixelLine.append(D1GfxPixel(false, rawData[offset++]));
        }
        for (int j = 0; j < 2 * i; j++) {
            pixelLine.append(D1GfxPixel(true, 0));
        }
        offset += 2 * (i % 2);
        frame.pixels.insert(0, pixelLine);
    }
}

void D1CelTilesetFrame::LoadTopHalfSquare(D1GfxFrame &frame, QByteArray &rawData)
{
    int offset = 288;
    for (int i = 1; i <= frame.height / 2; i++) {
        QList<D1GfxPixel> pixelLine;
        for (int j = 0; j < frame.width; j++) {
            pixelLine.append(D1GfxPixel(false, rawData[offset++]));
        }
        frame.pixels.insert(0, pixelLine);
    }
}

void D1CelTilesetFrame::LoadLeftTrapezoid(D1GfxFrame &frame, QByteArray &rawData)
{
    D1CelTilesetFrame::LoadBottomLeftTriangle(frame, rawData);
    D1CelTilesetFrame::LoadTopHalfSquare(frame, rawData);
}

void D1CelTilesetFrame::LoadRightTrapezoid(D1GfxFrame &frame, QByteArray &rawData)
{
    D1CelTilesetFrame::LoadBottomRightTriangle(frame, rawData);
    D1CelTilesetFrame::LoadTopHalfSquare(frame, rawData);
}

quint8 *D1CelTilesetFrame::writeFrameData(D1GfxFrame &frame, quint8 *pDst)
{
    switch (frame.frameType) {
    case D1CEL_FRAME_TYPE::LeftTriangle:
        pDst = D1CelTilesetFrame::WriteLeftTriangle(frame, pDst);
        break;
    case D1CEL_FRAME_TYPE::RightTriangle:
        pDst = D1CelTilesetFrame::WriteRightTriangle(frame, pDst);
        break;
    case D1CEL_FRAME_TYPE::LeftTrapezoid:
        pDst = D1CelTilesetFrame::WriteLeftTrapezoid(frame, pDst);
        break;
    case D1CEL_FRAME_TYPE::RightTrapezoid:
        pDst = D1CelTilesetFrame::WriteRightTrapezoid(frame, pDst);
        break;
    case D1CEL_FRAME_TYPE::Square:
        pDst = D1CelTilesetFrame::WriteSquare(frame, pDst);
        break;
    case D1CEL_FRAME_TYPE::TransparentSquare:
        pDst = D1CelTilesetFrame::WriteTransparentSquare(frame, pDst);
        break;
    default:
        // case D1CEL_FRAME_TYPE::Unknown:
        QMessageBox::critical(nullptr, "Error", "Unknown frame type.");
        break;
    }
    return pDst;
}

quint8 *D1CelTilesetFrame::WriteSquare(D1GfxFrame &frame, quint8 *pDst)
{
    D1GfxPixel pixel;
    int x, y;
    // int length = MICRO_WIDTH * MICRO_HEIGHT;

    // add opaque pixels
    for (y = MICRO_HEIGHT - 1; y >= 0; y--) {
        for (x = 0; x < MICRO_WIDTH; ++x) {
            pixel = frame.getPixel(x, y);
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

quint8 *D1CelTilesetFrame::WriteTransparentSquare(D1GfxFrame &frame, quint8 *pDst)
{
    D1GfxPixel pixel;
    int x, y;
    // int length = MICRO_WIDTH * MICRO_HEIGHT;
    bool hasColor = false;
    quint8 *pHead = pDst;
    pDst++;
    for (y = MICRO_HEIGHT - 1; y >= 0; y--) {
        bool alpha = false;
        for (x = 0; x < MICRO_WIDTH; x++) {
            pixel = frame.getPixel(x, y);
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

quint8 *D1CelTilesetFrame::WriteLeftTriangle(D1GfxFrame &frame, quint8 *pDst)
{
    D1GfxPixel pixel;
    int i, x, y;
    // int length = MICRO_WIDTH * MICRO_HEIGHT / 2 + MICRO_HEIGHT;

    // memset(pDst, 0, length); -- unnecessary for the game, and the current user did this anyway
    y = MICRO_HEIGHT - 1;
    for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, y--) {
        // check transparent pixels
        for (x = 0; x < i; x++) {
            pixel = frame.getPixel(x, y);
            if (!pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid non-transparent pixel in a Left Triangle frame I.");
                return pDst;
            }
        }
        pDst += i & 2;
        // add opaque pixels
        for (x = i; x < MICRO_WIDTH; x++) {
            pixel = frame.getPixel(x, y);
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
            pixel = frame.getPixel(x, y);
            if (!pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid non-transparent pixel in a Left Triangle frame II.");
                return pDst;
            }
        }
        pDst += i & 2;
        // add opaque pixels
        for (x = i; x < MICRO_WIDTH; ++x) {
            pixel = frame.getPixel(x, y);
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

quint8 *D1CelTilesetFrame::WriteRightTriangle(D1GfxFrame &frame, quint8 *pDst)
{
    D1GfxPixel pixel;
    int i, x, y;
    // int length = MICRO_WIDTH * MICRO_HEIGHT / 2 + MICRO_HEIGHT;

    // memset(pDst, 0, length); -- unnecessary for the game, and the current user did this anyway
    y = MICRO_HEIGHT - 1;
    for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, y--) {
        // add opaque pixels
        for (x = 0; x < (MICRO_WIDTH - i); x++) {
            pixel = frame.getPixel(x, y);
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
            pixel = frame.getPixel(x, y);
            if (!pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid non-transparent pixel in a Right Triangle frame I.");
                return pDst;
            }
        }
    }

    for (i = 2; i != MICRO_HEIGHT; i += 2, y--) {
        // add opaque pixels
        for (x = 0; x < (MICRO_WIDTH - i); x++) {
            pixel = frame.getPixel(x, y);
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
            pixel = frame.getPixel(x, y);
            if (!pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid non-transparent pixel in a Right Triangle frame II.");
                return pDst;
            }
        }
    }
    return pDst;
}

quint8 *D1CelTilesetFrame::WriteLeftTrapezoid(D1GfxFrame &frame, quint8 *pDst)
{
    D1GfxPixel pixel;
    int i, x, y;
    // int length = (MICRO_WIDTH * MICRO_HEIGHT) / 2 + MICRO_HEIGHT * (2 + MICRO_HEIGHT) / 4 + MICRO_HEIGHT / 2;

    // memset(pDst, 0, length); -- unnecessary for the game, and the current user did this anyway
    y = MICRO_HEIGHT - 1;
    for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, y--) {
        // check transparent pixels
        for (x = 0; x < i; x++) {
            pixel = frame.getPixel(x, y);
            if (!pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid non-transparent pixel in a Left Trapezoid frame I.");
                return pDst;
            }
        }
        pDst += i & 2;
        // add opaque pixels
        for (x = i; x < MICRO_WIDTH; x++) {
            pixel = frame.getPixel(x, y);
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
            pixel = frame.getPixel(x, y);
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

quint8 *D1CelTilesetFrame::WriteRightTrapezoid(D1GfxFrame &frame, quint8 *pDst)
{
    D1GfxPixel pixel;
    int i, x, y;
    // int length = (MICRO_WIDTH * MICRO_HEIGHT) / 2 + MICRO_HEIGHT * (2 + MICRO_HEIGHT) / 4 + MICRO_HEIGHT / 2;

    // memset(pDst, 0, length); -- unnecessary for the game, and the current user did this anyway
    y = MICRO_HEIGHT - 1;
    for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, y--) {
        // add opaque pixels
        for (x = 0; x < (MICRO_WIDTH - i); x++) {
            pixel = frame.getPixel(x, y);
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
            pixel = frame.getPixel(x, y);
            if (!pixel.isTransparent()) {
                QMessageBox::critical(nullptr, "Error", "Invalid non-transparent pixel in a Right Trapezoid frame I.");
                return pDst;
            }
        }
    }
    // add opaque pixels
    for (i = MICRO_HEIGHT / 2; i != 0; i--, y--) {
        for (x = 0; x < MICRO_WIDTH; x++) {
            pixel = frame.getPixel(x, y);
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
