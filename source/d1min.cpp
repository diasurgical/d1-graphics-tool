#include "d1min.h"

#include <QBuffer>
#include <QMap>
#include <QPointer>

D1Min::~D1Min()
{
    if (this->file.isOpen())
        this->file.close();
}

bool D1Min::load(QString minFilePath, quint16 subtileCount)
{
    // Opening MIN file with a QBuffer to load it in RAM
    if (!QFile::exists(minFilePath))
        return false;

    if (this->file.isOpen())
        this->file.close();

    this->file.setFileName(minFilePath);

    if (!this->file.open(QIODevice::ReadOnly))
        return false;

    if (this->file.size() == 0)
        return false;

    QByteArray fileData = this->file.readAll();
    QBuffer fileBuffer(&fileData);

    if (!fileBuffer.open(QIODevice::ReadOnly))
        return false;

    // Read MIN binary data
    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    this->subtileHeight = this->file.size() / 2 / subtileCount / 2;

    // File size check
    if (this->file.size() % this->subtileHeight != 0)
        return false;

    // Read sub-tile data
    quint8 subtileNumberOfCelFrames = this->subtileWidth * this->subtileHeight;
    fileBuffer.seek(0);
    this->celFrameIndices.clear();
    this->celFrameTypes.clear();
    for (int i = 0; i < subtileCount; i++) {
        QList<quint16> celFrameIndicesList;
        for (int j = 0; j < subtileNumberOfCelFrames; j++) {
            quint16 readWord;
            in >> readWord;
            quint16 id = readWord & 0x0FFF;
            celFrameIndicesList.append(id);
            this->celFrameTypes[id] = static_cast<D1CEL_FRAME_TYPE>((readWord & 0x7000) >> 12);
        }
        this->celFrameIndices.append(celFrameIndicesList);
    }

    return true;
}

QImage D1Min::getSubtileImage(quint16 subtileIndex)
{
    if (this->cel == nullptr || subtileIndex >= this->celFrameIndices.size())
        return QImage();

    QImage subtile = QImage(this->subtileWidth * 32,
        this->subtileHeight * 32, QImage::Format_ARGB32);
    subtile.fill(Qt::transparent);
    QPainter subtilePainter(&subtile);

    quint16 dx = 0, dy = 0;
    for (int i = 0; i < this->subtileWidth * this->subtileHeight; i++) {
        quint16 celFrameIndex = this->celFrameIndices.at(subtileIndex).at(i);

        if (celFrameIndex > 0)
            subtilePainter.drawImage(dx, dy,
                this->cel->getFrameImage(celFrameIndex - 1));

        if (dx == 32) {
            dy += 32;
            dx = 0;
        } else {
            dx = 32;
        }
    }

    subtilePainter.end();
    return subtile;
}

D1MIN_TYPE D1Min::getType()
{
    return this->type;
}

QString D1Min::getFilePath()
{
    if (!this->file.isOpen())
        return QString();

    return this->file.fileName();
}

D1CelBase *D1Min::getCel()
{
    return this->cel;
}

void D1Min::setCel(D1CelBase *c)
{
    this->cel = c;
}

D1CEL_FRAME_TYPE D1Min::getFrameType(quint16 id)
{
    return this->celFrameTypes[id];
}

quint16 D1Min::getSubtileWidth()
{
    return this->subtileWidth;
}

quint16 D1Min::getSubtileHeight()
{
    return this->subtileHeight;
}

QList<quint16> D1Min::getCelFrameIndices(quint16 subTileIndex)
{
    if (subTileIndex >= this->celFrameIndices.count())
        return QList<quint16>();

    return this->celFrameIndices.at(subTileIndex);
}
