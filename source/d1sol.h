#pragma once

#include <QList>

#include "saveasdialog.h"

class D1Sol : public QObject {
    Q_OBJECT

public:
    D1Sol() = default;
    ~D1Sol() = default;

    bool load(QString filePath);
    bool save(SaveAsParam *params = nullptr);

    QString getFilePath();
    quint16 getSubtileCount();
    quint8 getSubtileProperties(quint16);
    void setSubtileProperties(quint16 tileIndex, quint8 value);

private:
    QString solFilePath;
    QList<quint8> subProperties;
};
