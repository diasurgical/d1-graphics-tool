#include "export.h"

const QMap<QString,QString> TOWN_FILE_PATHS =
{
    {"cel","levels/towndata/town.cel"},
    {"min","levels/towndata/town.min"},
    {"til","levels/towndata/town.til"},
    {"pal","levels/towndata/town.pal"}
};

// town.pal is not in nlevels, it's in the Diablo levels folder
const QMap<QString,QString> TOWN_HELLFIRE_FILE_PATHS =
{
    {"cel","nlevels/towndata/town.cel"},
    {"min","nlevels/towndata/town.min"},
    {"til","nlevels/towndata/town.til"},
    {"pal","levels/towndata/town.pal"}
};

const QMap<QString,QString> L1_FILE_PATHS =
{
    {"cel","levels/l1data/l1.cel"},
    {"special_cel","levels/l1data/l1s.cel"},
    {"min","levels/l1data/l1.min"},
    {"til","levels/l1data/l1.til"},
    {"pal","levels/l1data/l1.pal"}
};

const QMap<QString,QString> L2_FILE_PATHS =
{
    {"cel","levels/l2data/l2.cel"},
    {"special_cel","levels/l2data/l2s.cel"},
    {"min","levels/l2data/l2.min"},
    {"til","levels/l2data/l2.til"},
    {"pal","levels/l2data/l2.pal"}
};

const QMap<QString,QString> L3_FILE_PATHS =
{
    {"cel","levels/l3data/l3.cel"},
    {"min","levels/l3data/l3.min"},
    {"til","levels/l3data/l3.til"},
    {"pal","levels/l3data/l3.pal"}
};

const QMap<QString,QString> L4_FILE_PATHS =
{
    {"cel","levels/l4data/l4.cel"},
    {"min","levels/l4data/l4.min"},
    {"til","levels/l4data/l4.til"},
    {"pal","levels/l4data/l4_1.pal"}
};

const QMap<QString,QString> L5_FILE_PATHS =
{
    {"cel","nlevels/l5data/l5.cel"},
    {"special_cel","nlevels/l5data/l5s.cel"},
    {"min","nlevels/l5data/l5.min"},
    {"til","nlevels/l5data/l5.til"},
    {"pal","nlevels/l5data/l5base.pal"}
};

const QMap<QString,QString> L6_FILE_PATHS =
{
    {"cel","nlevels/l6data/l6.cel"},
    {"min","nlevels/l6data/l6.min"},
    {"til","nlevels/l6data/l6.til"},
    {"pal","nlevels/l6data/l6base.pal"}
};


// In a tile, subtiles are layed out as follow:
//    3
//  2   1
//    0
// The following array contains the position (from bottom left corner)
// of the four subtiles in the tile
const QPoint LEVEL_SUBTILES_POSITIONS[4] =
{
    QPoint(32,0),
    QPoint(64,16),
    QPoint(0,16),
    QPoint(32,32)
};

// In a tile, subtiles are layed out as follow:
//    3
//  2   1
//    0
// Each index in this array is a l1s.cel frame index
// Each value is the corresponding subtile position for the frame in the tile
const QList<quint16> L1S_BLIT_SUBTILE_INDEXES =
{
    2, 1, 1, 2, 2, 2, 2, 1
};

const QList<quint16> L2S_BLIT_SUBTILE_INDEXES =
{
    2, 2, 1, 1, 2, 1
};

const QList<quint16> L5S_BLIT_SUBTILE_INDEXES =
{
    2, 1
};

// First quint16 is the tile index
// Then the QList contains the L1S frame index(es) to blit into the tile
const QMap<quint16,QList<quint16>> L1_TILES_BLIT_SPECIAL_INDEXES =
{
    {4, {0, 1}},
    {7, {0}}
};

const QMap<quint16,QList<quint16>> L2_TILES_BLIT_SPECIAL_INDEXES =
{
};

const QMap<quint16,QList<quint16>> L5_TILES_BLIT_SPECIAL_INDEXES =
{
};

Export::Export(QObject *parent) : QObject(parent)
{}

bool Export::checkLevelFilePaths(QString rootPath, QMap<QString,QString> filePaths)
{
    QString absoluteFilePath;

    absoluteFilePath = rootPath + "/" + filePaths.value("cel");
    if(!QFileInfo::exists(absoluteFilePath))
        return false;

    if(filePaths.contains("special_cel"))
    {
        absoluteFilePath = rootPath + "/" + filePaths.value("special_cel");
        if(!QFileInfo::exists(absoluteFilePath))
            return false;
    }

    absoluteFilePath = rootPath + "/" + filePaths.value("min");
    if(!QFileInfo::exists(absoluteFilePath))
        return false;

    absoluteFilePath = rootPath + "/" + filePaths.value("til");
    if(!QFileInfo::exists(absoluteFilePath))
        return false;

    absoluteFilePath = rootPath + "/" + filePaths.value("pal");
    if(!QFileInfo::exists(absoluteFilePath))
        return false;

    return true;
}

QImage Export::getLevelTileWithSpecial(
    D1Til* til, D1Cel* specialCel, QList<quint16> subtileIndexes, QMap<quint16,QList<quint16>> tileBlitSpecialIndexes)
{

    return QImage();
}



