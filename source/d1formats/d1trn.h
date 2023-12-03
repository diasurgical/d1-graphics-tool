#pragma once

#include <QPointer>

#include "palette/d1pal.h"

#define D1TRN_TRANSLATIONS 256

class D1Trn : public QObject {
    Q_OBJECT

public:
    static constexpr const char *IDENTITY_PATH = ":/null.trn";
    static constexpr const char *IDENTITY_NAME = "_null.trn";

    D1Trn() = default;
    D1Trn(D1Pal *pal);
    ~D1Trn() = default;

    bool load(QString);
    bool save(QString);

    bool isModified() const;

    void refreshResultingPalette();
    QColor getResultingColor(quint8);

    QString getFilePath();
    quint8 getTranslation(quint8);
    void setTranslation(quint8, quint8);
    void setPalette(D1Pal *pal);
    D1Pal *getResultingPalette();

private:
    QString trnFilePath;
    bool modified;
    quint8 translations[D1TRN_TRANSLATIONS];
    QPointer<D1Pal> palette;
    D1Pal resultingPalette;
};
