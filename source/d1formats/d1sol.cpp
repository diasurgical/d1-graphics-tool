#include "d1sol.h"

#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

bool D1Sol::load(QString filePath)
{
    // prepare file data source
    QFile file;
    // done by the caller
    // if (!params.solFilePath.isEmpty()) {
    //    filePath = params.solFilePath;
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

    int subTileCount = file.size();

    this->subProperties.clear();

    // Read SOL binary data
    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    for (int i = 0; i < subTileCount; i++) {
        quint8 readBytr;
        in >> readBytr;
        this->subProperties.append(readBytr);
    }

    this->solFilePath = filePath;
    this->modified = false;

    return true;
}

bool D1Sol::save(const QString &gfxPath)
{
    QString filePath = gfxPath;
    filePath.chop(3);
    filePath += "sol";

    QFile outFile = QFile(filePath);
    if (!outFile.open(QIODevice::WriteOnly | QFile::Truncate)) {
        QMessageBox::critical(nullptr, "Error", "Failed open file: " + filePath);
        return false;
    }

    // write to file
    QDataStream out(&outFile);
    for (int i = 0; i < this->subProperties.size(); i++) {
        out << this->subProperties[i];
    }

    this->solFilePath = filePath;
    this->modified = false;

    return true;
}

bool D1Sol::isModified() const
{
    return this->modified;
}

QString D1Sol::getFilePath()
{
    return this->solFilePath;
}

quint16 D1Sol::getSubtileCount()
{
    return this->subProperties.count();
}

quint8 D1Sol::getSubtileProperties(int subtileIndex)
{
    if (subtileIndex >= this->subProperties.count())
        return 0;

    return this->subProperties.at(subtileIndex);
}

void D1Sol::insertSubtile(int subtileIndex, quint8 value)
{
    this->subProperties.insert(subtileIndex, value);
    this->modified = true;
}

void D1Sol::setSubtileProperties(int subtileIndex, quint8 value)
{
    this->subProperties[subtileIndex] = value;
    this->modified = true;
}

void D1Sol::createSubtile()
{
    this->subProperties.append(0);
    this->modified = true;
}

void D1Sol::removeSubtile(int subtileIndex)
{
    this->subProperties.removeAt(subtileIndex);
    this->modified = true;
}

void D1Sol::remapSubtiles(const QMap<unsigned, unsigned> &remap)
{
    QList<quint8> newSubProperties;

    for (auto iter = remap.cbegin(); iter != remap.cend(); ++iter) {
        newSubProperties.append(this->subProperties.at(iter.value()));
    }
    this->subProperties.swap(newSubProperties);
    this->modified = true;
}
