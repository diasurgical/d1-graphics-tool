#include "d1trn.h"

D1Trn::D1Trn(D1Pal *pal)
    : palette(pal)
{
}

bool D1Trn::load(QString filePath)
{
    if (this->palette == nullptr)
        return false;

    QFile file = QFile(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    if (file.size() != D1TRN_TRANSLATIONS)
        return false;

    int readBytes = file.read((char *)this->translations, D1TRN_TRANSLATIONS);
    if (readBytes != D1TRN_TRANSLATIONS)
        return false;

    this->refreshResultingPalette();

    this->trnFilePath = filePath;
    this->modified = false;
    return true;
}

bool D1Trn::save(QString filePath)
{
    QFile file = QFile(filePath);

    if (!file.open(QIODevice::WriteOnly))
        return false;

    int outBytes = file.write((char *)this->translations, D1TRN_TRANSLATIONS);
    if (outBytes != D1TRN_TRANSLATIONS)
        return false;

    if (this->trnFilePath == filePath) {
        this->modified = false;
    } else {
        // -- do not update, the user is creating a new one and the original needs to be preserved
        // this->modified = false;
        // this->trnFilePath = filePath;
    }
    return true;
}

bool D1Trn::isModified() const
{
    return this->modified;
}

void D1Trn::refreshResultingPalette()
{
    for (int i = 0; i < D1TRN_TRANSLATIONS; i++) {
        this->resultingPalette.setColor(
            i, this->palette->getColor(this->translations[i]));
    }
}

QColor D1Trn::getResultingColor(quint8 index)
{
    return this->resultingPalette.getColor(index);
}

QString D1Trn::getFilePath()
{
    return this->trnFilePath;
}

QString D1Trn::getDefaultPath() const
{
    return IDENTITY_PATH;
}

QString D1Trn::getDefaultName() const
{
    return IDENTITY_NAME;
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

void D1Trn::setPalette(D1Pal *pal)
{
    this->palette = pal;
}

D1Pal *D1Trn::getResultingPalette()
{
    return &this->resultingPalette;
}
