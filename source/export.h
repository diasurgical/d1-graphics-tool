#ifndef EXPORT_H
#define EXPORT_H

#include <QObject>
#include <QFileInfo>

#include "d1cel.h"
#include "d1cl2.h"
#include "d1min.h"
#include "d1til.h"

const quint16 LEVEL_TILES_PER_LINE = 16;

extern const QMap<QString,QString> TOWN_FILE_PATHS;
extern const QMap<QString,QString> TOWN_HELLFIRE_FILE_PATHS;
extern const QMap<QString,QString> L1_FILE_PATHS;
extern const QMap<QString,QString> L2_FILE_PATHS;
extern const QMap<QString,QString> L3_FILE_PATHS;
extern const QMap<QString,QString> L4_FILE_PATHS;
extern const QMap<QString,QString> L5_FILE_PATHS;
extern const QMap<QString,QString> L6_FILE_PATHS;

extern const QPoint LEVEL_SUBTILES_POSITIONS[4];
extern const QList<quint16> L1S_BLIT_SUBTILE_INDEXES;
extern const QList<quint16> L2S_BLIT_SUBTILE_INDEXES;
extern const QList<quint16> L5S_BLIT_SUBTILE_INDEXES;

extern const QMap<quint16,QList<quint16>> L1_TILES_BLIT_SPECIAL_INDEXES;
extern const QMap<quint16,QList<quint16>> L2_TILES_BLIT_SPECIAL_INDEXES;
extern const QMap<quint16,QList<quint16>> L5_TILES_BLIT_SPECIAL_INDEXES;

class Export : public QObject
{
    Q_OBJECT

public:
    explicit Export(QObject *parent = nullptr);

    static bool checkLevelFilePaths(QString, QMap<QString,QString>);

    static QImage getLevelTileWithSpecial(
        D1Til*, D1Cel*, QList<quint16>, QMap<quint16, QList<quint16>>);
    static QImage getAllLevelTilesWith(QMap<QString,QString>);

signals:

};

#endif // EXPORT_H
