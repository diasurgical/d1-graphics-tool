#pragma once

#include <QColor>
#include <QFile>
#include <QObject>
#include <QString>

#define D1PAL_COLORS 256
#define D1PAL_COLOR_BITS 8
#define D1PAL_SIZE_BYTES 768

enum class D1PAL_TYPE {
    REGULAR,
    JASC
};

enum class D1PAL_CYCLE_TYPE {
    CAVES,
    HELL,
    CRYPT,
    NEST,
};

class D1Pal : public QObject {
    Q_OBJECT

public:
    static constexpr const char *DEFAULT_PATH = ":/default.pal";
    static constexpr const char *DEFAULT_NAME = "_default.pal";

    D1Pal() = default;
    ~D1Pal() override = default;

    virtual bool load(QString);
    virtual bool save(QString);

    [[nodiscard]] virtual bool isModified() const;

    virtual QString getFilePath();

    [[nodiscard]] virtual QString getDefaultPath() const;
    [[nodiscard]] virtual QString getDefaultName() const;

    QColor getColor(quint8);
    void setColor(quint8, QColor);

    void resetColors();
    void cycleColors(D1PAL_CYCLE_TYPE type);

private:
    void loadRegularPalette(QFile &file);
    bool loadJascPalette(QFile &file);

private:
    QString palFilePath;
    bool modified;
    QColor colors[D1PAL_COLORS];
    quint8 currentCycleCounter = 3;
    // buffer to store the original colors in case of color cycling
    QColor origCyclePalette[32];
};
