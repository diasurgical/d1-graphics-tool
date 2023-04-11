#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

class D1Sol : public QObject {
    Q_OBJECT

public:
    D1Sol() = default;
    ~D1Sol() = default;

    bool load(QString filePath);
    bool save(const QString &gfxPath);

    void insertSubtile(int subtileIndex, quint8 value);
    void createSubtile();
    void removeSubtile(int subtileIndex);
    void remapSubtiles(const QMap<unsigned, unsigned> &remap);

    bool isModified() const;
    QString getFilePath();
    quint16 getSubtileCount();
    quint8 getSubtileProperties(int subtileIndex);
    void setSubtileProperties(int subtileIndex, quint8 value);

private:
    bool modified;
    QString solFilePath;
    QList<quint8> subProperties;
};
