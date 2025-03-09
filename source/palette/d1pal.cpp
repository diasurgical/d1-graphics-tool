#include "d1pal.h"

#include <QDataStream>
#include <QTextStream>

bool D1Pal::load(QString filePath)
{
    QFile file = QFile(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    if (file.size() == D1PAL_SIZE_BYTES) {
        this->loadRegularPalette(file);
    } else if (!this->loadJascPalette(file)) {
        return false;
    }

    for (int i = 0; i < 32; i++)
        this->origCyclePalette[i] = this->colors[i];

    this->palFilePath = filePath;
    this->modified = false;
    return true;
}

void D1Pal::loadRegularPalette(QFile &file)
{
    QDataStream in(&file);

    for (int i = 0; i < D1PAL_COLORS; i++) {
        quint8 red;
        in >> red;

        quint8 green;
        in >> green;

        quint8 blue;
        in >> blue;

        this->colors[i] = QColor(red, green, blue);
    }
}

bool D1Pal::loadJascPalette(QFile &file)
{
    QTextStream txt(&file);
    QString line;
    QStringList lineParts;
    quint16 lineNumber = 0;

    while (!txt.atEnd()) {
        line = txt.readLine();
        lineNumber++;

        if (lineNumber == 1 && line != "JASC-PAL")
            return false;
        if (lineNumber == 3 && line != "256")
            return false;

        if (lineNumber <= 3)
            continue;
        if (lineNumber > 256 + 3)
            continue;

        lineParts = line.split(" ");
        if (lineParts.size() != 3) {
            return false;
        }

        quint8 red = lineParts[0].toInt();
        quint8 green = lineParts[1].toInt();
        quint8 blue = lineParts[2].toInt();
        // assert(D1PAL_COLORS == 256);
        this->colors[lineNumber - 4] = QColor(red, green, blue);
    }

    return lineNumber >= D1PAL_COLORS + 3;
}

bool D1Pal::save(QString filePath)
{
    QFile file = QFile(filePath);

    if (!file.open(QIODevice::WriteOnly))
        return false;

    QDataStream out(&file);
    for (int i = 0; i < D1PAL_COLORS; i++) {
        QColor color = this->colors[i];
        quint8 byteToWrite;

        byteToWrite = color.red();
        out << byteToWrite;

        byteToWrite = color.green();
        out << byteToWrite;

        byteToWrite = color.blue();
        out << byteToWrite;
    }

    if (this->palFilePath == filePath) {
        this->modified = false;
    } else {
        // -- do not update, the user is creating a new one and the original needs to be preserved
        // this->modified = false;
        // this->palFilePath = filePath;
    }
    return true;
}

bool D1Pal::isModified() const
{
    return this->modified;
}

QString D1Pal::getFilePath()
{
    return this->palFilePath;
}

QString D1Pal::getDefaultPath() const
{
    return DEFAULT_PATH;
}

QString D1Pal::getDefaultName() const
{
    return DEFAULT_NAME;
}

QColor D1Pal::getColor(quint8 index)
{
    return this->colors[index];
}

void D1Pal::setColor(quint8 index, QColor color)
{
    this->colors[index] = color;
    if (index < 32)
        this->origCyclePalette[index] = color;
    this->modified = true;
}

void D1Pal::resetColors()
{
    for (int i = 0; i < 32; i++)
        this->colors[i] = this->origCyclePalette[i];
}

void D1Pal::cycleColors(D1PAL_CYCLE_TYPE type)
{
    QColor celColor;
    int i;

    switch (type) {
    case D1PAL_CYCLE_TYPE::CAVES:
    case D1PAL_CYCLE_TYPE::HELL:
        celColor = this->colors[1];
        for (i = 1; i < 31; i++) {
            this->colors[i] = this->colors[i + 1];
        }
        this->colors[i] = celColor;
        break;
    case D1PAL_CYCLE_TYPE::NEST:
        if (--this->currentCycleCounter != 0)
            break;
        this->currentCycleCounter = 3;
        celColor = this->colors[8];
        for (i = 8; i > 1; i--) {
            this->colors[i] = this->colors[i - 1];
        }
        this->colors[i] = celColor;

        celColor = this->colors[15];
        for (i = 15; i > 9; i--) {
            this->colors[i] = this->colors[i - 1];
        }
        this->colors[i] = celColor;
        break;
    case D1PAL_CYCLE_TYPE::CRYPT:
        if (--this->currentCycleCounter == 0) {
            this->currentCycleCounter = 3;

            celColor = this->colors[15];
            for (i = 15; i > 1; i--) {
                this->colors[i] = this->colors[i - 1];
            }
            this->colors[i] = celColor;
        }

        celColor = this->colors[31];
        for (i = 31; i > 16; i--) {
            this->colors[i] = this->colors[i - 1];
        }
        this->colors[i] = celColor;
        break;
    }
}
