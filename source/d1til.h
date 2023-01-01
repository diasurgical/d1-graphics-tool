#pragma once

#include <QImage>

#include "d1min.h"
#include "saveasdialog.h"

enum class D1TIL_TYPE {
    REGULAR_HEIGHT,
    EXTENDED_HEIGHT
};

class D1Til : public QObject {
    Q_OBJECT

public:
    D1Til() = default;
    ~D1Til() = default;

    bool load(QString filePath);
    bool save(SaveAsParam *params = nullptr);

    QImage getTileImage(quint16);

    D1TIL_TYPE getType();
    QString getFilePath();
    void setMin(D1Min *m);
    int getTileCount();
    quint16 getTileWidth();
    quint16 getTileHeight();
    quint16 getTilePixelWidth();
    quint16 getTilePixelHeight();
    QList<quint16> getSubtileIndices(int tileIndex);

private:
    D1TIL_TYPE type = D1TIL_TYPE::REGULAR_HEIGHT;
    QString tilFilePath;
    D1Min *min = nullptr;
    int tileCount = 0;
    quint16 tileWidth = 0;
    quint16 tileHeight = 0;
    quint16 tilePixelWidth = 0;
    quint16 tilePixelHeight = 0;
    QList<QList<quint16>> subtileIndices;
};
