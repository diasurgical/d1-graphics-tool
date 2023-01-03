#include "d1min.h"

#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QPainter>

bool D1Min::load(QString filePath, quint16 subtileCount, std::map<unsigned, D1CEL_FRAME_TYPE> &celFrameTypes)
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

    this->subtileWidth = 2;
    this->subtileHeight = file.size() / (subtileCount * this->subtileWidth * 2);
    if (file.size() != subtileCount * this->subtileWidth * this->subtileHeight * 2) {
        qDebug() << "The size of sol-file does not align with min-file";
        subtileCount = file.size() / (this->subtileWidth * this->subtileHeight * 2);
    }

    // File size check
    if (file.size() % this->subtileHeight != 0)
        return false;

    // Read sub-tile data
    quint8 subtileNumberOfCelFrames = this->subtileWidth * this->subtileHeight;
    fileBuffer.seek(0);
    this->celFrameIndices.clear();
    for (int i = 0; i < subtileCount; i++) {
        QList<quint16> celFrameIndicesList;
        for (int j = 0; j < subtileNumberOfCelFrames; j++) {
            quint16 readWord;
            in >> readWord;
            quint16 id = readWord & 0x0FFF;
            celFrameIndicesList.append(id);
            celFrameTypes[id] = static_cast<D1CEL_FRAME_TYPE>((readWord & 0x7000) >> 12);
        }
        this->celFrameIndices.append(celFrameIndicesList);
    }
    this->minFilePath = filePath;
    return true;
}

bool D1Min::save(D1Gfx *gfx, SaveAsParam *params)
{
    QString selectedPath = params != nullptr ? params->minFilePath : "";
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
                writeWord |= ((quint16)gfx->getFrame(writeWord - 1)->getFrameType()) << 12;
            }
            out << writeWord;
        }
    }

    QFileInfo fileinfo = QFileInfo(**outFile);
    this->minFilePath = fileinfo.fileName(); // this->load(filePath, subtileCount);
    delete *outFile;

    return true;
}

QImage D1Min::getSubtileImage(quint16 subtileIndex)
{
    if (this->cel == nullptr || subtileIndex >= this->celFrameIndices.size())
        return QImage();

    QImage subtile = QImage(this->subtileWidth * MICRO_WIDTH,
        this->subtileHeight * MICRO_HEIGHT, QImage::Format_ARGB32);
    subtile.fill(Qt::transparent);
    QPainter subtilePainter(&subtile);

    quint16 dx = 0, dy = 0;
    for (int i = 0; i < this->subtileWidth * this->subtileHeight; i++) {
        quint16 celFrameIndex = this->celFrameIndices.at(subtileIndex).at(i);

        if (celFrameIndex > 0)
            subtilePainter.drawImage(dx, dy,
                this->cel->getFrameImage(celFrameIndex - 1));

        if (dx == MICRO_WIDTH) {
            dy += MICRO_HEIGHT;
            dx = 0;
        } else {
            dx = MICRO_WIDTH;
        }
    }

    subtilePainter.end();
    return subtile;
}

QString D1Min::getFilePath()
{
    return this->minFilePath;
}

void D1Min::setCel(D1Gfx *c)
{
    this->cel = c;
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

QList<quint16> &D1Min::getCelFrameIndices(quint16 subTileIndex)
{
    return const_cast<QList<quint16> &>(this->celFrameIndices.at(subTileIndex));
}
