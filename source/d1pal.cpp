#include "d1pal.h"

#include <QTextStream>

D1Pal::~D1Pal()
{
    delete[] colors;
}

bool D1Pal::load(QString filePath)
{
    if (!QFile::exists(filePath))
        return false;

    QFile file = QFile(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    // Check if the palette is a JASC palette
    if (file.read(8) == QString("JASC-PAL").toUtf8())
        this->type = D1PAL_TYPE::JASC;
    else
        this->type = D1PAL_TYPE::REGULAR;
    file.seek(0);

    bool result;
    if (this->type == D1PAL_TYPE::REGULAR) {
        result = this->loadRegularPalette(file);
    } else {
        result = this->loadJascPalette(file);
    }

    this->palFilePath = filePath;
    return result;
}

bool D1Pal::loadRegularPalette(QFile &file)
{
    if (file.size() != D1PAL_SIZE_BYTES)
        return false;

    for (int i = 0; i < D1PAL_COLORS; i++) {
        QByteArray colorBytes = file.read(3);

        quint8 red = colorBytes[0];
        quint8 green = colorBytes[1];
        quint8 blue = colorBytes[2];

        this->colors[i] = QColor(red, green, blue);
    }

    this->modified = false;

    return true;
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

bool D1Pal::save(QString filePath)
{
    QFile file = QFile(filePath);

    if (!file.open(QIODevice::ReadWrite))
        return false;

    for (int i = 0; i < D1PAL_COLORS; i++) {
        QColor color = this->colors[i];

        QByteArray colorBytes;
        colorBytes.resize(3);
        colorBytes[0] = color.red();
        colorBytes[1] = color.green();
        colorBytes[2] = color.blue();

        if (file.write(colorBytes) == -1)
            return false;
    }

    if (!file.flush())
        return false;

    if (file.size() != D1PAL_SIZE_BYTES)
        return false;

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

QColor D1Pal::getColor(quint8 index)
{
    return this->colors[index];
}

void D1Pal::setColor(quint8 index, QColor color)
{
    this->colors[index] = color;
    this->modified = true;
}

void D1Pal::cycleColors(D1PAL_CYCLE_TYPE type)
{
    QColor celColor;
    int i;

    switch (type) {
    case D1PAL_CYCLE_TYPE::CAVES:
        // celColor = this->getColor(1);
        celColor = this->colors[1];
        for (i = 1; i < 31; i++) {
            // this->setColor(i, this->getColor(i + 1));
            this->colors[i] = this->colors[i + 1];
        }
        // this->setColor(i, celColor);
        this->colors[i] = celColor;
        break;
    case D1PAL_CYCLE_TYPE::NEST:
        if (--this->currentCycleCounter != 0)
            break;
        this->currentCycleCounter = 3;
        // celColor = this->getColor(8);
        celColor = this->colors[8];
        for (i = 8; i > 1; i--) {
            // this->setColor(i, this->getColor(i - 1));
            this->colors[i] = this->colors[i - 1];
        }
        // this->setColor(i, celColor);
        this->colors[i] = celColor;

        // celColor = this->getColor(15);
        celColor = this->colors[15];
        for (i = 15; i > 9; i--) {
            // this->setColor(i, this->getColor(i - 1));
            this->colors[i] = this->colors[i - 1];
        }
        // this->setColor(i, celColor);
        this->colors[i] = celColor;
        break;
    case D1PAL_CYCLE_TYPE::CRYPT:
        if (--this->currentCycleCounter == 0) {
            this->currentCycleCounter = 3;

            // celColor = this->getColor(15);
            celColor = this->colors[15];
            for (i = 15; i > 1; i--) {
                // this->setColor(i, this->getColor(i - 1));
                this->colors[i] = this->colors[i - 1];
            }
            // this->setColor(i, celColor);
            this->colors[i] = celColor;
        }

        // celColor = this->getColor(31);
        celColor = this->colors[31];
        for (i = 31; i > 16; i--) {
            // this->setColor(i, this->getColor(i - 1));
            this->colors[i] = this->colors[i - 1];
        }
        // this->setColor(i, celColor);
        this->colors[i] = celColor;
        break;
    }
}
