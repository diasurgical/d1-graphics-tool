#include "d1sol.h"

#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>

bool D1Sol::load(QString filePath)
{
    this->subProperties.clear();

    // Opening SOL file with a QBuffer to load it in RAM
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

    // Read SOL binary data
    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    quint8 readBytr;
    for (int i = 0; i < file.size(); i++) {
        in >> readBytr;
        this->subProperties.append(readBytr);
    }

    this->solFilePath = filePath;
    return true;
}

bool D1Sol::save(SaveAsParam *params)
{
    QString selectedPath = params != nullptr ? params->solFilePath : "";
    std::optional<QFile *> outFile = SaveAsParam::getValidSaveOutput(this->getFilePath(), selectedPath);
    if (!outFile) {
        return false;
    }

    // write to file
    QDataStream out(*outFile);
    for (int i = 0; i < this->subProperties.size(); i++) {
        out << this->subProperties[i];
    }

    QFileInfo fileinfo = QFileInfo(**outFile);
    this->solFilePath = fileinfo.fileName(); // this->load(filePath);
    delete *outFile;

    return true;
}

QString D1Sol::getFilePath()
{
    return this->solFilePath;
}

quint16 D1Sol::getSubtileCount()
{
    return this->subProperties.count();
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
