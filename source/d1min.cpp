#include "d1min.h"

#include <QBuffer>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QPainter>

#include "d1image.h"

bool D1Min::load(QString filePath, D1Gfx *g, D1Sol *sol, std::map<unsigned, D1CEL_FRAME_TYPE> &celFrameTypes, const OpenAsParam &params)
{
    // prepare file data source
    QFile file;
    // done by the caller
    // if (!params.minFilePath.isEmpty()) {
    //    filePath = params.minFilePath;
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

    // calculate subtileWidth/Height
    auto fileSize = file.size();
    int subtileCount = sol->getSubtileCount();
    int width = params.minWidth;
    if (width == 0) {
        width = 2;
    }
    int height = params.minHeight;
    if (height == 0) {
        if (fileSize == 0 || subtileCount == 0) {
            height = 5;
        } else {
            // guess subtileHeight based on the data
            height = fileSize / (subtileCount * width * 2);
        }
    }

    // File size check
    int subtileNumberOfCelFrames = width * height;
    if ((fileSize % (subtileNumberOfCelFrames * 2)) != 0) {
        qDebug() << "Sub-tile width/height does not align with min-file.";
        return false;
    }

    this->gfx = g;
    this->subtileWidth = width;
    this->subtileHeight = height;
    int minSubtileCount = fileSize / (subtileNumberOfCelFrames * 2);
    if (minSubtileCount != subtileCount) {
        qDebug() << "The size of sol-file does not align with min-file";
        // add subtiles to sol if necessary
        while (minSubtileCount > subtileCount) {
            subtileCount++;
            sol->createSubtile();
        }
    }

    // prepare an empty list with zeros
    this->celFrameIndices.clear();
    for (int i = 0; i < subtileCount; i++) {
        QList<quint16> celFrameIndicesList;
        for (int j = 0; j < subtileNumberOfCelFrames; j++) {
            celFrameIndicesList.append(0);
        }
        this->celFrameIndices.append(celFrameIndicesList);
    }

    // Read MIN binary data
    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    for (int i = 0; i < minSubtileCount; i++) {
        QList<quint16> &celFrameIndicesList = this->celFrameIndices[i];
        for (int j = 0; j < subtileNumberOfCelFrames; j++) {
            quint16 readWord;
            in >> readWord;
            quint16 id = readWord & 0x0FFF;
            celFrameIndicesList[j] = id;
            celFrameTypes[id] = static_cast<D1CEL_FRAME_TYPE>((readWord & 0x7000) >> 12);
        }
    }
    this->minFilePath = filePath;
    return true;
}

bool D1Min::save(const QString &gfxPath)
{
    QString filePath = gfxPath;
    filePath.chop(3);
    filePath += "min";

    QFile outFile = QFile(filePath);
    if (!outFile.open(QIODevice::WriteOnly | QFile::Truncate)) {
        QMessageBox::critical(nullptr, "Error", "Failed open file: " + filePath);
        return false;
    }

    // write to file
    QDataStream out(&outFile);
    out.setByteOrder(QDataStream::LittleEndian);
    for (int i = 0; i < this->celFrameIndices.size(); i++) {
        QList<quint16> &celFrameIndicesList = this->celFrameIndices[i];
        for (int j = 0; j < celFrameIndicesList.count(); j++) {
            quint16 writeWord = celFrameIndicesList[j];
            if (writeWord != 0) {
                writeWord |= ((quint16)this->gfx->getFrame(writeWord - 1)->getFrameType()) << 12;
            }
            out << writeWord;
        }
    }

    this->minFilePath = filePath; // this->load(filePath, subtileCount);

    return true;
}

QImage D1Min::getSubtileImage(int subtileIndex)
{
    if (subtileIndex < 0 || subtileIndex >= this->celFrameIndices.size())
        return QImage();

    unsigned subtileWidthPx = this->subtileWidth * MICRO_WIDTH;
    QImage subtile = QImage(subtileWidthPx,
        this->subtileHeight * MICRO_HEIGHT, QImage::Format_ARGB32);
    subtile.fill(Qt::transparent);
    QPainter subtilePainter(&subtile);

    unsigned dx = 0, dy = 0;
    int n = this->subtileWidth * this->subtileHeight;
    for (int i = 0; i < n; i++) {
        quint16 celFrameIndex = this->celFrameIndices.at(subtileIndex).at(i);

        if (celFrameIndex > 0)
            subtilePainter.drawImage(dx, dy,
                this->gfx->getFrameImage(celFrameIndex - 1));

        dx += MICRO_WIDTH;
        if (dx == subtileWidthPx) {
            dx = 0;
            dy += MICRO_HEIGHT;
        }
    }

    subtilePainter.end();
    return subtile;
}

QString D1Min::getFilePath()
{
    return this->minFilePath;
}

int D1Min::getSubtileCount()
{
    return this->celFrameIndices.count();
}

quint16 D1Min::getSubtileWidth()
{
    return this->subtileWidth;
}

void D1Min::setSubtileWidth(int width)
{
    if (width == 0) {
        return;
    }
    int prevWidth = this->subtileWidth;
    int diff = width - prevWidth;
    if (diff > 0) {
        // extend the subtile-width
        for (int i = 0; i < this->celFrameIndices.size(); i++) {
            QList<quint16> &celFrameIndicesList = this->celFrameIndices[i];
            for (int y = 0; y < this->subtileHeight; y++) {
                for (int dx = 0; dx < diff; dx++) {
                    celFrameIndicesList.insert(y * width + prevWidth, 0);
                }
            }
        }
    } else if (diff < 0) {
        diff = -diff;
        // check if there is a non-zero frame in the subtiles
        bool hasFrame = false;
        for (int i = 0; i < this->celFrameIndices.size() && !hasFrame; i++) {
            QList<quint16> &celFrameIndicesList = this->celFrameIndices[i];
            for (int y = 0; y < this->subtileHeight; y++) {
                for (int x = width; x < prevWidth; x++) {
                    if (celFrameIndicesList[y * prevWidth + x] != 0) {
                        hasFrame = true;
                        break;
                    }
                }
            }
        }
        if (hasFrame) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(nullptr, "Confirmation", "Non-transparent frames are going to be eliminited. Are you sure you want to proceed?", QMessageBox::Yes | QMessageBox::No);
            if (reply != QMessageBox::Yes) {
                return;
            }
        }
        // reduce the subtile-width
        for (int i = 0; i < this->celFrameIndices.size(); i++) {
            QList<quint16> &celFrameIndicesList = this->celFrameIndices[i];
            for (int y = 0; y < this->subtileHeight; y++) {
                for (int dx = 0; dx < diff; dx++) {
                    celFrameIndicesList.takeAt((y + 1) * width);
                }
            }
        }
    }
    this->subtileWidth = width;
}

quint16 D1Min::getSubtileHeight()
{
    return this->subtileHeight;
}

void D1Min::setSubtileHeight(int height)
{
    if (height == 0) {
        return;
    }
    int width = this->subtileWidth;
    int diff = height - this->subtileHeight;
    if (diff > 0) {
        // extend the subtile-height
        int n = diff * width;
        for (int i = 0; i < this->celFrameIndices.size(); i++) {
            QList<quint16> &celFrameIndicesList = this->celFrameIndices[i];
            for (int j = 0; j < n; j++) {
                celFrameIndicesList.push_front(0);
            }
        }
    } else if (diff < 0) {
        diff = -diff;
        // check if there is a non-zero frame in the subtiles
        bool hasFrame = false;
        int n = diff * width;
        for (int i = 0; i < this->celFrameIndices.size() && !hasFrame; i++) {
            QList<quint16> &celFrameIndicesList = this->celFrameIndices[i];
            for (int j = 0; j < n; j++) {
                if (celFrameIndicesList[j] != 0) {
                    hasFrame = true;
                    break;
                }
            }
        }
        if (hasFrame) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(nullptr, "Confirmation", "Non-transparent frames are going to be eliminited. Are you sure you want to proceed?", QMessageBox::Yes | QMessageBox::No);
            if (reply != QMessageBox::Yes) {
                return;
            }
        }
        // reduce the subtile-height
        for (int i = 0; i < this->celFrameIndices.size(); i++) {
            QList<quint16> &celFrameIndicesList = this->celFrameIndices[i];
            celFrameIndicesList.erase(celFrameIndicesList.begin(), celFrameIndicesList.begin() + n);
        }
    }
    this->subtileHeight = height;
}

QList<quint16> &D1Min::getCelFrameIndices(int subtileIndex)
{
    return const_cast<QList<quint16> &>(this->celFrameIndices.at(subtileIndex));
}

void D1Min::insertSubtile(int subtileIndex, const QList<quint16> &frameIndicesList)
{
    this->celFrameIndices.insert(subtileIndex, frameIndicesList);
}

void D1Min::createSubtile()
{
    QList<quint16> celFrameIndicesList;
    int n = this->subtileWidth * this->subtileHeight;

    for (int i = 0; i < n; i++) {
        celFrameIndicesList.append(0);
    }
    this->celFrameIndices.append(celFrameIndicesList);
}

void D1Min::removeSubtile(int subtileIndex)
{
    this->celFrameIndices.removeAt(subtileIndex);
}

void D1Min::remapSubtiles(const QMap<unsigned, unsigned> &remap)
{
    QList<QList<quint16>> newCelFrameIndices;

    for (auto iter = remap.cbegin(); iter != remap.cend(); ++iter) {
        newCelFrameIndices.append(this->celFrameIndices.at(iter.value()));
    }
    this->celFrameIndices.swap(newCelFrameIndices);
}
