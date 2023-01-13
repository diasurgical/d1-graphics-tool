#include "d1amp.h"

#include <QBuffer>
#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

bool D1Amp::load(QString filePath, int tileCount, const OpenAsParam &params)
{
    // prepare file data source
    QFile file;
    // done by the caller
    // if (!params.ampFilePath.isEmpty()) {
    //    filePath = params.ampFilePath;
    // }
    if (!filePath.isEmpty()) {
        file.setFileName(filePath);
        if (!file.open(QIODevice::ReadOnly) && !params.ampFilePath.isEmpty()) {
            return false; // report read-error only if the file was explicitly requested
        }
    }

    QByteArray fileData = file.readAll();
    QBuffer fileBuffer(&fileData);

    if (!fileBuffer.open(QIODevice::ReadOnly)) {
        return false;
    }

    // File size check
    auto fileSize = file.size();
    if (fileSize % 2 != 0) {
        qDebug() << "Invalid amp-file.";
        return false;
    }

    int ampTileCount = fileSize / 2;
    if (ampTileCount != tileCount) {
        if (ampTileCount != 0) {
            qDebug() << "The size of amp-file does not align with til-file";
        }
        if (ampTileCount > tileCount) {
            ampTileCount = tileCount; // skip unusable data
        }
    }

    // prepare empty lists with zeros
    this->properties.clear();
    this->types.clear();
    for (int i = 0; i < tileCount; i++) {
        this->types.append(0);
        this->properties.append(0);
    }

    // Read AMP binary data
    QDataStream in(&fileBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    for (int i = 0; i < ampTileCount; i++) {
        quint8 readBytr;
        in >> readBytr;
        this->types[i] = readBytr;
        in >> readBytr;
        this->properties[i] = readBytr;
    }

    this->ampFilePath = filePath;
    return true;
}

bool D1Amp::save(const SaveAsParam &params)
{
    QString filePath = this->getFilePath();
    if (!params.ampFilePath.isEmpty()) {
        filePath = params.ampFilePath;
        if (QFile::exists(filePath)) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(nullptr, "Confirmation", "Are you sure you want to overwrite " + filePath + "?", QMessageBox::Yes | QMessageBox::No);
            if (reply != QMessageBox::Yes) {
                return false;
            }
        }
    }

    QFile outFile = QFile(filePath);
    if (!outFile.open(QIODevice::WriteOnly | QFile::Truncate)) {
        QMessageBox::critical(nullptr, "Error", "Failed open file: " + filePath);
        return false;
    }

    // write to file
    QDataStream out(&outFile);
    for (int i = 0; i < this->types.size(); i++) {
        out << this->types[i];
        out << this->properties[i];
    }

    this->ampFilePath = filePath; // this->load(filePath, allocate);

    return true;
}

QString D1Amp::getFilePath()
{
    return this->ampFilePath;
}

quint8 D1Amp::getTileType(quint16 tileIndex)
{
    if (tileIndex >= this->types.count())
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

void D1Amp::createTile()
{
    this->types.append(0);
    this->properties.append(0);
}

void D1Amp::removeTile(int tileIndex)
{
    this->types.removeAt(tileIndex);
    this->properties.removeAt(tileIndex);
}
