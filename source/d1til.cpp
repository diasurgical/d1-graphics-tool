#include "d1til.h"

#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QPainter>

#define TILE_SIZE 4

bool D1Til::load(QString filePath)
{
    // prepare file data source
    QFile file;
    // done by the caller
    // if (!params.tilFilePath.isEmpty()) {
    //    filePath = params.tilFilePath;
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

    // File size check
    auto fileSize = file.size();
    if (fileSize % (2 * TILE_SIZE) != 0) {
        qDebug() << "Invalid til-file.";
        return false;
    }

    int tileCount = fileSize / (2 * TILE_SIZE);

    // Read TIL binary data
    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    this->subtileIndices.clear();
    for (int i = 0; i < tileCount; i++) {
        QList<quint16> subtileIndicesList;
        for (int j = 0; j < TILE_SIZE; j++) {
            quint16 readWord;
            in >> readWord;
            subtileIndicesList.append(readWord);
        }
        this->subtileIndices.append(subtileIndicesList);
    }
    this->tilFilePath = filePath;
    return true;
}

bool D1Til::save(const SaveAsParam &params)
{
    QString selectedPath = params.tilFilePath;
    std::optional<QFile *> outFile = SaveAsParam::getValidSaveOutput(this->getFilePath(), selectedPath);
    if (!outFile) {
        return false;
    }

    // write to file
    QDataStream out(*outFile);
    out.setByteOrder(QDataStream::LittleEndian);
    for (int i = 0; i < this->subtileIndices.count(); i++) {
        QList<quint16> &subtileIndicesList = this->subtileIndices[i];
        for (int j = 0; j < TILE_SIZE; j++) {
            quint16 writeWord = subtileIndicesList[j];
            out << writeWord;
        }
    }

    QFileInfo fileinfo = QFileInfo(**outFile);
    this->tilFilePath = fileinfo.filePath(); // this->load(filePath);
    delete *outFile;

    return true;
}

QImage D1Til::getTileImage(quint16 tileIndex)
{
    if (this->min == nullptr || tileIndex >= this->subtileIndices.size())
        return QImage();

    QImage tile = QImage(this->tilePixelWidth,
        this->tilePixelHeight, QImage::Format_ARGB32);
    tile.fill(Qt::transparent);
    QPainter tilePainter(&tile);

    tilePainter.drawImage(32, 0,
        this->min->getSubtileImage(
            this->subtileIndices.at(tileIndex).at(0)));

    tilePainter.drawImage(64, 16,
        this->min->getSubtileImage(
            this->subtileIndices.at(tileIndex).at(1)));

    tilePainter.drawImage(0, 16,
        this->min->getSubtileImage(
            this->subtileIndices.at(tileIndex).at(2)));

    tilePainter.drawImage(32, 32,
        this->min->getSubtileImage(
            this->subtileIndices.at(tileIndex).at(3)));

    tilePainter.end();
    return tile;
}

QString D1Til::getFilePath()
{
    return this->tilFilePath;
}

void D1Til::setMin(D1Min *m)
{
    this->min = m;

    this->tileWidth = this->min->getSubtileWidth() * 2;
    this->tileHeight = this->min->getSubtileHeight() + 1;

    this->tilePixelWidth = this->tileWidth * MICRO_WIDTH;
    this->tilePixelHeight = this->tileHeight * MICRO_HEIGHT;
}

int D1Til::getTileCount()
{
    return this->subtileIndices.count();
}

quint16 D1Til::getTileWidth()
{
    return this->tileWidth;
}

quint16 D1Til::getTileHeight()
{
    return this->tileHeight;
}

quint16 D1Til::getTilePixelWidth()
{
    return this->tilePixelWidth;
}

quint16 D1Til::getTilePixelHeight()
{
    return this->tilePixelHeight;
}

QList<quint16> &D1Til::getSubtileIndices(int tileIndex)
{
    return const_cast<QList<quint16> &>(this->subtileIndices.at(tileIndex));
}

void D1Til::createTile()
{
    QList<quint16> subtileIndices;

    for (int i = 0; i < TILE_SIZE; i++) {
        subtileIndices.append(0);
    }
    this->subtileIndices.append(subtileIndices);
}

void D1Til::removeTile(int tileIndex)
{
    this->subtileIndices.removeAt(tileIndex);
}
