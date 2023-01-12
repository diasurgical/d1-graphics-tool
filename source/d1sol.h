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

    void insertSubtile(int subtileIndex, quint8 value);
    void createSubtile();
    void removeSubtile(int subtileIndex);

    QString getFilePath();
    quint16 getSubtileCount();
    quint8 getSubtileProperties(int subtileIndex);
    void setSubtileProperties(int subtileIndex, quint8 value);

private:
    QString solFilePath;
    QList<quint8> subProperties;
};
