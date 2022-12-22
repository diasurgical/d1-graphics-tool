#include "d1amp.h"

#include <QBuffer>
#include <QDataStream>

D1Amp::D1Amp(QString path)
{
    this->load(path);
}

D1Amp::~D1Amp()
{
    if (this->file.isOpen())
        this->file.close();
}

bool D1Amp::load(QString ampFilePath, int allocate)
{
    this->properties.clear();
    this->properties.reserve(allocate);
    std::fill(this->properties.begin(), this->properties.end(), 0);
    this->types.clear();
    this->types.reserve(allocate);
    std::fill(this->types.begin(), this->types.end(), 0);

    // Opening AMP file with a QBuffer to load it in RAM
    if (!QFile::exists(ampFilePath))
        return false;

    if (this->file.isOpen())
        this->file.close();

    this->file.setFileName(ampFilePath);

    if (!this->file.open(QIODevice::ReadOnly))
        return false;

    QByteArray fileData = this->file.readAll();
    QBuffer fileBuffer(&fileData);

    if (!fileBuffer.open(QIODevice::ReadOnly))
        return false;

    // Read AMP binary data
    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    this->types.reserve(this->file.size());
    std::fill(this->types.begin(), this->types.end(), 0);
    this->properties.reserve(this->file.size());
    std::fill(this->properties.begin(), this->properties.end(), 0);
    quint8 readBytr;
    for (int i = 0; i < this->file.size() / 2; i++) {
        in >> readBytr;
        this->types[i] = readBytr;
        in >> readBytr;
        this->properties[i] = readBytr;
    }

    return true;
}

QString D1Amp::getFilePath()
{
    if (!this->file.isOpen())
        return QString();

    return this->file.fileName();
}

quint8 D1Amp::getTileType(quint16 tileIndex)
{
    if (tileIndex >= this->properties.count())
        return 0;

    return this->types.at(tileIndex);
}

quint8 D1Amp::getTileProperties(quint16 tileIndex)
{
    if (tileIndex >= this->properties.count())
        return 0;

    return this->properties.at(tileIndex);
}

void D1Amp::setTileType(quint16 tileIndex, quint8 value)
{
    this->types[tileIndex] = value;
}

void D1Amp::setTileProperties(quint16 tileIndex, quint8 value)
{
    this->properties[tileIndex] = value;
}
