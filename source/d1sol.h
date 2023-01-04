#pragma once

#include <QList>

#include "saveasdialog.h"

class D1Sol : public QObject {
    Q_OBJECT

public:
    D1Sol() = default;
    ~D1Sol() = default;

    bool load(QString filePath);
    bool save(const SaveAsParam &params);

    QString getFilePath();
    quint16 getSubtileCount();
    quint8 getSubtileProperties(int subtileIndex);
    void setSubtileProperties(int subtileIndex, quint8 value);
    void removeSubtile(int subtileIndex);

private:
    QString solFilePath;
    QList<quint8> subProperties;
};
