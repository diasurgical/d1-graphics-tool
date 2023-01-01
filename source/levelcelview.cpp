#include "levelcelview.h"

#include <algorithm>

#include <QFileInfo>
#include <QGraphicsPixmapItem>

#include "mainwindow.h"
#include "ui_levelcelview.h"

void LevelCelScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "Clicked: " << event->scenePos().x() << "," << event->scenePos().y();

    quint16 x = (quint16)event->scenePos().x();
    quint16 y = (quint16)event->scenePos().y();

    emit this->framePixelClicked(x, y);
}

LevelCelView::LevelCelView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LevelCelView)
{
    ui->setupUi(this);
    ui->celGraphicsView->setScene(this->celScene);
    ui->zoomEdit->setText(QString::number(this->currentZoomFactor));
    ui->playDelayEdit->setText(QString::number(this->currentPlayDelay));
    ui->stopButton->setEnabled(false);
    this->playTimer.connect(&this->playTimer, SIGNAL(timeout()), this, SLOT(playGroup()));

    // If a pixel of the frame, subtile or tile was clicked get pixel color index and notify the palette widgets
    QObject::connect(this->celScene, &LevelCelScene::framePixelClicked, this, &LevelCelView::framePixelClicked);
}

LevelCelView::~LevelCelView()
{
    delete ui;
    delete celScene;
}

void LevelCelView::initialize(D1Gfx *g, D1Min *m, D1Til *t, D1Sol *s, D1Amp *a)
{
    this->gfx = g;
    this->min = m;
    this->til = t;
    this->sol = s;
    this->amp = a;

    // Displaying CEL file path information
    QFileInfo celFileInfo(this->gfx->getFilePath());
    QFileInfo minFileInfo(this->min->getFilePath());
    QFileInfo tilFileInfo(this->til->getFilePath());
    QFileInfo solFileInfo(this->sol->getFilePath());
    QFileInfo ampFileInfo(this->amp->getFilePath());
    ui->celLabel->setText(celFileInfo.fileName() + ", " + minFileInfo.fileName() + ", " + tilFileInfo.fileName() + ", " + solFileInfo.fileName() + ", " + ampFileInfo.fileName());

    ui->frameNumberEdit->setText(
        QString::number(this->gfx->getFrameCount()));

    ui->subtileNumberEdit->setText(
        QString::number(this->min->getSubtileCount()));

    ui->tileNumberEdit->setText(
        QString::number(this->til->getTileCount()));
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

void LevelCelView::framePixelClicked(quint16 x, quint16 y)
{
    quint8 index = 0;

    quint16 celFrameWidth = this->gfx->getFrameWidth(this->currentFrameIndex);
    quint16 subtileWidth = this->min->getSubtileWidth() * MICRO_WIDTH;
    quint16 tileWidth = subtileWidth * 2;

    quint16 celFrameHeight = this->gfx->getFrameHeight(this->currentFrameIndex);
    quint16 subtileHeight = this->min->getSubtileHeight() * MICRO_HEIGHT;
    quint16 tileHeight = subtileHeight + 32;

    if (x > CEL_SCENE_SPACING && x < (celFrameWidth + CEL_SCENE_SPACING)
        && y > CEL_SCENE_SPACING && y < (celFrameHeight + CEL_SCENE_SPACING)) {
        // If CEL frame color is clicked, select it in the palette widgets
        index = this->gfx->getFrame(this->currentFrameIndex)
                    ->getPixel(x - CEL_SCENE_SPACING, y - CEL_SCENE_SPACING)
                    .getPaletteIndex();

        emit this->colorIndexClicked(index);
    } else if (x > (celFrameWidth + CEL_SCENE_SPACING * 2)
        && x < (celFrameWidth + subtileWidth + CEL_SCENE_SPACING * 2)
        && y > CEL_SCENE_SPACING
        && y < (subtileHeight + CEL_SCENE_SPACING)) {
        // When a CEL frame is clicked in the subtile, display the corresponding CEL frame

        // Adjust coordinates
        quint16 stx = x - celFrameWidth - CEL_SCENE_SPACING * 2;
        quint16 sty = y - CEL_SCENE_SPACING;

        // qDebug() << "Subtile clicked: " << stx << "," << sty;

        quint8 stFrame = (sty / 32) * 2 + (stx / 32);
        quint16 frameIndex = this->min->getCelFrameIndices(this->currentSubtileIndex).at(stFrame);

        // qDebug() << "stFrame: " << stFrame << ", frameIndex: " << frameIndex;

        if (frameIndex > 0)
            this->currentFrameIndex = frameIndex - 1;
        this->displayFrame();
    } else if (x > (celFrameWidth + subtileWidth + CEL_SCENE_SPACING * 3)
        && x < (celFrameWidth + subtileWidth + tileWidth + CEL_SCENE_SPACING * 3)
        && y > CEL_SCENE_SPACING
        && y < tileHeight + CEL_SCENE_SPACING) {
        // When a subtile is clicked in the tile, display the corresponding subtile

        // Adjust coordinates
        quint16 tx = x - celFrameWidth - subtileWidth - CEL_SCENE_SPACING * 3;
        quint16 ty = y - CEL_SCENE_SPACING;

        // qDebug() << "Tile clicked" << tx << "," << ty;

        // Ground squares must be clicked
        // The four squares can be delimited by the following functions
        // f(x) = 0.5x + (tileHeight - 4*16)
        // g(x) = -0.5x + tileHeight
        // f(tx)
        int ftx = tx / 2 + tileHeight - 64;
        // g(tx)
        int gtx = -tx / 2 + tileHeight;
        // qDebug() << "fx=" << ftx << ", gx=" << gtx;
        quint8 tSubtile = 0;
        if (ty < ftx) {
            if (ty < gtx) {
                // tx to allow selecting subtile 1 and 2 if tile is clicked on the bottom left and bottom right side
                if (tx < 32)
                    tSubtile = 2;
                else if (tx > 96)
                    tSubtile = 1;
                else
                    tSubtile = 0;
            } else
                tSubtile = 1;
        } else {
            if (ty < gtx)
                tSubtile = 2;
            else
                tSubtile = 3;
        }

        quint16 subtileIndex = this->til->getSubtileIndices(this->currentTileIndex).at(tSubtile);

        // qDebug() << "tSubtile=" << tSubtile << ", subtileIndex=" << subtileIndex;

        this->currentSubtileIndex = subtileIndex;
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
    quint8 sol = this->sol->getSubtileProperties(this->currentSubtileIndex);
    quint8 ampType = this->amp->getTileType(this->currentTileIndex);
    quint8 ampProperty = this->amp->getTileProperties(this->currentTileIndex);

    this->ui->sol0->setChecked((sol & 1 << 0) != 0);
    this->ui->sol1->setChecked((sol & 1 << 1) != 0);
    this->ui->sol2->setChecked((sol & 1 << 2) != 0);
    this->ui->sol3->setChecked((sol & 1 << 3) != 0);
    this->ui->sol4->setChecked((sol & 1 << 4) != 0);
    this->ui->sol5->setChecked((sol & 1 << 5) != 0);
    this->ui->sol7->setChecked((sol & 1 << 7) != 0);

    this->ui->ampType->setCurrentIndex(ampType);

    this->ui->amp0->setChecked((ampProperty & 1 << 0) != 0);
    this->ui->amp1->setChecked((ampProperty & 1 << 1) != 0);
    this->ui->amp2->setChecked((ampProperty & 1 << 2) != 0);
    this->ui->amp3->setChecked((ampProperty & 1 << 3) != 0);
    this->ui->amp4->setChecked((ampProperty & 1 << 4) != 0);
    this->ui->amp5->setChecked((ampProperty & 1 << 5) != 0);
    this->ui->amp6->setChecked((ampProperty & 1 << 6) != 0);
    this->ui->amp7->setChecked((ampProperty & 1 << 7) != 0);

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
    this->ui->celFrameWidthEdit->setText(QString::number(celFrame.width()));
    this->ui->celFrameHeightEdit->setText(QString::number(celFrame.height()));

    // Set current frame text
    this->ui->frameIndexEdit->setText(
        QString::number(this->currentFrameIndex + 1));

    // MIN
    minPosX = celFrame.width() + CEL_SCENE_SPACING * 2;
    this->celScene->addPixmap(QPixmap::fromImage(subtileBackground))
        ->setPos(minPosX, CEL_SCENE_SPACING);
    this->celScene->addPixmap(QPixmap::fromImage(subtile))
        ->setPos(minPosX, CEL_SCENE_SPACING);

    // Set current frame width and height
    this->ui->minFrameWidthEdit->setText(QString::number(subtile.width()));
    this->ui->minFrameHeightEdit->setText(QString::number(subtile.height()));

    // Set current subtile text
    this->ui->subtileIndexEdit->setText(
        QString::number(this->currentSubtileIndex + 1));

    // TIL
    tilPosX = minPosX + subtile.width() + CEL_SCENE_SPACING;
    this->celScene->addPixmap(QPixmap::fromImage(tileBackground))
        ->setPos(tilPosX, CEL_SCENE_SPACING);
    this->celScene->addPixmap(QPixmap::fromImage(tile))
        ->setPos(tilPosX, CEL_SCENE_SPACING);

    // Set current frame width and height
    this->ui->tilFrameWidthEdit->setText(QString::number(tile.width()));
    this->ui->tilFrameHeightEdit->setText(QString::number(tile.height()));

    // Set current tile text
    this->ui->tileIndexEdit->setText(
        QString::number(this->currentTileIndex + 1));

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

void LevelCelView::on_firstFrameButton_clicked()
{
    this->currentFrameIndex = 0;
    this->displayFrame();
}

void LevelCelView::on_previousFrameButton_clicked()
{
    if (this->currentFrameIndex >= 1)
        this->currentFrameIndex--;
    else
        this->currentFrameIndex = std::max(0, this->gfx->getFrameCount() - 1);

    this->displayFrame();
}

void LevelCelView::on_nextFrameButton_clicked()
{
    if (this->currentFrameIndex < (this->gfx->getFrameCount() - 1))
        this->currentFrameIndex++;
    else
        this->currentFrameIndex = 0;

    this->displayFrame();
}

void LevelCelView::on_lastFrameButton_clicked()
{
    this->currentFrameIndex = std::max(0, this->gfx->getFrameCount() - 1);
    this->displayFrame();
}

void LevelCelView::on_frameIndexEdit_returnPressed()
{
    int frameIndex = this->ui->frameIndexEdit->text().toInt() - 1;

    if (frameIndex >= 0 && frameIndex < this->gfx->getFrameCount()) {
        this->currentFrameIndex = frameIndex;
        this->displayFrame();
    }
}

void LevelCelView::on_firstSubtileButton_clicked()
{
    this->currentSubtileIndex = 0;
    this->displayFrame();
}

void LevelCelView::on_previousSubtileButton_clicked()
{
    if (this->currentSubtileIndex >= 1)
        this->currentSubtileIndex--;
    else
        this->currentSubtileIndex = std::max(0, this->min->getSubtileCount() - 1);

    this->displayFrame();
}

void LevelCelView::on_nextSubtileButton_clicked()
{
    if (this->currentSubtileIndex < this->min->getSubtileCount() - 1)
        this->currentSubtileIndex++;
    else
        this->currentSubtileIndex = 0;

    this->displayFrame();
}

void LevelCelView::on_lastSubtileButton_clicked()
{
    this->currentSubtileIndex = std::max(0, this->min->getSubtileCount() - 1);
    this->displayFrame();
}

void LevelCelView::on_subtileIndexEdit_returnPressed()
{
    int subtileIndex = this->ui->subtileIndexEdit->text().toInt() - 1;

    if (subtileIndex >= 0 && subtileIndex < this->min->getSubtileCount()) {
        this->currentSubtileIndex = subtileIndex;
        this->displayFrame();
    }
}

void LevelCelView::on_firstTileButton_clicked()
{
    this->currentTileIndex = 0;
    this->displayFrame();
}

void LevelCelView::on_previousTileButton_clicked()
{
    if (this->currentTileIndex >= 1)
        this->currentTileIndex--;
    else
        this->currentTileIndex = std::max(0, this->til->getTileCount() - 1);

    this->displayFrame();
}

void LevelCelView::on_nextTileButton_clicked()
{
    if (this->currentTileIndex < this->til->getTileCount() - 1)
        this->currentTileIndex++;
    else
        this->currentTileIndex = 0;

    this->displayFrame();
}

void LevelCelView::on_lastTileButton_clicked()
{
    this->currentTileIndex = std::max(0, this->til->getTileCount() - 1);
    this->displayFrame();
}

void LevelCelView::on_tileIndexEdit_returnPressed()
{
    int tileIndex = this->ui->tileIndexEdit->text().toInt() - 1;

    if (tileIndex >= 0 && tileIndex < this->til->getTileCount()) {
        this->currentTileIndex = tileIndex;
        this->displayFrame();
    }
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

void LevelCelView::on_ampType_activated(int index)
{
    this->amp->setTileType(this->currentTileIndex, index);
}

void LevelCelView::updateAmpProperty()
{
    quint8 flags = 0;
    if (this->ui->amp0->checkState())
        flags |= 1 << 0;
    if (this->ui->amp1->checkState())
        flags |= 1 << 1;
    if (this->ui->amp2->checkState())
        flags |= 1 << 2;
    if (this->ui->amp3->checkState())
        flags |= 1 << 3;
    if (this->ui->amp4->checkState())
        flags |= 1 << 4;
    if (this->ui->amp5->checkState())
        flags |= 1 << 5;
    if (this->ui->amp6->checkState())
        flags |= 1 << 6;
    if (this->ui->amp7->checkState())
        flags |= 1 << 7;

    this->amp->setTileProperties(this->currentTileIndex, flags);
}

void LevelCelView::on_amp0_clicked()
{
    this->updateAmpProperty();
}

void LevelCelView::on_amp1_clicked()
{
    this->updateAmpProperty();
}

void LevelCelView::on_amp2_clicked()
{
    this->updateAmpProperty();
}

void LevelCelView::on_amp3_clicked()
{
    this->updateAmpProperty();
}

void LevelCelView::on_amp4_clicked()
{
    this->updateAmpProperty();
}

void LevelCelView::on_amp5_clicked()
{
    this->updateAmpProperty();
}

void LevelCelView::on_amp6_clicked()
{
    this->updateAmpProperty();
}

void LevelCelView::on_amp7_clicked()
{
    this->updateAmpProperty();
}

void LevelCelView::updateSolProperty()
{
    quint8 flags = 0;
    if (this->ui->sol0->checkState())
        flags |= 1 << 0;
    if (this->ui->sol1->checkState())
        flags |= 1 << 1;
    if (this->ui->sol2->checkState())
        flags |= 1 << 2;
    if (this->ui->sol3->checkState())
        flags |= 1 << 3;
    if (this->ui->sol4->checkState())
        flags |= 1 << 4;
    if (this->ui->sol5->checkState())
        flags |= 1 << 5;
    if (this->ui->sol7->checkState())
        flags |= 1 << 7;

    this->sol->setSubtileProperties(this->currentSubtileIndex, flags);
}

void LevelCelView::on_sol0_clicked()
{
    this->updateSolProperty();
}

void LevelCelView::on_sol1_clicked()
{
    this->updateSolProperty();
}

void LevelCelView::on_sol2_clicked()
{
    this->updateSolProperty();
}

void LevelCelView::on_sol3_clicked()
{
    this->updateSolProperty();
}

void LevelCelView::on_sol4_clicked()
{
    this->updateSolProperty();
}

void LevelCelView::on_sol5_clicked()
{
    this->updateSolProperty();
}

void LevelCelView::on_sol7_clicked()
{
    this->updateSolProperty();
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
    this->ui->playDelayEdit->setEnabled(false);
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
    this->ui->playDelayEdit->setEnabled(true);
    this->ui->playComboBox->setEnabled(true);
}
