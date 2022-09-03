#pragma once

#include <QPointer>

#include "d1pal.h"

#define D1TRN_TRANSLATIONS 256
#define D1TRN_TRANSLATIONS_BYTES 256

class D1Trn : public QObject {
    Q_OBJECT

public:
    D1Trn();
    D1Trn(QString, D1Pal *);
    ~D1Trn();

    bool load(QString);
    bool save(QString);

    bool isModified();

    void refreshResultingPalette();
    QColor getResultingColor(quint8);

    QString getFilePath();
    quint8 getTranslation(quint8);
    void setTranslation(quint8, quint8);
    D1Pal *getPalette();
    void setPalette(D1Pal *);
    D1Pal *getResultingPalette();

private:
    bool modified;
    QFile file;
    quint8 *translations;
    QPointer<D1Pal> palette;
    QPointer<D1Pal> resultingPalette;
};
