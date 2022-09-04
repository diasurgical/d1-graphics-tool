#include "d1sol.h"

#include <QBuffer>

D1Sol::D1Sol(QString path)
{
    this->load(path);
}

D1Sol::~D1Sol()
{
    if (this->file.isOpen())
        this->file.close();
}

bool D1Sol::load(QString solFilePath, int allocate)
{
    this->subProperties.clear();
    this->subProperties.fill(0, allocate);

    // Opening SOL file with a QBuffer to load it in RAM
    if (!QFile::exists(solFilePath))
        return false;

    if (this->file.isOpen())
        this->file.close();

    this->file.setFileName(solFilePath);

    if (!this->file.open(QIODevice::ReadOnly))
        return false;

    QByteArray fileData = this->file.readAll();
    QBuffer fileBuffer(&fileData);

    if (!fileBuffer.open(QIODevice::ReadOnly))
        return false;

    // Read SOL binary data
    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    quint8 readBytr;
    for (int i = 0; i < this->file.size(); i++) {
        in >> readBytr;
        this->subProperties.append(readBytr);
    }

    return true;
}

QString D1Sol::getFilePath()
{
    if (!this->file.isOpen())
        return QString();

    return this->file.fileName();
}

quint8 D1Sol::getSubtileProperties(quint16 tileIndex)
{
    if (tileIndex >= this->subProperties.count())
        return 0;

    return this->subProperties.at(tileIndex);
}

void D1Sol::setSubtileProperties(quint16 tileIndex, quint8 value)
{
    this->subProperties[tileIndex] = value;
}
