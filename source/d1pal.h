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

class D1Pal : public QObject {
    Q_OBJECT

public:
    static constexpr const char *DEFAULT_PATH = ":/default.pal";
    static constexpr const char *DEFAULT_NAME = "_default.pal";

    D1Pal() = default;
    ~D1Pal();

    bool load(QString);
    bool loadRegularPalette();
    bool loadJascPalette();
    bool save(QString);

    bool isModified() const;

    QString getFilePath();

    QColor getColor(quint8);
    void setColor(quint8, QColor);

private:
    D1PAL_TYPE type = D1PAL_TYPE::REGULAR;
    bool modified = false;
    QFile file;
    QColor *colors = new QColor[D1PAL_COLORS];
};
