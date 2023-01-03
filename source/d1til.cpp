#include "d1til.h"

#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QPainter>

bool D1Til::load(QString filePath)
{
    quint16 readWord;
    QList<quint16> subtileIndicesList;

    // Opening MIN file with a QBuffer to load it in RAM
    if (!QFile::exists(filePath))
        return false;

    QFile file = QFile(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    if (file.size() < 16)
        return false;

    QByteArray fileData = file.readAll();
    QBuffer fileBuffer(&fileData);

    if (!fileBuffer.open(QIODevice::ReadOnly))
        return false;

    // Read TIL binary data
    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    int tileCount = file.size() / 2 / 4;

    this->subtileIndices.clear();
    for (int i = 0; i < tileCount; i++) {
        subtileIndicesList.clear();
        for (int j = 0; j < 4; j++) {
            in >> readWord;
            subtileIndicesList.append(readWord);
        }
        this->subtileIndices.append(subtileIndicesList);
    }
    this->tilFilePath = filePath;
    return true;
}

bool D1Til::save(SaveAsParam *params)
{
    QString selectedPath = params != nullptr ? params->tilFilePath : "";
    std::optional<QFile *> outFile = SaveAsParam::getValidSaveOutput(this->getFilePath(), selectedPath);
    if (!outFile) {
        return false;
    }

    // write to file
    QDataStream out(*outFile);
    out.setByteOrder(QDataStream::LittleEndian);
    for (int i = 0; i < this->subtileIndices.count(); i++) {
        QList<quint16> &subtileIndicesList = this->subtileIndices[i];
        for (int j = 0; j < 4; j++) {
            quint16 writeWord = subtileIndicesList[j];
            out << writeWord;
        }
    }

    QFileInfo fileinfo = QFileInfo(**outFile);
    this->tilFilePath = fileinfo.fileName(); // this->load(filePath);
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
