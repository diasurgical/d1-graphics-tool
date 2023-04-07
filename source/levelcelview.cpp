#include "levelcelview.h"

#include <algorithm>
#include <set>

#include <QAction>
#include <QDebug>
#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QImageReader>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>

#include "d1image.h"
#include "mainwindow.h"
#include "ui_levelcelview.h"

LevelCelView::LevelCelView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LevelCelView())
    , celScene(new CelScene(this))
{
    this->ui->setupUi(this);
    this->ui->celGraphicsView->setScene(this->celScene);
    this->ui->zoomEdit->setText(QString::number(this->currentZoomFactor));
    this->ui->playDelayEdit->setText(QString::number(this->currentPlayDelay));
    this->ui->stopButton->setEnabled(false);
    this->playTimer.connect(&this->playTimer, SIGNAL(timeout()), this, SLOT(playGroup()));
    this->ui->tilesTabs->addTab(this->tabTileWidget, "Tile properties");
    this->ui->tilesTabs->addTab(this->tabSubTileWidget, "Subtile properties");
    this->ui->tilesTabs->addTab(this->tabFrameWidget, "Frame properties");

    // If a pixel of the frame, subtile or tile was clicked get pixel color index and notify the palette widgets
    QObject::connect(this->celScene, &CelScene::framePixelClicked, this, &LevelCelView::framePixelClicked);

    // setup context menu
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowContextMenu(const QPoint &)));
    QObject::connect(this->celScene, &CelScene::showContextMenu, this, &LevelCelView::ShowContextMenu);

    setAcceptDrops(true);
}

LevelCelView::~LevelCelView()
{
    delete ui;
    delete celScene;
    delete tabTileWidget;
    delete tabSubTileWidget;
    delete tabFrameWidget;
}

void LevelCelView::initialize(D1Gfx *g, D1Min *m, D1Til *t, D1Sol *s, D1Amp *a)
{
    this->gfx = g;
    this->min = m;
    this->til = t;
    this->sol = s;
    this->amp = a;
    this->update();
}

void LevelCelView::update()
{
    // Displaying CEL file path information
    QFileInfo gfxFileInfo(this->gfx->getFilePath());
    QFileInfo minFileInfo(this->min->getFilePath());
    QFileInfo tilFileInfo(this->til->getFilePath());
    QFileInfo solFileInfo(this->sol->getFilePath());
    QFileInfo ampFileInfo(this->amp->getFilePath());
    ui->celLabel->setText(gfxFileInfo.fileName() + ", " + minFileInfo.fileName() + ", " + tilFileInfo.fileName() + ", " + solFileInfo.fileName() + ", " + ampFileInfo.fileName());

    if (this->mode == TILESET_MODE::FREE)
        ui->modeLabel->setText("");
    else if (this->mode == TILESET_MODE::SUBTILE)
        ui->modeLabel->setText("Subtile mode");
    else if (this->mode == TILESET_MODE::TILE)
        ui->modeLabel->setText("Tile mode");

    ui->frameNumberEdit->setText(
        QString::number(this->gfx->getFrameCount()));

    ui->subtileNumberEdit->setText(
        QString::number(this->min->getSubtileCount()));

    ui->tileNumberEdit->setText(
        QString::number(this->til->getTileCount()));

    this->tabTileWidget->initialize(this, this->til, this->min, this->amp);
    this->tabSubTileWidget->initialize(this, this->gfx, this->min, this->sol);
    this->tabFrameWidget->initialize(this, this->gfx);
}

int LevelCelView::getCurrentFrameIndex()
{
    return this->currentFrameIndex;
}

int LevelCelView::getCurrentSubtileIndex()
{
    return this->currentSubtileIndex;
}

int LevelCelView::getCurrentTileIndex()
{
    return this->currentTileIndex;
}

void LevelCelView::framePixelClicked(unsigned x, unsigned y)
{
    quint8 index = 0;

    unsigned celFrameWidth = MICRO_WIDTH; // this->gfx->getFrameWidth(this->currentFrameIndex);
    unsigned subtileWidth = this->min->getSubtileWidth() * MICRO_WIDTH;
    unsigned tileWidth = subtileWidth * TILE_WIDTH;

    unsigned celFrameHeight = MICRO_HEIGHT; // this->gfx->getFrameHeight(this->currentFrameIndex);
    unsigned subtileHeight = this->min->getSubtileHeight() * MICRO_HEIGHT;
    unsigned subtileShiftY = subtileWidth / 4;
    unsigned tileHeight = subtileHeight + 2 * subtileShiftY;

    this->mode = TILESET_MODE::FREE;

    if (x >= CEL_SCENE_SPACING && x < (celFrameWidth + CEL_SCENE_SPACING)
        && y >= CEL_SCENE_SPACING && y < (celFrameHeight + CEL_SCENE_SPACING)
        && this->gfx->getFrameCount() != 0) {
        // If CEL frame color is clicked, select it in the palette widgets
        D1GfxFrame *frame = this->gfx->getFrame(this->currentFrameIndex);
        index = frame->getPixel(x - CEL_SCENE_SPACING, y - CEL_SCENE_SPACING).getPaletteIndex();

        emit this->colorIndexClicked(index);
    } else if (x >= (celFrameWidth + CEL_SCENE_SPACING * 2)
        && x < (celFrameWidth + subtileWidth + CEL_SCENE_SPACING * 2)
        && y >= CEL_SCENE_SPACING
        && y < (subtileHeight + CEL_SCENE_SPACING)
        && this->min->getSubtileCount() != 0) {
        this->mode = TILESET_MODE::SUBTILE;
        // When a CEL frame is clicked in the subtile, display the corresponding CEL frame

        // Adjust coordinates
        unsigned stx = x - (celFrameWidth + CEL_SCENE_SPACING * 2);
        unsigned sty = y - CEL_SCENE_SPACING;

        // qDebug() << "Subtile clicked: " << stx << "," << sty;

        int stFrame = (sty / MICRO_HEIGHT) * TILE_WIDTH + (stx / MICRO_WIDTH);
        this->editIndex = stFrame;
        QList<quint16> &minFrames = this->min->getCelFrameIndices(this->currentSubtileIndex);
        quint16 frameIndex = minFrames.count() > stFrame ? minFrames.at(stFrame) : 0;

        if (frameIndex > 0) {
            this->currentFrameIndex = frameIndex - 1;
            this->displayFrame();
        }
    } else if (x >= (celFrameWidth + subtileWidth + CEL_SCENE_SPACING * 3)
        && x < (celFrameWidth + subtileWidth + tileWidth + CEL_SCENE_SPACING * 3)
        && y >= CEL_SCENE_SPACING
        && y < (tileHeight + CEL_SCENE_SPACING)
        && this->til->getTileCount() != 0) {
        this->mode = TILESET_MODE::TILE;
        // When a subtile is clicked in the tile, display the corresponding subtile

        // Adjust coordinates
        unsigned tx = x - (celFrameWidth + subtileWidth + CEL_SCENE_SPACING * 3);
        unsigned ty = y - CEL_SCENE_SPACING;

        // qDebug() << "Tile clicked" << tx << "," << ty;

        // Split the area based on the ground squares
        // f(x)\ 0 /
        //      \ /
        //   2   *   1
        //      / \
        // g(x)/ 3 \
        //
        // f(x) = (tileHeight - 2 * subtileShiftY) + 0.5x
        unsigned ftx = (tileHeight - 2 * subtileShiftY) + tx / 2;
        // g(tx) = tileHeight - 0.5x
        unsigned gtx = tileHeight - tx / 2;
        // qDebug() << "fx=" << ftx << ", gx=" << gtx;
        int tSubtile = 0;
        if (ty < ftx) {
            if (ty < gtx) {
                // limit the area of 0 horizontally
                if (tx < subtileWidth / 2)
                    tSubtile = 2;
                else if (tx >= (subtileWidth / 2 + subtileWidth))
                    tSubtile = 1;
                else
                    tSubtile = 0;
            } else {
                tSubtile = 1;
            }
        } else {
            if (ty < gtx)
                tSubtile = 2;
            else
                tSubtile = 3;
        }
        this->editIndex = tSubtile;

        QList<quint16> &tilSubtiles = this->til->getSubtileIndices(this->currentTileIndex);
        if (tilSubtiles.count() > tSubtile) {
            this->currentSubtileIndex = tilSubtiles.at(tSubtile);
            this->displayFrame();
        }
    }

    this->update();
}

void LevelCelView::insertImageFiles(IMAGE_FILE_MODE mode, const QStringList &imagefilePaths, bool append)
{
    if (mode == IMAGE_FILE_MODE::FRAME || mode == IMAGE_FILE_MODE::AUTO) {
        this->insertFrames(mode, imagefilePaths, append);
    }
    if (mode == IMAGE_FILE_MODE::SUBTILE || mode == IMAGE_FILE_MODE::AUTO) {
        this->insertSubtiles(mode, imagefilePaths, append);
    }
    if (mode == IMAGE_FILE_MODE::TILE || mode == IMAGE_FILE_MODE::AUTO) {
        this->insertTiles(mode, imagefilePaths, append);
    }
}

void LevelCelView::assignFrames(const QImage &image, int subtileIndex, int frameIndex)
{
    QList<quint16> frameIndicesList;

    // TODO: merge with LevelCelView::insertSubtile ?
    QImage subImage = QImage(MICRO_WIDTH, MICRO_HEIGHT, QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); y += MICRO_HEIGHT) {
        for (int x = 0; x < image.width(); x += MICRO_WIDTH) {
            // subImage.fill(Qt::transparent);

            bool hasColor = false;
            for (int j = 0; j < MICRO_HEIGHT; j++) {
                for (int i = 0; i < MICRO_WIDTH; i++) {
                    const QColor color = image.pixelColor(x + i, y + j);
                    if (color.alpha() >= COLOR_ALPHA_LIMIT) {
                        hasColor = true;
                    }
                    subImage.setPixelColor(i, j, color);
                }
            }
            frameIndicesList.append(hasColor ? frameIndex + 1 : 0);
            if (!hasColor) {
                continue;
            }

            D1GfxFrame *frame = this->gfx->insertFrame(frameIndex, subImage);
            LevelTabFrameWidget::selectFrameType(frame);
            frameIndex++;
        }
    }

    if (subtileIndex >= 0) {
        this->min->getCelFrameIndices(subtileIndex).swap(frameIndicesList);
        // reset subtile flags
        this->sol->setSubtileProperties(subtileIndex, 0);
    }
}

void LevelCelView::insertFrames(IMAGE_FILE_MODE mode, int index, const QImage &image)
{
    if ((image.width() % MICRO_WIDTH) != 0 || (image.height() % MICRO_HEIGHT) != 0) {
        return;
    }

    if (mode == IMAGE_FILE_MODE::AUTO) {
        // check for subtile dimensions to be more lenient than EXPORT_LVLFRAMES_PER_LINE
        unsigned subtileWidth = this->min->getSubtileWidth() * MICRO_WIDTH;
        unsigned subtileHeight = this->min->getSubtileHeight() * MICRO_HEIGHT;

        if ((image.width() % subtileWidth) == 0 && (image.height() % subtileHeight) == 0) {
            return; // this is a subtile or a tile (or subtiles or tiles) -> ignore
        }
    }

    this->assignFrames(image, -1, index);
}

void LevelCelView::insertFrames(IMAGE_FILE_MODE mode, int index, const QString &imagefilePath)
{
    QImageReader reader = QImageReader(imagefilePath);
    int numImages = 0;

    while (true) {
        QImage image = reader.read();
        if (image.isNull()) {
            break;
        }
        this->insertFrames(mode, index + numImages, image);
        numImages++;
    }

    if (mode != IMAGE_FILE_MODE::AUTO && numImages == 0) {
        QMessageBox::critical(this, "Error", "Failed read image file: " + imagefilePath);
    }
}

void LevelCelView::insertFrames(IMAGE_FILE_MODE mode, const QStringList &imagefilePaths, bool append)
{
    int prevFrameCount = this->gfx->getFrameCount();

    if (append) {
        // append the frame(s)
        for (int i = 0; i < imagefilePaths.count(); i++) {
            this->insertFrames(mode, this->gfx->getFrameCount(), imagefilePaths[i]);
        }
        int deltaFrameCount = this->gfx->getFrameCount() - prevFrameCount;
        if (deltaFrameCount == 0) {
            return; // no new frame -> done
        }
        // jump to the first appended frame
        this->currentFrameIndex = prevFrameCount;
    } else {
        // insert the frame(s)
        for (int i = imagefilePaths.count() - 1; i >= 0; i--) {
            this->insertFrames(mode, this->currentFrameIndex, imagefilePaths[i]);
        }
        int deltaFrameCount = this->gfx->getFrameCount() - prevFrameCount;
        if (deltaFrameCount == 0) {
            return; // no new frame -> done
        }
        // shift references
        unsigned refIndex = this->currentFrameIndex + 1;
        // shift frame indices of the subtiles
        for (int i = 0; i < this->min->getSubtileCount(); i++) {
            QList<quint16> &frameIndices = this->min->getCelFrameIndices(i);
            for (int n = 0; n < frameIndices.count(); n++) {
                if (frameIndices[n] >= refIndex) {
                    frameIndices[n] += deltaFrameCount;
                }
            }
        }
    }
    // update the view
    this->update();
    this->displayFrame();
}

void LevelCelView::assignSubtiles(const QImage &image, int tileIndex, int subtileIndex)
{
    QList<quint16> *subtileIndices = nullptr;
    if (tileIndex >= 0) {
        subtileIndices = &this->til->getSubtileIndices(tileIndex);
        subtileIndices->clear();
    }
    // TODO: merge with LevelCelView::insertTile ?
    unsigned subtileWidth = this->min->getSubtileWidth() * MICRO_WIDTH;
    unsigned subtileHeight = this->min->getSubtileHeight() * MICRO_HEIGHT;

    QImage subImage = QImage(subtileWidth, subtileHeight, QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); y += subtileHeight) {
        for (int x = 0; x < image.width(); x += subtileWidth) {
            // subImage.fill(Qt::transparent);

            bool hasColor = false;
            for (unsigned j = 0; j < subtileHeight; j++) {
                for (unsigned i = 0; i < subtileWidth; i++) {
                    const QColor color = image.pixelColor(x + i, y + j);
                    if (color.alpha() >= COLOR_ALPHA_LIMIT) {
                        hasColor = true;
                    }
                    subImage.setPixelColor(i, j, color);
                }
            }

            if (subtileIndices != nullptr) {
                subtileIndices->append(subtileIndex);
            } else if (!hasColor) {
                continue;
            }

            this->insertSubtile(subtileIndex, subImage);
            subtileIndex++;
        }
    }
}

void LevelCelView::insertSubtiles(IMAGE_FILE_MODE mode, int index, const QImage &image)
{
    unsigned subtileWidth = this->min->getSubtileWidth() * MICRO_WIDTH;
    unsigned subtileHeight = this->min->getSubtileHeight() * MICRO_HEIGHT;

    if ((image.width() % subtileWidth) != 0 || (image.height() % subtileHeight) != 0) {
        return;
    }

    if (mode == IMAGE_FILE_MODE::AUTO) {
        // check for tile dimensions to be more lenient than EXPORT_SUBTILES_PER_LINE
        unsigned tileWidth = subtileWidth * TILE_WIDTH * TILE_HEIGHT;
        unsigned tileHeight = subtileHeight;

        if ((image.width() % tileWidth) == 0 && (image.height() % tileHeight) == 0) {
            return; // this is a tile (or tiles) -> ignore
        }
    }

    this->assignSubtiles(image, -1, index);
}

void LevelCelView::insertSubtiles(IMAGE_FILE_MODE mode, int index, const QString &imagefilePath)
{
    QImageReader reader = QImageReader(imagefilePath);
    int numImages = 0;

    while (true) {
        QImage image = reader.read();
        if (image.isNull()) {
            break;
        }
        this->insertSubtiles(mode, index + numImages, image);
        numImages++;
    }

    if (mode != IMAGE_FILE_MODE::AUTO && numImages == 0) {
        QMessageBox::critical(this, "Error", "Failed read image file: " + imagefilePath);
    }
}

void LevelCelView::insertSubtiles(IMAGE_FILE_MODE mode, const QStringList &imagefilePaths, bool append)
{
    int prevSubtileCount = this->min->getSubtileCount();

    if (append) {
        // append the subtile(s)
        for (int i = 0; i < imagefilePaths.count(); i++) {
            this->insertSubtiles(mode, this->min->getSubtileCount(), imagefilePaths[i]);
        }
        int deltaSubtileCount = this->min->getSubtileCount() - prevSubtileCount;
        if (deltaSubtileCount == 0) {
            return; // no new subtile -> done
        }
        // jump to the first appended subtile
        this->currentSubtileIndex = prevSubtileCount;
    } else {
        // insert the subtile(s)
        for (int i = imagefilePaths.count() - 1; i >= 0; i--) {
            this->insertSubtiles(mode, this->currentSubtileIndex, imagefilePaths[i]);
        }
        int deltaSubtileCount = this->min->getSubtileCount() - prevSubtileCount;
        if (deltaSubtileCount == 0) {
            return; // no new subtile -> done
        }
        // shift references
        unsigned refIndex = this->currentSubtileIndex;
        // shift subtile indices of the tiles
        for (int i = 0; i < this->til->getTileCount(); i++) {
            QList<quint16> &subtileIndices = this->til->getSubtileIndices(i);
            for (int n = 0; n < subtileIndices.count(); n++) {
                if (subtileIndices[n] >= refIndex) {
                    subtileIndices[n] += deltaSubtileCount;
                }
            }
        }
    }
    // update the view
    this->update();
    this->displayFrame();
}

void LevelCelView::insertSubtile(int subtileIndex, const QImage &image)
{
    QList<quint16> frameIndicesList;

    int frameIndex = this->gfx->getFrameCount();
    QImage subImage = QImage(MICRO_WIDTH, MICRO_HEIGHT, QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); y += MICRO_HEIGHT) {
        for (int x = 0; x < image.width(); x += MICRO_WIDTH) {
            // subImage.fill(Qt::transparent);

            bool hasColor = false;
            for (int j = 0; j < MICRO_HEIGHT; j++) {
                for (int i = 0; i < MICRO_WIDTH; i++) {
                    const QColor color = image.pixelColor(x + i, y + j);
                    if (color.alpha() >= COLOR_ALPHA_LIMIT) {
                        hasColor = true;
                    }
                    subImage.setPixelColor(i, j, color);
                }
            }

            frameIndicesList.append(hasColor ? frameIndex + 1 : 0);

            if (!hasColor) {
                continue;
            }

            this->gfx->insertFrame(frameIndex, subImage);
            frameIndex++;
        }
    }
    this->min->insertSubtile(subtileIndex, frameIndicesList);
    this->sol->insertSubtile(subtileIndex, 0);
}

void LevelCelView::insertTile(int tileIndex, const QImage &image)
{
    QList<quint16> subtileIndices;

    unsigned subtileWidth = this->min->getSubtileWidth() * MICRO_WIDTH;
    unsigned subtileHeight = this->min->getSubtileHeight() * MICRO_HEIGHT;

    QImage subImage = QImage(subtileWidth, subtileHeight, QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); y += subtileHeight) {
        for (int x = 0; x < image.width(); x += subtileWidth) {
            // subImage.fill(Qt::transparent);

            // bool hasColor = false;
            for (unsigned j = 0; j < subtileHeight; j++) {
                for (unsigned i = 0; i < subtileWidth; i++) {
                    const QColor color = image.pixelColor(x + i, y + j);
                    // if (color.alpha() >= COLOR_ALPHA_LIMIT) {
                    //    hasColor = true;
                    // }
                    subImage.setPixelColor(i, j, color);
                }
            }

            int index = this->min->getSubtileCount();
            subtileIndices.append(index);
            this->insertSubtile(index, subImage);
        }
    }

    this->til->insertTile(tileIndex, subtileIndices);
}

void LevelCelView::insertTiles(IMAGE_FILE_MODE mode, int index, const QImage &image)
{
    unsigned tileWidth = this->min->getSubtileWidth() * MICRO_WIDTH * TILE_WIDTH * TILE_HEIGHT;
    unsigned tileHeight = this->min->getSubtileHeight() * MICRO_HEIGHT;

    if ((image.width() % tileWidth) != 0 || (image.height() % tileHeight) != 0) {
        return;
    }

    /*if (mode == IMAGE_FILE_MODE::AUTO
        && (image.width() != subtileWidth || image.height() != subtileHeight) && image.width() != subtileWidth * EXPORT_TILES_PER_LINE) {
        // not a column of tiles
        // not a row or tiles
        // not a grouped tiles from an export -> ignore
        return;
    }*/

    QImage subImage = QImage(tileWidth, tileHeight, QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); y += tileHeight) {
        for (int x = 0; x < image.width(); x += tileWidth) {
            // subImage.fill(Qt::transparent);

            bool hasColor = false;
            for (unsigned j = 0; j < tileHeight; j++) {
                for (unsigned i = 0; i < tileWidth; i++) {
                    const QColor color = image.pixelColor(x + i, y + j);
                    if (color.alpha() >= COLOR_ALPHA_LIMIT) {
                        hasColor = true;
                    }
                    subImage.setPixelColor(i, j, color);
                }
            }

            if (!hasColor) {
                continue;
            }

            this->insertTile(index, subImage);
            index++;
        }
    }
}

void LevelCelView::insertTiles(IMAGE_FILE_MODE mode, int index, const QString &imagefilePath)
{
    QImageReader reader = QImageReader(imagefilePath);
    int numImages = 0;

    while (true) {
        QImage image = reader.read();
        if (image.isNull()) {
            break;
        }
        this->insertTiles(mode, index + numImages, image);
        numImages++;
    }

    if (mode != IMAGE_FILE_MODE::AUTO && numImages == 0) {
        QMessageBox::critical(this, "Error", "Failed read image file: " + imagefilePath);
    }
}

void LevelCelView::insertTiles(IMAGE_FILE_MODE mode, const QStringList &imagefilePaths, bool append)
{
    int prevTileCount = this->til->getTileCount();

    if (append) {
        // append the tile(s)
        for (int i = 0; i < imagefilePaths.count(); i++) {
            this->insertTiles(mode, this->til->getTileCount(), imagefilePaths[i]);
        }
        int deltaTileCount = this->til->getTileCount() - prevTileCount;
        if (deltaTileCount == 0) {
            return; // no new tile -> done
        }
        // jump to the first appended tile
        this->currentTileIndex = prevTileCount;
    } else {
        // insert the tile(s)
        for (int i = imagefilePaths.count() - 1; i >= 0; i--) {
            this->insertTiles(mode, this->currentTileIndex, imagefilePaths[i]);
        }
        int deltaTileCount = this->til->getTileCount() - prevTileCount;
        if (deltaTileCount == 0) {
            return; // no new tile -> done
        }
    }
    // update the view
    this->initialize(this->gfx, this->min, this->til, this->sol, this->amp);
    this->displayFrame();
}

void LevelCelView::replaceCurrentFrame(const QString &imagefilePath)
{
    QImage image = QImage(imagefilePath);

    if (image.isNull()) {
        QMessageBox::critical(nullptr, "Error", "Failed open image file: " + imagefilePath);
        return;
    }

    if (image.width() != MICRO_WIDTH || image.height() != MICRO_HEIGHT) {
        QMessageBox::warning(this, "Warning", "The image must be 32px * 32px to be used as a level-frame.");
        return;
    }

    D1GfxFrame *frame = this->gfx->replaceFrame(this->currentFrameIndex, image);

    if (frame != nullptr) {
        LevelTabFrameWidget::selectFrameType(frame);
        // update the view
        this->initialize(this->gfx, this->min, this->til, this->sol, this->amp);
        this->displayFrame();
    }
}

void LevelCelView::removeFrame(int frameIndex)
{
    // remove the frame
    this->gfx->removeFrame(frameIndex);
    if (this->gfx->getFrameCount() == this->currentFrameIndex) {
        this->currentFrameIndex = std::max(0, this->currentFrameIndex - 1);
    }
    // shift references
    // - shift frame indices of the subtiles
    unsigned refIndex = frameIndex + 1;
    for (int i = 0; i < this->min->getSubtileCount(); i++) {
        QList<quint16> &frameIndices = this->min->getCelFrameIndices(i);
        for (int n = 0; n < frameIndices.count(); n++) {
            if (frameIndices[n] >= refIndex) {
                if (frameIndices[n] == refIndex) {
                    frameIndices[n] = 0;
                } else {
                    frameIndices[n] -= 1;
                }
            }
        }
    }
}

void LevelCelView::removeCurrentFrame()
{
    // check if the current frame is used
    QList<int> frameUsers;

    this->collectFrameUsers(this->currentFrameIndex, frameUsers);

    if (!frameUsers.isEmpty()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, "Confirmation", "The frame is used by Subtile " + QString::number(frameUsers.first() + 1) + " (and maybe others). Are you sure you want to proceed?", QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            return;
        }
    }
    // remove the current frame
    this->removeFrame(this->currentFrameIndex);
    // update the view
    this->initialize(this->gfx, this->min, this->til, this->sol, this->amp);
    this->displayFrame();
}

void LevelCelView::createSubtile()
{
    this->min->createSubtile();
    this->sol->createSubtile();
    // jump to the new subtile
    this->currentSubtileIndex = this->min->getSubtileCount() - 1;
    // update the view
    this->initialize(this->gfx, this->min, this->til, this->sol, this->amp);
    this->displayFrame();
}

void LevelCelView::cloneSubtile()
{
    int cloneFrom = this->currentSubtileIndex;
    this->createSubtile();
    this->min->getCelFrameIndices(this->currentSubtileIndex) = this->min->getCelFrameIndices(cloneFrom);
    this->displayFrame();
}

void LevelCelView::replaceCurrentSubtile(const QString &imagefilePath)
{
    QImage image = QImage(imagefilePath);

    if (image.isNull()) {
        QMessageBox::critical(nullptr, "Error", "Failed open image file: " + imagefilePath);
        return;
    }

    unsigned subtileWidth = this->min->getSubtileWidth() * MICRO_WIDTH;
    unsigned subtileHeight = this->min->getSubtileHeight() * MICRO_HEIGHT;

    if (image.width() != subtileWidth || image.height() != subtileHeight) {
        QMessageBox::warning(this, "Warning", "The image must be " + QString::number(subtileWidth) + "px * " + QString::number(subtileHeight) + "px to be used as a subtile.");
        return;
    }

    int subtileIndex = this->currentSubtileIndex;
    this->assignFrames(image, subtileIndex, this->gfx->getFrameCount());

    // update the view
    this->update();
    this->displayFrame();
}

void LevelCelView::removeSubtile(int subtileIndex)
{
    this->min->removeSubtile(subtileIndex);
    this->sol->removeSubtile(subtileIndex);
    // update subtile index if necessary
    if (this->currentSubtileIndex == this->min->getSubtileCount()) {
        this->currentSubtileIndex = std::max(0, this->currentSubtileIndex - 1);
    }
    // shift references
    // - shift subtile indices of the tiles
    unsigned refIndex = subtileIndex;
    for (int i = 0; i < this->til->getTileCount(); i++) {
        QList<quint16> &subtileIndices = this->til->getSubtileIndices(i);
        for (int n = 0; n < subtileIndices.count(); n++) {
            if (subtileIndices[n] >= refIndex) {
                // assert(subtileIndices[n] != refIndex);
                subtileIndices[n] -= 1;
            }
        }
    }
}

void LevelCelView::removeCurrentSubtile()
{
    // check if the current subtile is used
    QList<int> subtileUsers;

    this->collectSubtileUsers(this->currentSubtileIndex, subtileUsers);

    if (!subtileUsers.isEmpty()) {
        QMessageBox::critical(nullptr, "Error", "The subtile is used by Tile " + QString::number(subtileUsers.first() + 1) + " (and maybe others).");
        return;
    }
    // remove the current subtile
    this->removeSubtile(this->currentSubtileIndex);
    // update the view
    this->initialize(this->gfx, this->min, this->til, this->sol, this->amp);
    this->displayFrame();
}

void LevelCelView::createTile()
{
    this->til->createTile();
    this->amp->createTile();
    // jump to the new tile
    this->currentTileIndex = this->til->getTileCount() - 1;
    // update the view
    this->initialize(this->gfx, this->min, this->til, this->sol, this->amp);
    this->displayFrame();
}

void LevelCelView::cloneTile()
{
    int cloneFrom = this->currentTileIndex;
    this->createTile();
    this->til->getSubtileIndices(this->currentTileIndex) = this->til->getSubtileIndices(cloneFrom);
    this->displayFrame();
}

void LevelCelView::replaceCurrentTile(const QString &imagefilePath)
{
    QImage image = QImage(imagefilePath);

    if (image.isNull()) {
        QMessageBox::critical(nullptr, "Error", "Failed open image file: " + imagefilePath);
        return;
    }

    unsigned tileWidth = this->min->getSubtileWidth() * MICRO_WIDTH * TILE_WIDTH * TILE_HEIGHT;
    unsigned tileHeight = this->min->getSubtileHeight() * MICRO_HEIGHT;

    if (image.width() != tileWidth || image.height() != tileHeight) {
        QMessageBox::warning(this, "Warning", "The image must be " + QString::number(tileWidth) + "px * " + QString::number(tileHeight) + "px to be used as a tile.");
        return;
    }

    int tileIndex = this->currentTileIndex;
    this->assignSubtiles(image, tileIndex, this->min->getSubtileCount());

    // reset tile flags
    this->amp->setTileProperties(tileIndex, 0);

    // update the view
    this->update();
    this->displayFrame();
}

void LevelCelView::removeCurrentTile()
{
    this->til->removeTile(this->currentTileIndex);
    this->amp->removeTile(this->currentTileIndex);
    // update tile index if necessary
    if (this->currentTileIndex == this->til->getTileCount()) {
        this->currentTileIndex = std::max(0, this->currentTileIndex - 1);
    }
    // update the view
    this->initialize(this->gfx, this->min, this->til, this->sol, this->amp);
    this->displayFrame();
}

void LevelCelView::collectFrameUsers(int frameIndex, QList<int> &users) const
{
    unsigned refIndex = frameIndex + 1;

    for (int i = 0; i < this->min->getSubtileCount(); i++) {
        const QList<quint16> &frameIndices = this->min->getCelFrameIndices(i);
        for (quint16 index : frameIndices) {
            if (index == refIndex) {
                users.append(i);
                break;
            }
        }
    }
}

void LevelCelView::collectSubtileUsers(int subtileIndex, QList<int> &users) const
{
    for (int i = 0; i < this->til->getTileCount(); i++) {
        const QList<quint16> &subtileIndices = this->til->getSubtileIndices(i);
        for (quint16 index : subtileIndices) {
            if (index == subtileIndex) {
                users.append(i);
                break;
            }
        }
    }
}

void LevelCelView::reportUsage()
{
    QString frameUses;

    if (this->gfx->getFrameCount() > this->currentFrameIndex) {
        QList<int> frameUsers;
        this->collectFrameUsers(this->currentFrameIndex, frameUsers);

        frameUses = "Frame " + QString::number(this->currentFrameIndex + 1);
        if (frameUsers.isEmpty()) {
            frameUses += " is not used by any subtile.";
        } else {
            frameUses += " is used by subtile ";
            for (int user : frameUsers) {
                frameUses += QString::number(user + 1) + ", ";
            }
            frameUses.chop(2);
            frameUses += ".";
        }
    }

    QString subtileUses;

    if (this->min->getSubtileCount() > this->currentSubtileIndex) {
        QList<int> subtileUsers;
        this->collectSubtileUsers(this->currentSubtileIndex, subtileUsers);

        subtileUses = "Subtile " + QString::number(this->currentSubtileIndex + 1);
        if (subtileUsers.isEmpty()) {
            subtileUses += " is not used by any tile.";
        } else {
            subtileUses += " is used by tile ";
            for (int user : subtileUsers) {
                subtileUses += QString::number(user + 1) + ", ";
            }
            subtileUses.chop(2);
            subtileUses += ".";
        }
    }

    QString msg;

    if (!frameUses.isEmpty()) {
        msg = frameUses;

        if (!subtileUses.isEmpty()) {
            msg += "\n\n" + subtileUses;
        }
    } else {
        msg = "The tileset is empty.";
    }

    QMessageBox::information(this, "Information", msg);
}

static const char *getFrameTypeName(D1CEL_FRAME_TYPE type)
{
    switch (type) {
    case D1CEL_FRAME_TYPE::Square:
        return "Square";
    case D1CEL_FRAME_TYPE::TransparentSquare:
        return "Transparent square";
    case D1CEL_FRAME_TYPE::LeftTriangle:
        return "Left Triangle";
    case D1CEL_FRAME_TYPE::RightTriangle:
        return "Right Triangle";
    case D1CEL_FRAME_TYPE::LeftTrapezoid:
        return "Left Trapezoid";
    case D1CEL_FRAME_TYPE::RightTrapezoid:
        return "Right Trapezoid";
    case D1CEL_FRAME_TYPE::Empty:
        return "Empty";
    default:
        return "Unknown";
    }
}

void LevelCelView::resetFrameTypes()
{
    QString report;

    for (int i = 0; i < this->gfx->getFrameCount(); i++) {
        D1GfxFrame *frame = this->gfx->getFrame(i);
        D1CEL_FRAME_TYPE prevType = frame->getFrameType();
        LevelTabFrameWidget::selectFrameType(frame);
        D1CEL_FRAME_TYPE newType = frame->getFrameType();
        if (prevType != newType) {
            QString line = "Changed Frame %1 from '%2' to '%3'.\n";
            line = line.arg(i + 1).arg(getFrameTypeName(prevType)).arg(getFrameTypeName(newType));
            report.append(line);
        }
    }

    if (report.isEmpty()) {
        report = "No change was necessary.";
    } else {
        // update the view
        this->tabFrameWidget->update();

        report.chop(1);
    }
    QMessageBox::information(this, "Information", report);
}

void LevelCelView::removeUnusedFrames(QString &report)
{
    // collect every frame uses
    QList<bool> frameUsed;
    for (int i = 0; i < this->gfx->getFrameCount(); i++) {
        frameUsed.append(false);
    }
    for (int i = 0; i < this->min->getSubtileCount(); i++) {
        const QList<quint16> &frameIndices = this->min->getCelFrameIndices(i);
        for (quint16 frameRef : frameIndices) {
            if (frameRef != 0) {
                frameUsed[frameRef - 1] = true;
            }
        }
    }
    // remove the unused frames
    QList<int> frameRemoved;
    for (int i = this->gfx->getFrameCount() - 1; i >= 0; i--) {
        if (!frameUsed[i]) {
            frameRemoved.append(i);
            this->removeFrame(i);
        }
    }
    if (frameRemoved.isEmpty()) {
        return;
    }
    report = "Removed frame ";
    for (auto iter = frameRemoved.crbegin(); iter != frameRemoved.crend(); ++iter) {
        report += QString::number(*iter + 1) + ", ";
    }
    report.chop(2);
    report += ".";
}

void LevelCelView::removeUnusedSubtiles(QString &report)
{
    // collect every subtile uses
    QList<bool> subtileUsed;
    for (int i = 0; i < this->min->getSubtileCount(); i++) {
        subtileUsed.append(false);
    }
    for (int i = 0; i < this->til->getTileCount(); i++) {
        const QList<quint16> &subtileIndices = this->til->getSubtileIndices(i);
        for (quint16 subtileIndex : subtileIndices) {
            subtileUsed[subtileIndex] = true;
        }
    }
    // remove the unused subtiles
    QList<int> subtileRemoved;
    for (int i = this->min->getSubtileCount() - 1; i >= 0; i--) {
        if (!subtileUsed[i]) {
            subtileRemoved.append(i);
            this->removeSubtile(i);
        }
    }
    if (subtileRemoved.isEmpty()) {
        return;
    }
    report = "Removed subtile ";
    for (auto iter = subtileRemoved.crbegin(); iter != subtileRemoved.crend(); ++iter) {
        report += QString::number(*iter + 1) + ", ";
    }
    report.chop(2);
    report += ".";
}

void LevelCelView::cleanupFrames()
{
    QString report;
    this->removeUnusedFrames(report);

    if (report.isEmpty()) {
        report = "Every frame is used.";
    } else {
        // update the view
        this->update();
        this->displayFrame();
    }
    QMessageBox::information(this, "Information", report);
}

void LevelCelView::cleanupSubtiles()
{
    QString report;
    this->removeUnusedSubtiles(report);

    if (report.isEmpty()) {
        report = "Every subtile is used.";
    } else {
        // update the view
        this->update();
        this->displayFrame();
    }
    QMessageBox::information(this, "Information", report);
}

void LevelCelView::cleanupTileset()
{
    QString subtilesReport;
    this->removeUnusedSubtiles(subtilesReport);

    QString framesReport;
    this->removeUnusedFrames(framesReport);

    if (framesReport.isEmpty() && subtilesReport.isEmpty()) {
        framesReport = "Every subtile and frame are used.";
    } else {
        // update the view
        this->update();
        this->displayFrame();

        if (!subtilesReport.isEmpty()) {
            if (!framesReport.isEmpty()) {
                framesReport += "\n\n";
            }
            framesReport += subtilesReport;
        }
    }

    QMessageBox::information(this, "Information", framesReport);
}

void LevelCelView::reuseFrames(QString &report)
{
    QList<QPair<int, int>> frameRemoved;
    std::set<int> removedIndices;

    for (int i = 0; i < this->gfx->getFrameCount(); i++) {
        for (int j = i + 1; j < this->gfx->getFrameCount(); j++) {
            D1GfxFrame *frame0 = this->gfx->getFrame(i);
            D1GfxFrame *frame1 = this->gfx->getFrame(j);
            int width = frame0->getWidth();
            int height = frame0->getHeight();
            if (width != frame1->getWidth() || height != frame1->getHeight()) {
                continue; // should not happen, but better safe than sorry
            }
            bool match = true;
            for (int y = 0; y < height && match; y++) {
                for (int x = 0; x < width; x++) {
                    if (frame0->getPixel(x, y) == frame1->getPixel(x, y)) {
                        continue;
                    }
                    match = false;
                    break;
                }
            }
            if (!match) {
                continue;
            }
            // reuse frame0 instead of frame1
            const unsigned refIndex = j + 1;
            for (int n = 0; n < this->min->getSubtileCount(); n++) {
                QList<quint16> &frameIndices = this->min->getCelFrameIndices(n);
                for (auto iter = frameIndices.begin(); iter != frameIndices.end(); iter++) {
                    if (*iter == refIndex) {
                        *iter = i + 1;
                    }
                }
            }
            // eliminate frame1
            this->removeFrame(j);
            // calculate the original indices
            int originalIndexI = i;
            for (auto iter = removedIndices.cbegin(); iter != removedIndices.cend(); ++iter) {
                if (*iter <= originalIndexI) {
                    originalIndexI++;
                    continue;
                }
                break;
            }
            int originalIndexJ = j;
            for (auto iter = removedIndices.cbegin(); iter != removedIndices.cend(); ++iter) {
                if (*iter <= originalIndexJ) {
                    originalIndexJ++;
                    continue;
                }
                break;
            }
            removedIndices.insert(originalIndexJ);
            frameRemoved.append(qMakePair(originalIndexI, originalIndexJ));
            j--;
        }
    }

    if (frameRemoved.isEmpty()) {
        return;
    }

    report = "Using frame ";
    for (auto iter = frameRemoved.cbegin(); iter != frameRemoved.cend(); ++iter) {
        report += QString::number(iter->first + 1) + " instead of " + QString::number(iter->second + 1) + ", ";
    }
    report.chop(2);
    report += ".";
}

void LevelCelView::reuseSubtiles(QString &report)
{
    QList<QPair<int, int>> subtileRemoved;
    std::set<int> removedIndices;

    for (int i = 0; i < this->min->getSubtileCount(); i++) {
        for (int j = i + 1; j < this->min->getSubtileCount(); j++) {
            QList<quint16> &frameIndices0 = this->min->getCelFrameIndices(i);
            QList<quint16> &frameIndices1 = this->min->getCelFrameIndices(j);
            if (frameIndices0.count() != frameIndices1.count()) {
                continue; // should not happen, but better safe than sorry
            }
            bool match = true;
            for (int x = 0; x < frameIndices0.count(); x++) {
                if (frameIndices0[x] == frameIndices1[x]) {
                    continue;
                }
                match = false;
                break;
            }
            if (!match) {
                continue;
            }
            // use subtile 'i' instead of subtile 'j'
            const unsigned refIndex = j;
            for (int n = 0; n < this->til->getTileCount(); n++) {
                QList<quint16> &subtileIndices = this->til->getSubtileIndices(n);
                for (auto iter = subtileIndices.begin(); iter != subtileIndices.end(); iter++) {
                    if (*iter == refIndex) {
                        *iter = i;
                    }
                }
            }
            // eliminate subtile 'j'
            this->removeSubtile(j);
            // calculate the original indices
            int originalIndexI = i;
            for (auto iter = removedIndices.cbegin(); iter != removedIndices.cend(); ++iter) {
                if (*iter <= originalIndexI) {
                    originalIndexI++;
                    continue;
                }
                break;
            }
            int originalIndexJ = j;
            for (auto iter = removedIndices.cbegin(); iter != removedIndices.cend(); ++iter) {
                if (*iter <= originalIndexJ) {
                    originalIndexJ++;
                    continue;
                }
                break;
            }
            removedIndices.insert(originalIndexJ);
            subtileRemoved.append(qMakePair(originalIndexI, originalIndexJ));
            j--;
        }
    }

    if (subtileRemoved.isEmpty()) {
        return;
    }

    report = "Using subtile ";
    for (auto iter = subtileRemoved.cbegin(); iter != subtileRemoved.cend(); ++iter) {
        report += QString::number(iter->first + 1) + " instead of " + QString::number(iter->second + 1) + ", ";
    }
    report.chop(2);
    report += ".";
}

void LevelCelView::compressTileset()
{
    QString framesReport;
    this->reuseFrames(framesReport);

    QString subtilesReport;
    this->reuseSubtiles(subtilesReport);

    if (framesReport.isEmpty() && subtilesReport.isEmpty()) {
        framesReport = "Every subtile and frame are unique.";
    } else {
        // update the view
        this->update();
        this->displayFrame();

        if (!subtilesReport.isEmpty()) {
            if (!framesReport.isEmpty()) {
                framesReport += "\n\n";
            }
            framesReport += subtilesReport;
        }
    }

    QMessageBox::information(this, "Information", framesReport);
}

bool LevelCelView::sortFrames_impl()
{
    QMap<unsigned, unsigned> remap;
    bool change = false;
    unsigned idx = 1;

    for (int i = 0; i < this->min->getSubtileCount(); i++) {
        QList<quint16> &frameIndices = this->min->getCelFrameIndices(i);
        for (auto sit = frameIndices.begin(); sit != frameIndices.end(); ++sit) {
            if (*sit == 0) {
                continue;
            }
            auto mit = remap.find(*sit);
            if (mit != remap.end()) {
                *sit = mit.value();
            } else {
                remap[*sit] = idx;
                change |= *sit != idx;
                *sit = idx;
                idx++;
            }
        }
    }
    QMap<unsigned, unsigned> backmap;
    for (auto iter = remap.cbegin(); iter != remap.cend(); ++iter) {
        backmap[iter.value()] = iter.key();
    }
    this->gfx->remapFrames(backmap);
    return change;
}

bool LevelCelView::sortSubtiles_impl()
{
    QMap<unsigned, unsigned> remap;
    bool change = false;
    unsigned idx = 0;

    for (int i = 0; i < this->til->getTileCount(); i++) {
        QList<quint16> &subtileIndices = this->til->getSubtileIndices(i);
        for (auto sit = subtileIndices.begin(); sit != subtileIndices.end(); ++sit) {
            auto mit = remap.find(*sit);
            if (mit != remap.end()) {
                *sit = mit.value();
            } else {
                remap[*sit] = idx;
                change |= *sit != idx;
                *sit = idx;
                idx++;
            }
        }
    }
    QMap<unsigned, unsigned> backmap;
    for (auto iter = remap.cbegin(); iter != remap.cend(); ++iter) {
        backmap[iter.value()] = iter.key();
    }
    this->min->remapSubtiles(backmap);
    this->sol->remapSubtiles(backmap);
    return change;
}

void LevelCelView::sortFrames()
{
    if (this->sortFrames_impl()) {
        // update the view
        this->update();
        this->displayFrame();
    }
}

void LevelCelView::sortSubtiles()
{
    if (this->sortSubtiles_impl()) {
        // update the view
        this->update();
        this->displayFrame();
    }
}

void LevelCelView::displayFrame()
{
    quint16 minPosX = 0;
    quint16 tilPosX = 0;

    this->celScene->clear();

    // Getting the current frame/sub-tile/tile to display
    QImage celFrame = this->gfx->getFrameImage(this->currentFrameIndex);
    QImage subtile = this->min->getSubtileImage(this->currentSubtileIndex);
    QImage tile = this->til->getTileImage(this->currentTileIndex);

    this->tabSubTileWidget->update();
    this->tabTileWidget->update();
    this->tabFrameWidget->update();

    // Building a gray background of the width/height of the CEL frame
    QImage celFrameBackground = QImage(celFrame.width(), celFrame.height(), QImage::Format_ARGB32);
    celFrameBackground.fill(Qt::gray);
    // Building a gray background of the width/height of the MIN subtile
    QImage subtileBackground = QImage(subtile.width(), subtile.height(), QImage::Format_ARGB32);
    subtileBackground.fill(Qt::gray);
    // Building a gray background of the width/height of the MIN subtile
    QImage tileBackground = QImage(tile.width(), tile.height(), QImage::Format_ARGB32);
    tileBackground.fill(Qt::gray);

    // Resize the scene rectangle to include some padding around the CEL frame
    // the MIN subtile and the TIL tile
    this->celScene->setSceneRect(0, 0,
        celFrame.width() + subtile.width() + tile.width() + CEL_SCENE_SPACING * 4,
        tile.height() + CEL_SCENE_SPACING * 2);

    // Add the backgrond and CEL frame while aligning it in the center
    this->celScene->addPixmap(QPixmap::fromImage(celFrameBackground))
        ->setPos(CEL_SCENE_SPACING, CEL_SCENE_SPACING);
    this->celScene->addPixmap(QPixmap::fromImage(celFrame))
        ->setPos(CEL_SCENE_SPACING, CEL_SCENE_SPACING);

    // Set current frame width and height
    this->ui->celFrameWidthEdit->setText(QString::number(celFrame.width()) + " px");
    this->ui->celFrameHeightEdit->setText(QString::number(celFrame.height()) + " px");

    // Set current frame text
    this->ui->frameIndexEdit->setText(
        QString::number(this->gfx->getFrameCount() != 0 ? this->currentFrameIndex + 1 : 0));

    // MIN
    minPosX = celFrame.width() + CEL_SCENE_SPACING * 2;
    this->celScene->addPixmap(QPixmap::fromImage(subtileBackground))
        ->setPos(minPosX, CEL_SCENE_SPACING);
    this->celScene->addPixmap(QPixmap::fromImage(subtile))
        ->setPos(minPosX, CEL_SCENE_SPACING);

    // Set current frame width and height
    this->ui->minFrameWidthEdit->setText(QString::number(this->min->getSubtileWidth()));
    this->ui->minFrameWidthEdit->setToolTip(QString::number(subtile.width()) + " px");
    this->ui->minFrameHeightEdit->setText(QString::number(this->min->getSubtileHeight()));
    this->ui->minFrameHeightEdit->setToolTip(QString::number(subtile.height()) + " px");

    // Set current subtile text
    this->ui->subtileIndexEdit->setText(
        QString::number(this->min->getSubtileCount() != 0 ? this->currentSubtileIndex + 1 : 0));

    // TIL
    tilPosX = minPosX + subtile.width() + CEL_SCENE_SPACING;
    this->celScene->addPixmap(QPixmap::fromImage(tileBackground))
        ->setPos(tilPosX, CEL_SCENE_SPACING);
    this->celScene->addPixmap(QPixmap::fromImage(tile))
        ->setPos(tilPosX, CEL_SCENE_SPACING);

    // Set current frame width and height
    this->ui->tilFrameWidthEdit->setText(QString::number(TILE_WIDTH));
    this->ui->tilFrameWidthEdit->setToolTip(QString::number(tile.width()) + " px");
    this->ui->tilFrameHeightEdit->setText(QString::number(TILE_HEIGHT));
    this->ui->tilFrameHeightEdit->setToolTip(QString::number(tile.height()) + " px");

    // Set current tile text
    this->ui->tileIndexEdit->setText(
        QString::number(this->til->getTileCount() != 0 ? this->currentTileIndex + 1 : 0));

    // Notify PalView that the frame changed (used to refresh palette hits)
    emit frameRefreshed();
}

void LevelCelView::playGroup()
{
    MainWindow *mw = (MainWindow *)this->window();

    switch (this->ui->playComboBox->currentIndex()) {
    case 0: // caves
        mw->nextPaletteCycle(D1PAL_CYCLE_TYPE::CAVES);
        break;
    case 1: // nest
        mw->nextPaletteCycle(D1PAL_CYCLE_TYPE::NEST);
        break;
    case 2: // crypt
        mw->nextPaletteCycle(D1PAL_CYCLE_TYPE::CRYPT);
        break;
    }

    // this->displayFrame();
}

void LevelCelView::ShowContextMenu(const QPoint &pos)
{
    MainWindow *mw = (MainWindow *)this->window();

    QMenu contextMenu(tr("Context menu"), this);

    QMenu frameMenu(tr("Frame"), this);
    frameMenu.setToolTipsVisible(true);

    QAction action0("Insert", this);
    action0.setToolTip("Add new frames before the current one");
    QObject::connect(&action0, SIGNAL(triggered()), mw, SLOT(on_actionInsert_Frame_triggered()));
    frameMenu.addAction(&action0);

    QAction action1("Add", this);
    action1.setToolTip("Add new frames at the end");
    QObject::connect(&action1, SIGNAL(triggered()), mw, SLOT(on_actionAdd_Frame_triggered()));
    frameMenu.addAction(&action1);

    QAction action2("Replace", this);
    action2.setToolTip("Replace the current frame");
    QObject::connect(&action2, SIGNAL(triggered()), mw, SLOT(on_actionReplace_Frame_triggered()));
    if (this->gfx->getFrameCount() == 0) {
        action2.setEnabled(false);
    }
    frameMenu.addAction(&action2);

    QAction action3("Delete", this);
    action3.setToolTip("Delete the current frame");
    QObject::connect(&action3, SIGNAL(triggered()), mw, SLOT(on_actionDel_Frame_triggered()));
    if (this->gfx->getFrameCount() == 0) {
        action3.setEnabled(false);
    }
    frameMenu.addAction(&action3);

    contextMenu.addMenu(&frameMenu);

    QMenu subtileMenu(tr("Subtile"), this);
    subtileMenu.setToolTipsVisible(true);

    QAction action4("Create", this);
    action4.setToolTip("Create a new subtile");
    QObject::connect(&action4, SIGNAL(triggered()), mw, SLOT(on_actionCreate_Subtile_triggered()));
    subtileMenu.addAction(&action4);

    QAction action5("Insert", this);
    action5.setToolTip("Add new subtiles before the current one");
    QObject::connect(&action5, SIGNAL(triggered()), mw, SLOT(on_actionInsert_Subtile_triggered()));
    subtileMenu.addAction(&action5);

    QAction action6("Add", this);
    action6.setToolTip("Add new subtiles at the end");
    QObject::connect(&action6, SIGNAL(triggered()), mw, SLOT(on_actionAdd_Subtile_triggered()));
    subtileMenu.addAction(&action6);

    QAction action7("Replace", this);
    action7.setToolTip("Replace the current subtile");
    QObject::connect(&action7, SIGNAL(triggered()), mw, SLOT(on_actionReplace_Subtile_triggered()));
    if (this->min->getSubtileCount() == 0) {
        action7.setEnabled(false);
    }
    subtileMenu.addAction(&action7);

    QAction action8("Delete", this);
    action8.setToolTip("Delete the current subtile");
    QObject::connect(&action8, SIGNAL(triggered()), mw, SLOT(on_actionDel_Subtile_triggered()));
    if (this->min->getSubtileCount() == 0) {
        action8.setEnabled(false);
    }
    subtileMenu.addAction(&action8);

    contextMenu.addMenu(&subtileMenu);

    QMenu tileMenu(tr("Tile"), this);
    tileMenu.setToolTipsVisible(true);

    QAction action9("Create", this);
    action9.setToolTip("Create a new tile");
    QObject::connect(&action9, SIGNAL(triggered()), mw, SLOT(on_actionCreate_Tile_triggered()));
    if (this->min->getSubtileCount() == 0) {
        action9.setEnabled(false);
    }
    tileMenu.addAction(&action9);

    QAction action10("Insert", this);
    action10.setToolTip("Add new tiles before the current one");
    QObject::connect(&action10, SIGNAL(triggered()), mw, SLOT(on_actionInsert_Tile_triggered()));
    tileMenu.addAction(&action10);

    QAction action11("Add", this);
    action11.setToolTip("Add new tiles at the end");
    QObject::connect(&action11, SIGNAL(triggered()), mw, SLOT(on_actionAdd_Tile_triggered()));
    tileMenu.addAction(&action11);

    QAction action12("Replace", this);
    action12.setToolTip("Replace the current tile");
    QObject::connect(&action12, SIGNAL(triggered()), mw, SLOT(on_actionReplace_Tile_triggered()));
    if (this->til->getTileCount() == 0) {
        action12.setEnabled(false);
    }
    tileMenu.addAction(&action12);

    QAction action13("Delete", this);
    action13.setToolTip("Delete the current tile");
    QObject::connect(&action13, SIGNAL(triggered()), mw, SLOT(on_actionDel_Tile_triggered()));
    if (this->til->getTileCount() == 0) {
        action13.setEnabled(false);
    }
    tileMenu.addAction(&action13);

    contextMenu.addMenu(&tileMenu);

    contextMenu.exec(mapToGlobal(pos));
}

void LevelCelView::on_firstFrameButton_clicked()
{
    this->currentFrameIndex = 0;

    if (this->mode == TILESET_MODE::SUBTILE) {
        this->min->getCelFrameIndices(this->currentSubtileIndex)[this->editIndex] = this->currentFrameIndex;
    } else {
        this->mode = TILESET_MODE::FREE;
        this->update();
    }

    this->displayFrame();
}

void LevelCelView::on_previousFrameButton_clicked()
{
    if (this->currentFrameIndex >= 1)
        this->currentFrameIndex--;
    else
        this->currentFrameIndex = std::max(0, this->gfx->getFrameCount() - 1);

    if (this->mode == TILESET_MODE::SUBTILE) {
        this->min->getCelFrameIndices(this->currentSubtileIndex)[this->editIndex] = this->currentFrameIndex;
    } else {
        this->mode = TILESET_MODE::FREE;
        this->update();
    }

    this->displayFrame();
}

void LevelCelView::on_nextFrameButton_clicked()
{
    if (this->currentFrameIndex < (this->gfx->getFrameCount() - 1))
        this->currentFrameIndex++;
    else
        this->currentFrameIndex = 0;

    if (this->mode == TILESET_MODE::SUBTILE) {
        this->min->getCelFrameIndices(this->currentSubtileIndex)[this->editIndex] = this->currentFrameIndex;
    } else {
        this->mode = TILESET_MODE::FREE;
        this->update();
    }

    this->displayFrame();
}

void LevelCelView::on_lastFrameButton_clicked()
{
    this->currentFrameIndex = std::max(0, this->gfx->getFrameCount() - 1);

    if (this->mode == TILESET_MODE::SUBTILE) {
        this->min->getCelFrameIndices(this->currentSubtileIndex)[this->editIndex] = this->currentFrameIndex;
    } else {
        this->mode = TILESET_MODE::FREE;
        this->update();
    }

    this->displayFrame();
}

void LevelCelView::on_frameIndexEdit_returnPressed()
{
    int frameIndex = this->ui->frameIndexEdit->text().toInt() - 1;

    if (frameIndex >= 0 && frameIndex < this->gfx->getFrameCount()) {
        this->currentFrameIndex = frameIndex;

        if (this->mode == TILESET_MODE::SUBTILE) {
            this->min->getCelFrameIndices(this->currentSubtileIndex)[this->editIndex] = this->currentFrameIndex;
        } else {
            this->mode = TILESET_MODE::FREE;
            this->update();
        }

        this->displayFrame();
    }
}

void LevelCelView::on_firstSubtileButton_clicked()
{
    this->currentSubtileIndex = 0;

    if (this->mode == TILESET_MODE::TILE) {
        this->til->getSubtileIndices(this->currentTileIndex)[this->editIndex] = this->currentSubtileIndex;
    } else {
        this->mode = TILESET_MODE::FREE;
        this->update();
    }

    this->displayFrame();
}

void LevelCelView::on_previousSubtileButton_clicked()
{
    if (this->currentSubtileIndex >= 1)
        this->currentSubtileIndex--;
    else
        this->currentSubtileIndex = std::max(0, this->min->getSubtileCount() - 1);

    if (this->mode == TILESET_MODE::TILE) {
        this->til->getSubtileIndices(this->currentTileIndex)[this->editIndex] = this->currentSubtileIndex;
    } else {
        this->mode = TILESET_MODE::FREE;
        this->update();
    }

    this->displayFrame();
}

void LevelCelView::on_nextSubtileButton_clicked()
{
    if (this->currentSubtileIndex < this->min->getSubtileCount() - 1)
        this->currentSubtileIndex++;
    else
        this->currentSubtileIndex = 0;

    if (this->mode == TILESET_MODE::TILE) {
        this->til->getSubtileIndices(this->currentTileIndex)[this->editIndex] = this->currentSubtileIndex;
    } else {
        this->mode = TILESET_MODE::FREE;
        this->update();
    }

    this->displayFrame();
}

void LevelCelView::on_lastSubtileButton_clicked()
{
    this->currentSubtileIndex = std::max(0, this->min->getSubtileCount() - 1);

    if (this->mode == TILESET_MODE::TILE) {
        this->til->getSubtileIndices(this->currentTileIndex)[this->editIndex] = this->currentSubtileIndex;
    } else {
        this->mode = TILESET_MODE::FREE;
        this->update();
    }

    this->displayFrame();
}

void LevelCelView::on_subtileIndexEdit_returnPressed()
{
    int subtileIndex = this->ui->subtileIndexEdit->text().toInt() - 1;

    if (subtileIndex >= 0 && subtileIndex < this->min->getSubtileCount()) {
        this->currentSubtileIndex = subtileIndex;

        if (this->mode == TILESET_MODE::TILE) {
            this->til->getSubtileIndices(this->currentTileIndex)[this->editIndex] = this->currentSubtileIndex;
        } else {
            this->mode = TILESET_MODE::FREE;
            this->update();
        }

        this->displayFrame();
    }
}

void LevelCelView::on_firstTileButton_clicked()
{
    this->currentTileIndex = 0;

    this->mode = TILESET_MODE::FREE;
    this->update();
    this->displayFrame();
}

void LevelCelView::on_previousTileButton_clicked()
{
    if (this->currentTileIndex >= 1)
        this->currentTileIndex--;
    else
        this->currentTileIndex = std::max(0, this->til->getTileCount() - 1);

    this->mode = TILESET_MODE::FREE;
    this->update();
    this->displayFrame();
}

void LevelCelView::on_nextTileButton_clicked()
{
    if (this->currentTileIndex < this->til->getTileCount() - 1)
        this->currentTileIndex++;
    else
        this->currentTileIndex = 0;

    this->mode = TILESET_MODE::FREE;
    this->update();
    this->displayFrame();
}

void LevelCelView::on_lastTileButton_clicked()
{
    this->currentTileIndex = std::max(0, this->til->getTileCount() - 1);

    this->mode = TILESET_MODE::FREE;
    this->update();
    this->displayFrame();
}

void LevelCelView::on_tileIndexEdit_returnPressed()
{
    int tileIndex = this->ui->tileIndexEdit->text().toInt() - 1;

    if (tileIndex >= 0 && tileIndex < this->til->getTileCount()) {
        this->currentTileIndex = tileIndex;

        this->mode = TILESET_MODE::FREE;
        this->update();
        this->displayFrame();
    }
}

void LevelCelView::on_minFrameWidthEdit_returnPressed()
{
    unsigned width = this->ui->minFrameWidthEdit->text().toUInt();

    this->min->setSubtileWidth(width);
    this->displayFrame();
}

void LevelCelView::on_minFrameHeightEdit_returnPressed()
{
    unsigned height = this->ui->minFrameHeightEdit->text().toUInt();

    this->min->setSubtileHeight(height);
    this->displayFrame();
}

void LevelCelView::on_zoomOutButton_clicked()
{
    if (this->currentZoomFactor - 1 >= 1)
        this->currentZoomFactor -= 1;
    ui->celGraphicsView->resetTransform();
    ui->celGraphicsView->scale(this->currentZoomFactor, this->currentZoomFactor);
    ui->celGraphicsView->show();
    ui->zoomEdit->setText(QString::number(this->currentZoomFactor));
}

void LevelCelView::on_zoomInButton_clicked()
{
    if (this->currentZoomFactor + 1 <= 10)
        this->currentZoomFactor += 1;
    ui->celGraphicsView->resetTransform();
    ui->celGraphicsView->scale(this->currentZoomFactor, this->currentZoomFactor);
    ui->celGraphicsView->show();
    ui->zoomEdit->setText(QString::number(this->currentZoomFactor));
}

void LevelCelView::on_zoomEdit_returnPressed()
{
    quint8 zoom = this->ui->zoomEdit->text().toUShort();

    if (zoom >= 1 && zoom <= 10)
        this->currentZoomFactor = zoom;
    ui->celGraphicsView->resetTransform();
    ui->celGraphicsView->scale(this->currentZoomFactor, this->currentZoomFactor);
    ui->celGraphicsView->show();
    ui->zoomEdit->setText(QString::number(this->currentZoomFactor));
}

void LevelCelView::on_playDelayEdit_textChanged(const QString &text)
{
    quint16 playDelay = text.toUInt();

    if (playDelay != 0)
        this->currentPlayDelay = playDelay;
}

void LevelCelView::on_playButton_clicked()
{
    // disable the related fields
    this->ui->playButton->setEnabled(false);
    this->ui->playDelayEdit->setReadOnly(false);
    this->ui->playComboBox->setEnabled(false);
    // enable the stop button
    this->ui->stopButton->setEnabled(true);
    // preserve the palette
    ((MainWindow *)this->window())->initPaletteCycle();

    this->playTimer.start(this->currentPlayDelay);
}

void LevelCelView::on_stopButton_clicked()
{
    this->playTimer.stop();

    // restore palette
    ((MainWindow *)this->window())->resetPaletteCycle();
    // disable the stop button
    this->ui->stopButton->setEnabled(false);
    // enable the related fields
    this->ui->playButton->setEnabled(true);
    this->ui->playDelayEdit->setReadOnly(true);
    this->ui->playComboBox->setEnabled(true);
}

void LevelCelView::dragEnterEvent(QDragEnterEvent *event)
{
    this->dragMoveEvent(event);
}

void LevelCelView::dragMoveEvent(QDragMoveEvent *event)
{
    if (MainWindow::hasImageUrl(event->mimeData())) {
        event->acceptProposedAction();
    }
}

void LevelCelView::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();

    QStringList filePaths;
    for (const QUrl &url : event->mimeData()->urls()) {
        filePaths.append(url.toLocalFile());
    }
    // try to insert as frames
    ((MainWindow *)this->window())->openImageFiles(IMAGE_FILE_MODE::AUTO, filePaths, false);
}

void LevelCelView::on_addTileButton_clicked()
{
    this->createTile();
}

void LevelCelView::on_cloneTileButton_clicked()
{
    this->cloneTile();
}

void LevelCelView::on_addSubTileButton_clicked()
{
    this->createSubtile();
}

void LevelCelView::on_cloneSubTileButton_clicked()
{
    this->cloneSubtile();
}
