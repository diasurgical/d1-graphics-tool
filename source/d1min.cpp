#include "d1min.h"

#include <QBuffer>
#include <QMap>
#include <QPointer>

bool D1Min::load(QString filePath, quint16 subtileCount)
{
    // Opening MIN file with a QBuffer to load it in RAM
    if (!QFile::exists(filePath))
        return false;

    QFile file = QFile(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    if (file.size() == 0)
        return false;

    QByteArray fileData = file.readAll();
    QBuffer fileBuffer(&fileData);

    if (!fileBuffer.open(QIODevice::ReadOnly))
        return false;

    // Read MIN binary data
    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    this->subtileHeight = file.size() / 2 / subtileCount / 2;
    if ((file.size() / 2) % (this->subtileHeight * 2) == 0) {
        qDebug() << "The size of sol-file does not align with min-file";
        subtileCount = file.size() / 2 / this->subtileHeight / 2;
    }

    // File size check
    if (file.size() % this->subtileHeight != 0)
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
    this->minFilePath = filePath;
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
    return this->minFilePath;
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
    if (this->celFrameTypes.find(id) == this->celFrameTypes.end()) {
        return D1CEL_FRAME_TYPE::Unknown;
    } else {
        return this->celFrameTypes[id];
    }
}

quint16 D1Min::getSubtileCount()
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

QList<quint16> D1Min::getCelFrameIndices(quint16 subTileIndex)
{
    if (subTileIndex >= this->celFrameIndices.count())
        return QList<quint16>();

    return this->celFrameIndices.at(subTileIndex);
}
