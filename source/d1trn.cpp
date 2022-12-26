#include "d1trn.h"

D1Trn::D1Trn(D1Pal *pal)
    : palette(QPointer<D1Pal>(pal))
{
}

D1Trn::~D1Trn()
{
    delete[] translations;
    delete resultingPalette;
}

bool D1Trn::load(QString filePath)
{
    if (this->palette.isNull())
        return false;

    if (!QFile::exists(filePath))
        return false;

    QFile file = QFile(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    if (file.size() != D1TRN_TRANSLATIONS_BYTES)
        return false;

    for (int i = 0; i < D1TRN_TRANSLATIONS; i++) {
        QByteArray translationByte = file.read(1);
        this->translations[i] = translationByte[0];
        this->resultingPalette->setColor(
            i, this->palette->getColor(this->translations[i]));
    }

    this->modified = false;
    this->trnFilePath = filePath;
    return true;
}

bool D1Trn::save(QString filePath)
{
    QFile file = QFile(filePath);

    if (!file.open(QIODevice::ReadWrite))
        return false;

    for (int i = 0; i < D1TRN_TRANSLATIONS; i++) {
        QByteArray colorBytes;
        colorBytes.resize(1);
        colorBytes[0] = this->translations[i];

        if (file.write(colorBytes) == -1)
            return false;
    }

    if (!file.flush())
        return false;

    if (file.size() != D1TRN_TRANSLATIONS_BYTES)
        return false;

    this->modified = false;
    this->trnFilePath = filePath;
    return true;
}

bool D1Trn::isModified() const
{
    return this->modified;
}

void D1Trn::refreshResultingPalette()
{
    for (int i = 0; i < D1TRN_TRANSLATIONS; i++) {
        this->resultingPalette->setColor(
            i, this->palette->getColor(this->translations[i]));
    }
}

QColor D1Trn::getResultingColor(quint8 index)
{
    return this->resultingPalette->getColor(index);
}

QString D1Trn::getFilePath()
{
    return this->trnFilePath;
}

quint8 D1Trn::getTranslation(quint8 index)
{
    return this->translations[index];
}

void D1Trn::setTranslation(quint8 index, quint8 translation)
{
    this->translations[index] = translation;

    this->modified = true;
}

D1Pal *D1Trn::getPalette()
{
    return this->palette;
}

void D1Trn::setPalette(D1Pal *pal)
{
    this->palette = pal;
}

D1Pal *D1Trn::getResultingPalette()
{
    return this->resultingPalette;
}
