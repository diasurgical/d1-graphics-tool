#include "d1amp.h"

#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>

bool D1Amp::clear(int allocate)
{
    for (int i = 0; i < allocate; i++) {
        this->types.append(0);
        this->properties.append(0);
    }
    return false;
}

bool D1Amp::load(QString filePath, int allocate)
{
    this->properties.clear();
    this->types.clear();

    // Opening AMP file with a QBuffer to load it in RAM
    if (!QFile::exists(filePath))
        return this->clear(allocate);

    QFile file = QFile(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return this->clear(allocate);

    QByteArray fileData = file.readAll();
    QBuffer fileBuffer(&fileData);

    if (!fileBuffer.open(QIODevice::ReadOnly))
        return this->clear(allocate);

    // Read AMP binary data
    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    quint8 readBytr;
    for (int i = 0; i < file.size() / 2; i++) {
        in >> readBytr;
        this->types.append(readBytr);
        in >> readBytr;
        this->properties.append(readBytr);
    }

    while (this->types.size() < allocate) {
        this->types.append(0);
        this->properties.append(0);
    }
    this->ampFilePath = filePath;
    return true;
}

bool D1Amp::save(SaveAsParam *params)
{
    QString selectedPath = params != nullptr ? params->ampFilePath : "";
    std::optional<QFile *> outFile = SaveAsParam::getValidSaveOutput(this->getFilePath(), selectedPath);
    if (!outFile) {
        return false;
    }

    // write to file
    QDataStream out(*outFile);
    for (int i = 0; i < this->types.size(); i++) {
        out << this->types[i];
        out << this->properties[i];
    }

    QFileInfo fileinfo = QFileInfo(**outFile);
    this->ampFilePath = fileinfo.filePath(); // this->load(filePath, allocate);
    delete *outFile;

    return true;
}

QString D1Amp::getFilePath()
{
    return this->ampFilePath;
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
