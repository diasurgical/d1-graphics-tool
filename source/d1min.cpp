#include "d1min.h"

#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QPainter>

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

bool D1Min::save(const SaveAsParam &params)
{
    QString selectedPath = params.minFilePath;
    std::optional<QFile *> outFile = SaveAsParam::getValidSaveOutput(this->getFilePath(), selectedPath);
    if (!outFile) {
        return false;
    }

    // write to file
    QDataStream out(*outFile);
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

    QFileInfo fileinfo = QFileInfo(**outFile);
    this->minFilePath = fileinfo.filePath(); // this->load(filePath, subtileCount);
    delete *outFile;

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

quint16 D1Min::getSubtileHeight()
{
    return this->subtileHeight;
}

QList<quint16> &D1Min::getCelFrameIndices(int subTileIndex)
{
    return const_cast<QList<quint16> &>(this->celFrameIndices.at(subTileIndex));
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

void D1Min::removeSubtile(int subTileIndex)
{
    this->celFrameIndices.removeAt(subTileIndex);
}
