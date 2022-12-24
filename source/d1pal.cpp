#include "d1pal.h"

#include <QTextStream>

D1Pal::~D1Pal()
{
    delete[] colors;

    if (this->file.isOpen())
        this->file.close();
}

bool D1Pal::load(QString palFilePath)
{
    if (!QFile::exists(palFilePath))
        return false;

    if (this->file.isOpen())
        file.close();

    this->file.setFileName(palFilePath);

    if (!this->file.open(QIODevice::ReadOnly))
        return false;

    // Check if the palette is a JASC palette
    if (file.read(8) == QString("JASC-PAL").toUtf8())
        this->type = D1PAL_TYPE::JASC;
    else
        this->type = D1PAL_TYPE::REGULAR;
    file.seek(0);

    switch (this->type) {
    case D1PAL_TYPE::REGULAR:
        return this->loadRegularPalette();
        break;

    case D1PAL_TYPE::JASC:
        return this->loadJascPalette();
        break;

    default:
        return false;
        break;
    }
}

bool D1Pal::loadRegularPalette()
{
    if (this->file.size() != D1PAL_SIZE_BYTES)
        return false;

    for (int i = 0; i < D1PAL_COLORS; i++) {
        QByteArray colorBytes = this->file.read(3);

        quint8 red = colorBytes[0];
        quint8 green = colorBytes[1];
        quint8 blue = colorBytes[2];

        this->colors[i] = QColor(red, green, blue);
    }

    this->modified = false;

    return true;
}

bool D1Pal::loadJascPalette()
{
    QTextStream txt(&this->file);
    QString line;
    QStringList lineParts;
    quint16 lineNumber = 0;

    while (!txt.atEnd()) {
        line = txt.readLine();
        lineNumber++;

        if (lineNumber == 3 && line != "256")
            return false;

        if (lineNumber > 3) {
            lineParts = line.split(" ");

            if (lineParts.size() != 3) {
                return false;
            }

            quint8 red = lineParts[0].toInt();
            quint8 green = lineParts[1].toInt();
            quint8 blue = lineParts[2].toInt();
            this->colors[lineNumber - 4] = QColor(red, green, blue);
        }
    }

    this->modified = false;

    return true;
}

bool D1Pal::save(QString palFilePath)
{
    if (this->file.isOpen())
        file.close();

    this->file.setFileName(palFilePath);

    if (!this->file.open(QIODevice::ReadWrite))
        return false;

    for (int i = 0; i < D1PAL_COLORS; i++) {
        QColor color = this->colors[i];

        QByteArray colorBytes;
        colorBytes.resize(3);
        colorBytes[0] = color.red();
        colorBytes[1] = color.green();
        colorBytes[2] = color.blue();

        if (this->file.write(colorBytes) == -1)
            return false;
    }

    if (!this->file.flush())
        return false;

    if (this->file.size() != D1PAL_SIZE_BYTES)
        return false;

    this->modified = false;

    return true;
}

bool D1Pal::isModified() const
{
    return this->modified;
}

QString D1Pal::getFilePath()
{
    if (!this->file.isOpen())
        return QString();

    return this->file.fileName();
}

QColor D1Pal::getColor(quint8 index)
{
    return this->colors[index];
}

void D1Pal::setColor(quint8 index, QColor color)
{
    this->colors[index] = color;
    this->modified = true;
}
