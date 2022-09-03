#include "d1til.h"

D1Til::D1Til()
    : type(D1TIL_TYPE::REGULAR_HEIGHT)
    , file()
    , min(nullptr)
    , tileCount(0)
    , tileWidth(0)
    , tileHeight(0)
    , tilePixelWidth(0)
    , tilePixelHeight(0)
{
}

D1Til::D1Til(QString path, D1Min *m)
    : type(D1TIL_TYPE::REGULAR_HEIGHT)
    , file()
    , min(m)
    , tileCount(0)
    , tileWidth(0)
    , tileHeight(0)
    , tilePixelWidth(0)
    , tilePixelHeight(0)
{
    this->load(path);
}

D1Til::~D1Til()
{
    if (this->file.isOpen())
        this->file.close();
}

bool D1Til::load(QString tilFilePath)
{
    quint16 readWord;
    QList<quint16> subtileIndicesList;

    // Opening MIN file with a QBuffer to load it in RAM
    if (!QFile::exists(tilFilePath))
        return false;

    if (this->file.isOpen())
        this->file.close();

    this->file.setFileName(tilFilePath);

    if (!this->file.open(QIODevice::ReadOnly))
        return false;

    if (this->file.size() < 16)
        return false;

    QByteArray fileData = this->file.readAll();
    QBuffer fileBuffer(&fileData);

    if (!fileBuffer.open(QIODevice::ReadOnly))
        return false;

    // Read TIL binary data
    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    this->tileCount = this->file.size() / 8;

    this->subtileIndices.clear();
    for (int i = 0; i < this->tileCount; i++) {
        subtileIndicesList.clear();
        for (int j = 0; j < 4; j++) {
            in >> readWord;
            subtileIndicesList.append(readWord);
        }
        this->subtileIndices.append(subtileIndicesList);
    }

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

D1TIL_TYPE D1Til::getType()
{
    return this->type;
}

QString D1Til::getFilePath()
{
    if (!this->file.isOpen())
        return QString();

    return this->file.fileName();
}

bool D1Til::isFileOpen()
{
    return this->file.isOpen();
}

D1Min *D1Til::getMin()
{
    return this->min;
}

void D1Til::setMin(D1Min *m)
{
    this->min = m;

    this->tileWidth = this->min->getSubtileWidth() * 2;
    this->tileHeight = this->min->getSubtileHeight() + 1;

    this->tilePixelWidth = this->tileWidth * 32;
    this->tilePixelHeight = this->tileHeight * 32;
}

quint16 D1Til::getTileCount()
{
    return this->tileCount;
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

QList<quint16> D1Til::getSubtileIndices(quint16 tileIndex)
{
    if (tileIndex >= this->tileCount)
        return QList<quint16>();

    return this->subtileIndices.at(tileIndex);
}
