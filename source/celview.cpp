#include "celview.h"

#include <algorithm>

#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QMimeData>

#include "mainwindow.h"
#include "ui_celview.h"

CelScene::CelScene(QWidget *v)
    : QGraphicsScene()
    , view(v)
{
}

void CelScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "Clicked: " << event->scenePos().x() << "," << event->scenePos().y();

    quint16 x = (quint16)event->scenePos().x();
    quint16 y = (quint16)event->scenePos().y();

    emit this->framePixelClicked(x, y);
}

void CelScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CelScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->acceptProposedAction();
}

void CelScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!event->mimeData()->hasUrls()) {
        return;
    }

    event->acceptProposedAction();

    QStringList filePaths;
    for (const QUrl &url : event->mimeData()->urls()) {
        filePaths.append(url.toLocalFile());
    }
    // try to insert as frames
    ((MainWindow *)this->view->window())->openImageFiles(filePaths, false);
}

CelView::CelView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CelView)
    , celScene(new CelScene(this))
{
    ui->setupUi(this);
    ui->celGraphicsView->setScene(this->celScene);
    ui->zoomEdit->setText(QString::number(this->currentZoomFactor));
    ui->playDelayEdit->setText(QString::number(this->currentPlayDelay));
    ui->stopButton->setEnabled(false);
    this->playTimer.connect(&this->playTimer, SIGNAL(timeout()), this, SLOT(playGroup()));

    // If a pixel of the frame was clicked get pixel color index and notify the palette widgets
    QObject::connect(this->celScene, &CelScene::framePixelClicked, this, &CelView::framePixelClicked);

    setAcceptDrops(true);
}

CelView::~CelView()
{
    delete ui;
    delete celScene;
}

void CelView::initialize(D1Gfx *g)
{
    this->gfx = g;

    // Displaying CEL file path information
    QFileInfo celFileInfo(this->gfx->getFilePath());
    ui->celLabel->setText(celFileInfo.fileName());

    /*
    if( this->gfx->getGroupCount() == 1 )
    {
    }
    */

    ui->groupNumberEdit->setText(
        QString::number(this->gfx->getGroupCount()));

    ui->frameNumberEdit->setText(
        QString::number(this->gfx->getFrameCount()));
}

int CelView::getCurrentFrameIndex()
{
    return this->currentFrameIndex;
}

void CelView::framePixelClicked(quint16 x, quint16 y)
{
    quint8 index = 0;

    // If the the click event lands in the scene spacing, ignore
    if (x < CEL_SCENE_SPACING
        || x >= (CEL_SCENE_SPACING + this->gfx->getFrameWidth(this->currentFrameIndex))
        || y < CEL_SCENE_SPACING
        || y >= (CEL_SCENE_SPACING + this->gfx->getFrameHeight(this->currentFrameIndex)))
        return;

    index = this->gfx->getFrame(
                         this->currentFrameIndex)
                ->getPixel(x - CEL_SCENE_SPACING, y - CEL_SCENE_SPACING)
                .getPaletteIndex();

    emit this->colorIndexClicked(index);
}

void CelView::insertFrames(QStringList imagefilePaths, bool append)
{
    if (append) {
        // append the frame(s)
        for (int i = 0; i < imagefilePaths.count(); i++) {
            this->gfx->insertFrame(this->gfx->getFrameCount(), imagefilePaths[i]);
        }
    } else {
        // insert the frame(s)
        for (int i = 1; i <= imagefilePaths.count(); i++) {
            this->gfx->insertFrame(this->currentFrameIndex, imagefilePaths[imagefilePaths.count() - i]);
        }
    }
    // update the view
    this->initialize(this->gfx);
    this->displayFrame();
}

void CelView::replaceCurrentFrame(QString imagefilePath)
{
    D1GfxFrame *frame = this->gfx->replaceFrame(this->currentFrameIndex, imagefilePath);

    if (frame != nullptr) {
        // update the view
        this->initialize(this->gfx);
        this->displayFrame();
    }
}

void CelView::removeCurrentFrame()
{
    // remove the frame
    this->gfx->removeFrame(this->currentFrameIndex);
    if (this->gfx->getFrameCount() == this->currentFrameIndex) {
        this->currentFrameIndex = std::max(0, this->currentFrameIndex - 1);
    }
    this->updateGroupIndex();
    // update the view
    this->initialize(this->gfx);
    this->displayFrame();
}

void CelView::displayFrame()
{
    this->celScene->clear();

    // Getting the current frame to display
    QImage celFrame = this->gfx->getFrameImage(this->currentFrameIndex);

    // Building a gray background of the width/height of the CEL frame
    QImage celFrameBackground = QImage(celFrame.width(), celFrame.height(), QImage::Format_ARGB32);
    celFrameBackground.fill(Qt::gray);

    // Resize the scene rectangle to include some padding around the CEL frame
    this->celScene->setSceneRect(0, 0,
        celFrame.width() + CEL_SCENE_SPACING * 2,
        celFrame.height() + CEL_SCENE_SPACING * 2);
    // ui->celGraphicsView->adjustSize();
    // ui->celFrameWidget->adjustSize();

    // Add the backgrond and CEL frame while aligning it in the center
    this->celScene->addPixmap(QPixmap::fromImage(celFrameBackground))
        ->setPos(CEL_SCENE_SPACING, CEL_SCENE_SPACING);
    this->celScene->addPixmap(QPixmap::fromImage(celFrame))
        ->setPos(CEL_SCENE_SPACING, CEL_SCENE_SPACING);

    // Set current frame width and height
    this->ui->celFrameWidthEdit->setText(QString::number(celFrame.width()));
    this->ui->celFrameHeightEdit->setText(QString::number(celFrame.height()));

    // Set current group text
    this->ui->groupIndexEdit->setText(
        QString::number(this->gfx->getGroupCount() != 0 ? this->currentGroupIndex + 1 : 0));

    // Set current frame text
    this->ui->frameIndexEdit->setText(
        QString::number(this->gfx->getFrameCount() != 0 ? this->currentFrameIndex + 1 : 0));

    // Notify PalView that the frame changed (used to refresh palette hits)
    emit this->frameRefreshed();
}

void CelView::updateGroupIndex()
{
    QPair<quint16, quint16> groupFrameIndices = this->gfx->getGroupFrameIndices(this->currentGroupIndex);

    if (this->currentFrameIndex < groupFrameIndices.first || this->currentFrameIndex > groupFrameIndices.second) {
        this->setGroupIndex();
    }
}

void CelView::setGroupIndex()
{
    QPair<quint16, quint16> groupFrameIndices;
    int i = 0;

    for (; i < this->gfx->getGroupCount(); i++) {
        groupFrameIndices = this->gfx->getGroupFrameIndices(i);

        if (this->currentFrameIndex >= groupFrameIndices.first
            && this->currentFrameIndex <= groupFrameIndices.second) {
            break;
        }
    }
    this->currentGroupIndex = i;
}

void CelView::playGroup()
{
    QPair<quint16, quint16> groupFrameIndices = this->gfx->getGroupFrameIndices(this->currentGroupIndex);

    if (this->currentFrameIndex < groupFrameIndices.second)
        this->currentFrameIndex++;
    else
        this->currentFrameIndex = groupFrameIndices.first;

    MainWindow *mw = (MainWindow *)this->window();
    switch (this->ui->playComboBox->currentIndex()) {
    case 0: // normal
        this->displayFrame();
        break;
    case 1:
        mw->nextPaletteCycle(D1PAL_CYCLE_TYPE::CAVES);
        break;
    case 2:
        mw->nextPaletteCycle(D1PAL_CYCLE_TYPE::NEST);
        break;
    case 3:
        mw->nextPaletteCycle(D1PAL_CYCLE_TYPE::CRYPT);
        break;
    }

    // this->displayFrame();
}

void CelView::on_firstFrameButton_clicked()
{
    this->currentGroupIndex = 0;
    this->currentFrameIndex = 0;
    this->displayFrame();
}

void CelView::on_previousFrameButton_clicked()
{
    if (this->currentFrameIndex >= 1)
        this->currentFrameIndex--;
    else
        this->currentFrameIndex = std::max(0, this->gfx->getFrameCount() - 1);

    this->updateGroupIndex();
    this->displayFrame();
}

void CelView::on_nextFrameButton_clicked()
{
    if (this->currentFrameIndex < (this->gfx->getFrameCount() - 1))
        this->currentFrameIndex++;
    else
        this->currentFrameIndex = 0;

    this->updateGroupIndex();
    this->displayFrame();
}

void CelView::on_lastFrameButton_clicked()
{
    this->currentGroupIndex = std::max(0, this->gfx->getGroupCount() - 1);
    this->currentFrameIndex = std::max(0, this->gfx->getFrameCount() - 1);
    this->displayFrame();
}

void CelView::on_frameIndexEdit_returnPressed()
{
    int frameIndex = this->ui->frameIndexEdit->text().toInt() - 1;

    if (frameIndex >= 0 && frameIndex < this->gfx->getFrameCount()) {
        this->currentFrameIndex = frameIndex;
        this->updateGroupIndex();
        this->displayFrame();
    }
}

void CelView::on_firstGroupButton_clicked()
{
    this->on_firstFrameButton_clicked();
}

void CelView::on_previousGroupButton_clicked()
{
    if (this->currentGroupIndex >= 1)
        this->currentGroupIndex--;
    else
        this->currentGroupIndex = std::max(0, this->gfx->getGroupCount() - 1);

    this->currentFrameIndex = this->gfx->getGroupFrameIndices(this->currentGroupIndex).first;
    this->displayFrame();
}

void CelView::on_groupIndexEdit_returnPressed()
{
    int groupIndex = this->ui->groupIndexEdit->text().toInt() - 1;

    if (groupIndex >= 0 && groupIndex < this->gfx->getGroupCount()) {
        this->currentGroupIndex = groupIndex;
        this->currentFrameIndex = this->gfx->getGroupFrameIndices(this->currentGroupIndex).first;
        this->displayFrame();
    }
}

void CelView::on_nextGroupButton_clicked()
{
    if (this->currentGroupIndex < (this->gfx->getGroupCount() - 1))
        this->currentGroupIndex++;
    else
        this->currentGroupIndex = 0;

    this->currentFrameIndex = this->gfx->getGroupFrameIndices(this->currentGroupIndex).first;
    this->displayFrame();
}

void CelView::on_lastGroupButton_clicked()
{
    this->currentGroupIndex = std::max(0, this->gfx->getGroupCount() - 1);
    this->currentFrameIndex = this->gfx->getGroupFrameIndices(this->currentGroupIndex).first;
    this->displayFrame();
}

void CelView::on_zoomOutButton_clicked()
{
    if (this->currentZoomFactor - 1 >= 1)
        this->currentZoomFactor -= 1;
    ui->celGraphicsView->resetTransform();
    ui->celGraphicsView->scale(this->currentZoomFactor, this->currentZoomFactor);
    ui->celGraphicsView->show();
    ui->zoomEdit->setText(QString::number(this->currentZoomFactor));
}

void CelView::on_zoomInButton_clicked()
{
    if (this->currentZoomFactor + 1 <= 10)
        this->currentZoomFactor += 1;
    ui->celGraphicsView->resetTransform();
    ui->celGraphicsView->scale(this->currentZoomFactor, this->currentZoomFactor);
    ui->celGraphicsView->show();
    ui->zoomEdit->setText(QString::number(this->currentZoomFactor));
}

void CelView::on_zoomEdit_returnPressed()
{
    quint8 zoom = this->ui->zoomEdit->text().toUShort();

    if (zoom >= 1 && zoom <= 10)
        this->currentZoomFactor = zoom;
    ui->celGraphicsView->resetTransform();
    ui->celGraphicsView->scale(this->currentZoomFactor, this->currentZoomFactor);
    ui->celGraphicsView->show();
    ui->zoomEdit->setText(QString::number(this->currentZoomFactor));
}

void CelView::on_playDelayEdit_textChanged(const QString &text)
{
    quint16 playDelay = text.toUInt();

    if (playDelay != 0)
        this->currentPlayDelay = playDelay;
}

void CelView::on_playButton_clicked()
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

void CelView::on_stopButton_clicked()
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

void CelView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CelView::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasUrls()) {
        return;
    }

    event->acceptProposedAction();

    QStringList filePaths;
    for (const QUrl &url : event->mimeData()->urls()) {
        filePaths.append(url.toLocalFile());
    }
    // try to insert as frames
    ((MainWindow *)this->window())->openImageFiles(filePaths, false);
}
