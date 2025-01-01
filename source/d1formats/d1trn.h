#pragma once

#include <QPointer>

#include "palette/d1pal.h"

#define D1TRN_TRANSLATIONS 256

class D1Trn final : public D1Pal {
    Q_OBJECT

public:
    static constexpr const char *IDENTITY_PATH = ":/null.trn";
    static constexpr const char *IDENTITY_NAME = "_null.trn";

    D1Trn() = default;
    D1Trn(D1Pal *pal);
    ~D1Trn() = default;

    bool load(QString filepath) override;
    bool save(QString filepath) override;

    [[nodiscard]] bool isModified() const override;

    void refreshResultingPalette();
    QColor getResultingColor(quint8);

    QString getFilePath() override;

    [[nodiscard]] QString getDefaultPath() const override;
    [[nodiscard]] QString getDefaultName() const override;

    quint8 getTranslation(quint8);
    void setTranslation(quint8, quint8);
    void setPalette(D1Pal *pal);
    D1Pal *getResultingPalette();

private:
    QString trnFilePath;
    bool modified;
    quint8 translations[D1TRN_TRANSLATIONS];
    D1Pal *palette = nullptr;
    D1Pal resultingPalette;
};
