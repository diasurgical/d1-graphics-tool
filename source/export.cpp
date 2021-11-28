#include "export.h"

const QMap<QString,QString> TOWN_FILE_PATHS =
{
    {"pal","levels/towndata/town.pal"},
    {"cel","levels/towndata/town.cel"},
    {"min","levels/towndata/town.min"},
    {"til","levels/towndata/town.til"},
    {"png","town.png"}
};

// town.pal is not in nlevels, it's in the Diablo levels folder
const QMap<QString,QString> TOWN_HELLFIRE_FILE_PATHS =
{
    {"pal","levels/towndata/town.pal"},
    {"cel","nlevels/towndata/town.cel"},
    {"min","nlevels/towndata/town.min"},
    {"til","nlevels/towndata/town.til"},
    {"png","town.png"}
};

const QMap<QString,QString> L1_FILE_PATHS =
{
    {"pal","levels/l1data/l1.pal"},
    {"cel","levels/l1data/l1.cel"},
    {"special_cel","levels/l1data/l1s.cel"},
    {"min","levels/l1data/l1.min"},
    {"til","levels/l1data/l1.til"},
    {"png","l1.png"}
};

const QMap<QString,QString> L2_FILE_PATHS =
{
    {"pal","levels/l2data/l2.pal"},
    {"cel","levels/l2data/l2.cel"},
    {"special_cel","levels/l2data/l2s.cel"},
    {"min","levels/l2data/l2.min"},
    {"til","levels/l2data/l2.til"},
    {"png","l2.png"}
};

const QMap<QString,QString> L3_FILE_PATHS =
{
    {"pal","levels/l3data/l3.pal"},
    {"cel","levels/l3data/l3.cel"},
    {"min","levels/l3data/l3.min"},
    {"til","levels/l3data/l3.til"},
    {"png","l3.png"}
};

const QMap<QString,QString> L4_FILE_PATHS =
{
    {"pal","levels/l4data/l4_1.pal"},
    {"cel","levels/l4data/l4.cel"},
    {"min","levels/l4data/l4.min"},
    {"til","levels/l4data/l4.til"},
    {"png","l4.png"}
};

const QMap<QString,QString> L5_FILE_PATHS =
{
    {"pal","nlevels/l5data/l5base.pal"},
    {"cel","nlevels/l5data/l5.cel"},
    {"special_cel","nlevels/l5data/l5s.cel"},
    {"min","nlevels/l5data/l5.min"},
    {"til","nlevels/l5data/l5.til"},
    {"png","l5.png"}
};

const QMap<QString,QString> L6_FILE_PATHS =
{
    {"pal","nlevels/l6data/l6base.pal"},
    {"cel","nlevels/l6data/l6.cel"},
    {"min","nlevels/l6data/l6.min"},
    {"til","nlevels/l6data/l6.til"},
    {"png","l6.png"}
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
    {4,   {0, 1}},
    {7,   {0}},
    {8,   {1}},
    {9,   {1}},
    {10,  {0}},
    {11,  {1}},
    {13,  {0}},
    {37,  {1}},
    {39,  {1}},
    {41,  {0}},
    {43,  {0}},
    {101, {0}},
    {117, {1}},
    {119, {2}},
    {120, {3}},
    {122, {4}},
    {125, {5}},
    {146, {0}},
    {148, {1}},
    {152, {1}},
    {158, {0}},
    {160, {1}},
    {174, {6}},
    {175, {7}},
    {176, {6, 7}},
    {177, {6}},
    {178, {7}},
    {179, {6}},
    {180, {7}},
    {181, {6,1}},
    {182, {6}},
    {183, {0,7}},
    {184, {7}},
    {185, {6}},
    {186, {6}},
    {187, {7}},
    {188, {6}},
    {189, {7}},
    {192, {6}},
    {193, {7}},
    {196, {6}},
    {197, {7}}
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

QImage Export::getAllLevelTilesWithSpecials(
    QString rootPath, QMap<QString, QString> filePaths,
    QList<quint16> specialCelBlitSubtileIndexes, QMap<quint16, QList<quint16>> tilesBlitSpecialIndexes)
{
    D1Pal* pal = NULL;
    D1Cel* cel = NULL;
    D1Cel* specialCel = NULL;
    D1Min* min = NULL;
    D1Til* til = NULL;

    // Load level files
    pal = new D1Pal(rootPath + "/" + filePaths["pal"]);
    cel = new D1Cel(rootPath + "/" + filePaths["cel"], pal);
    if(filePaths.contains("special_cel"))
        specialCel = new D1Cel(rootPath + "/" + filePaths["special_cel"], pal);
    min = new D1Min(rootPath + "/" + filePaths["min"], cel);
    til = new D1Til(rootPath + "/" + filePaths["til"], min);

    // Initialize output image
    quint16 tileWidth = til->getTilePixelWidth();
    quint16 tileHeight = til->getTilePixelHeight();
    quint16 tileCount = til->getTileCount();

    quint32 outputImageWidth = tileWidth * LEVEL_TILES_PER_LINE;
    quint32 outputImageHeight = tileHeight * (quint32)(tileCount/LEVEL_TILES_PER_LINE);
    if(tileCount%LEVEL_TILES_PER_LINE != 0 )
        outputImageHeight += tileHeight;
    QImage outputImage = QImage( outputImageWidth, outputImageHeight, QImage::Format_ARGB32 );
    outputImage.fill( Qt::transparent );

    // Loop through tiles
    QPainter painter(&outputImage);
    quint8 tileXIndex = 0;
    quint8 tileYIndex = 0;
    for(unsigned int i = 0; i < tileCount; i++)
    {
        // Get tile image, blit special CEL frames if necessary
        QImage t;
        if(filePaths.contains("special_cel") && tilesBlitSpecialIndexes.contains(i))
            t = getLevelTileWithSpecials(
                til, i, specialCel, &specialCelBlitSubtileIndexes, &tilesBlitSpecialIndexes[i]);
        else
            t = til->getTileImage(i);

        painter.drawImage(tileXIndex*tileWidth, tileYIndex*tileHeight, t);

        tileXIndex++;
        if(tileXIndex >= LEVEL_TILES_PER_LINE)
        {
            tileXIndex = 0;
            tileYIndex++;
        }
    }
    painter.end();

    // Cleanup
    delete pal;
    delete cel;
    if(filePaths.contains("special_cel"))
        delete specialCel;
    delete min;
    delete til;

    return outputImage;
}

QImage Export::getLevelTileWithSpecials(
    D1Til* til, quint16 tileIndex,
    D1Cel* specialCel, QList<quint16>* specialCelBlitSubtileIndexes, QList<quint16>* tileBlitSpecialIndexes)
{
    QImage tileImage = til->getTileImage(tileIndex);
    QPainter tilePainter( &tileImage );

    // Blit special CEL frame in tile at the right (subtile) position
    for(int i = 0; i < tileBlitSpecialIndexes->length(); i++)
    {
        quint16 specialCelFrameIndex = tileBlitSpecialIndexes->at(i);
        QImage specialCelFrame = specialCel->getFrameImage(specialCelFrameIndex);

        quint16 subtileIndex = specialCelBlitSubtileIndexes->at(specialCelFrameIndex);
        QPoint blitCoordinates = LEVEL_SUBTILES_POSITIONS[subtileIndex];

        tilePainter.drawImage(blitCoordinates.x(), blitCoordinates.y(), specialCelFrame);
    }

    tilePainter.end();

    return tileImage;
}

