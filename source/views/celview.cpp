#include "celview.h"
#include <algorithm>

#include <QAction>

#include "mainwindow.h"
#include <QDebug>
#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QImageReader>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <utility>

#include "ui_celview.h"
#include "undostack/framecmds.h"
#include "undostack/undomacro.h"

CelScene::CelScene(QWidget *v)
    : QGraphicsScene()
    , view(v)
{
}

void CelScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // return if currently set drag mode is scroll hand drag
    if (views()[0]->dragMode() == QGraphicsView::ScrollHandDrag)
        return;

    if (event->button() != Qt::LeftButton) {
        return;
    }

    int x = event->scenePos().x();
    int y = event->scenePos().y();

    qDebug() << "Clicked: " << x << "," << y;

    emit this->framePixelClicked(x, y);
}

void CelScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    bool isInImage = false;
    auto *levelcelview = dynamic_cast<LevelCelView *>(view);
    if (levelcelview != nullptr) {
        if (levelcelview->checkImageType(event->scenePos().x(), event->scenePos().y()) != IMAGE_TYPE::NONE)
            isInImage = true;
    } else {
        isInImage = dynamic_cast<CelView *>(view)->isInImage(event->scenePos().x(), event->scenePos().y());
    }

    if (isInImage) {
        dynamic_cast<MainWindow *>(view->window())->updateStatusBar(QString::fromStdString(std::to_string(static_cast<int>(event->scenePos().x())) + ", " + std::to_string(static_cast<int>(event->scenePos().y()))), "color: rgb(0, 0, 0);");
        return;
    }

    dynamic_cast<MainWindow *>(view->window())->updateStatusBar(QString::fromStdString(std::to_string(static_cast<int>(event->scenePos().x())) + ", " + std::to_string(static_cast<int>(event->scenePos().y()))), "color: rgb(160, 160, 160);");
}

void CelScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    this->dragMoveEvent(event);
}

void CelScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (MainWindow::hasImageUrl(event->mimeData())) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void CelScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    event->acceptProposedAction();

    QStringList filePaths;
    for (const QUrl &url : event->mimeData()->urls()) {
        filePaths.append(url.toLocalFile());
    }
    // try to insert as frames
    ((MainWindow *)this->view->window())->openImageFiles(IMAGE_FILE_MODE::AUTO, filePaths, false);
}

void CelScene::contextMenuEvent(QContextMenuEvent *event)
{
    emit this->showContextMenu(event->globalPos());
}

CelView::CelView(std::shared_ptr<UndoStack> us, QWidget *parent)
    : QWidget(parent)
    , undoStack(std::move(us))
    , ui(new Ui::CelView())
    , celScene(new CelScene(this))
{
    this->ui->setupUi(this);
    this->ui->celGraphicsView->setScene(this->celScene);
    this->ui->zoomEdit->setText(QString::number(this->currentZoomFactor));
    this->ui->playDelayEdit->setText(QString::number(this->currentPlayDelay));
    this->ui->stopButton->setEnabled(false);
    this->playTimer.connect(&this->playTimer, SIGNAL(timeout()), this, SLOT(playGroup()));

    // If a pixel of the frame was clicked get pixel color index and notify the palette widgets
    QObject::connect(this->celScene, &CelScene::framePixelClicked, this, &CelView::framePixelClicked);

    // setup context menu
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowContextMenu(const QPoint &)));
    QObject::connect(this->celScene, &CelScene::showContextMenu, this, &CelView::ShowContextMenu);

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
    QFileInfo gfxFileInfo(this->gfx->getFilePath());
    ui->celLabel->setText(gfxFileInfo.fileName());

    ui->groupNumberEdit->setText(
        QString::number(this->gfx->getGroupCount()));

    ui->frameNumberEdit->setText(
        QString::number(this->gfx->getFrameCount()));
}

int CelView::getCurrentFrameIndex()
{
    return this->currentFrameIndex;
}

bool CelView::isInImage(unsigned int x, unsigned int y) const
{
    const int frameIndex = this->currentFrameIndex;

    const int tx = x - CEL_SCENE_SPACING;
    const int ty = y - CEL_SCENE_SPACING;

    return ((ty > 0 && ty < this->gfx->getFrameHeight(frameIndex)) /* Coordinates are above/below image */
        && (tx > 0 && tx < this->gfx->getFrameWidth(frameIndex) /* Coordinates are to the left/right of the image */));
}

void CelView::framePixelClicked(unsigned x, unsigned y)
{
    if (!isInImage(x, y))
        return;

    int colorIndex = this->gfx->getFrame(this->currentFrameIndex)->getPixel(x - CEL_SCENE_SPACING, y - CEL_SCENE_SPACING).getPaletteIndex();

    emit this->colorIndexClicked(colorIndex);
}

void CelView::insertImageFiles(IMAGE_FILE_MODE mode, const QStringList &imagefilePaths, bool append)
{
    // FIXME: remove that boolean variable and separate functions so there are both append and
    // insert ones
    if (append) {
        // append the frame(s)
        for (int i = 0; i < imagefilePaths.count(); i++) {
            this->sendAddFrameCmd(mode, this->gfx->getFrameCount(), imagefilePaths[i]);
        }
    } else {
        // insert the frame(s)
        for (int i = imagefilePaths.count() - 1; i >= 0; i--) {
            this->sendAddFrameCmd(mode, this->currentFrameIndex, imagefilePaths[i]);
        }
    }
}

void CelView::insertImageFile(int frameIdx, const QImage img)
{
    int prevFrameCount = this->gfx->getFrameCount();

    // insert group with last frame that was deleted in it, or
    // insert a frame in a group where it was before
    if (!removedGroupIdxs.empty()) {
        int removedGroupIdx = removedGroupIdxs.top();
        this->gfx->insertGroup(removedGroupIdx, frameIdx, img);
        removedGroupIdxs.pop();
    } else {
        int groupIdx = this->removedFrameGroupIdxs.top();
        this->gfx->insertFrameInGroup(frameIdx, groupIdx, img);
        this->removedFrameGroupIdxs.pop();
    }

    int deltaFrameCount = this->gfx->getFrameCount() - prevFrameCount;
    if (deltaFrameCount == 0) {
        return; // no new frame -> done
    }
    // jump to the first appended frame
    this->currentFrameIndex = frameIdx;
    this->updateGroupIndex();

    // update the view
    this->initialize(this->gfx);
    this->displayFrame();
}

void CelView::insertFrames(int index, const QImage &image)
{
    int prevFrameCount = this->gfx->getFrameCount();

    this->gfx->insertFrame(index, image);

    int deltaFrameCount = this->gfx->getFrameCount() - prevFrameCount;
    if (deltaFrameCount == 0) {
        return; // no new frame -> done
    }

    this->currentFrameIndex = index;
    this->updateGroupIndex();

    // update the view
    this->initialize(this->gfx);
    this->displayFrame();
}

void CelView::removeFrames(int index)
{
    this->removeCurrentFrame(index);
}

void CelView::sendAddFrameCmd(IMAGE_FILE_MODE mode, int index, const QString &imagefilePath)
{
    QImageReader reader = QImageReader(imagefilePath);

    auto readImage = [&](QImage &img) -> bool {
        if (!reader.read(&img)) {
            QMessageBox::critical(this, "Error", "Failed to read image file: " + imagefilePath);
            return false;
        }

        return true;
    };

    auto connectCommand = [&](QImage &img) -> std::unique_ptr<AddFrameCommand> {
        auto command = std::make_unique<AddFrameCommand>(index, img);

        // Connect signals which will be called upon redo/undo operations of the undostack
        QObject::connect(command.get(), &AddFrameCommand::added, this, &CelView::insertFrames);
        QObject::connect(command.get(), &AddFrameCommand::undoAdded, this, &CelView::removeFrames);
        return command;
    };

    // If we have more than one image, then we want to use a macro
    if (reader.imageCount() > 1) {
        QObject::connect(this->undoStack.get(), &UndoStack::initializeWidget, dynamic_cast<MainWindow *>(this->window()), &MainWindow::setupUndoMacroWidget, Qt::UniqueConnection);
        QObject::connect(this->undoStack.get(), &UndoStack::updateWidget, dynamic_cast<MainWindow *>(this->window()), &MainWindow::updateUndoMacroWidget, Qt::UniqueConnection);

        UndoMacroFactory macroFactory({ "Inserting frames...", "Abort", { 0, reader.imageCount() } });

        int numImages = 0;
        while (numImages != reader.imageCount()) {
            QImage image;
            if (!readImage(image))
                return;

            auto command = connectCommand(image);

            macroFactory.add(std::move(command));

            numImages++;
        }

        undoStack->addMacro(macroFactory);
        return;
    }

    QImage image;
    if (!readImage(image))
        return;

    auto command = connectCommand(image);

    undoStack->push(std::move(command));
}

void CelView::replaceCurrentFrame(int frameIdx, const QImage &image)
{
    this->gfx->replaceFrame(frameIdx, image);

    // update the view
    this->initialize(this->gfx);
    this->displayFrame();
}

void CelView::sendReplaceCurrentFrameCmd(const QString &imagefilePath)
{
    QImage image = QImage(imagefilePath);

    if (image.isNull()) {
        return;
    }

    // send a command to undostack, making replacing frame undo/redoable
    std::unique_ptr<ReplaceFrameCommand> command = std::make_unique<ReplaceFrameCommand>(this->currentFrameIndex, image, this->gfx->getFrameImage(this->currentFrameIndex));
    QObject::connect(command.get(), &ReplaceFrameCommand::replaced, this, &CelView::replaceCurrentFrame);
    QObject::connect(command.get(), &ReplaceFrameCommand::undoReplaced, this, &CelView::replaceCurrentFrame);

    undoStack->push(std::move(command));
}

void CelView::sendRemoveFrameCmd()
{
    // send a command to undostack, making deleting frame undo/redoable
    std::unique_ptr<RemoveFrameCommand> command = std::make_unique<RemoveFrameCommand>(this->currentFrameIndex, this->gfx->getFrameImage(this->currentFrameIndex));
    QObject::connect(command.get(), &RemoveFrameCommand::removed, this, &CelView::removeCurrentFrame);
    QObject::connect(command.get(), &RemoveFrameCommand::inserted, this, &CelView::insertImageFile);

    this->undoStack->push(std::move(command));
}

void CelView::removeCurrentFrame(int frameIdx)
{
    // remove the frame
    auto removedGroupIdx = this->gfx->removeFrame(frameIdx);

    // if any group index has been removed, we have to store it so we can restore the group if we start inserting back frames
    // otherwise we need to store current group index, so if user does undo operation on remove frame operation
    // we can assign the frame to the correct group, where it was before
    if (removedGroupIdx != std::nullopt)
        removedGroupIdxs.push(*removedGroupIdx);
    else
        removedFrameGroupIdxs.push(this->currentGroupIndex);

    if (this->gfx->getFrameCount() == this->currentFrameIndex) {
        this->currentFrameIndex = std::max(0, this->currentFrameIndex - 1);
    }
    this->updateGroupIndex();
    // update the view
    this->initialize(this->gfx);
    this->displayFrame();
}

void CelView::regroupFrames(int numGroups)
{
    this->gfx->regroupFrames(numGroups);
    this->on_firstFrameButton_clicked();
}

void CelView::displayFrame()
{
    this->celScene->clear();

    // Getting the current frame to display
    QImage celFrame = this->gfx->getFrameImage(this->currentFrameIndex);
    int celFrameWidth = this->gfx->getFrameWidth(this->currentFrameIndex);
    int celFrameHeight = this->gfx->getFrameHeight(this->currentFrameIndex);

    // Resize the scene rectangle to include some padding around the CEL frame
    this->celScene->setSceneRect(0, 0,
        celFrame.width() + CEL_SCENE_SPACING * 2,
        celFrame.height() + CEL_SCENE_SPACING * 2);
    // ui->celGraphicsView->adjustSize();
    // ui->celFrameWidget->adjustSize();

    if (celFrameWidth > 0 && celFrameHeight > 0) {
        // Building a gray background of the width/height of the CEL frame
        QImage celFrameBackground = QImage(celFrame.width(), celFrame.height(), QImage::Format_ARGB32);
        celFrameBackground.fill(Qt::gray);

        // Add the background behind the CEL frame
        this->celScene->addPixmap(QPixmap::fromImage(celFrameBackground))
            ->setPos(CEL_SCENE_SPACING, CEL_SCENE_SPACING);
    }

    // Add the CEL frame while aligning it in the center
    this->celScene->addPixmap(QPixmap::fromImage(celFrame))
        ->setPos(CEL_SCENE_SPACING, CEL_SCENE_SPACING);

    // Set current frame width and height
    this->ui->celFrameWidthEdit->setText(QString::number(celFrameWidth) + " px");
    this->ui->celFrameHeightEdit->setText(QString::number(celFrameHeight) + " px");

    // Set current group text
    this->ui->groupIndexEdit->setText(
        QString::number(this->gfx->getGroupCount() != 0 ? this->currentGroupIndex + 1 : 0));

    ui->groupNumberEdit->setText(
        QString::number(this->gfx->getGroupCount() != 0 ? this->gfx->getGroupCount() : 0));

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
    // FIXME: this probably can be rewritten in a different way
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

    int cycleType = this->ui->playComboBox->currentIndex();
    if (cycleType == 0) {
        // normal playback
        this->displayFrame();
    } else {
        mw->nextPaletteCycle((D1PAL_CYCLE_TYPE)(cycleType - 1));
        // this->displayFrame();
    }
}

void CelView::ShowContextMenu(const QPoint &pos)
{
    MainWindow *mw = (MainWindow *)this->window();

    QMenu contextMenu(tr("Context menu"), this);
    contextMenu.setToolTipsVisible(true);

    QAction action0("Insert Frame", this);
    action0.setToolTip("Add new frames before the current one");
    QObject::connect(&action0, SIGNAL(triggered()), mw, SLOT(actionInsertFrame_triggered()));
    contextMenu.addAction(&action0);

    QAction action1("Append Frame", this);
    action1.setToolTip("Add new frames at the end");
    QObject::connect(&action1, SIGNAL(triggered()), mw, SLOT(actionAddFrame_triggered()));
    contextMenu.addAction(&action1);

    QAction action2("Replace Frame", this);
    action2.setToolTip("Replace the current frame");
    QObject::connect(&action2, SIGNAL(triggered()), mw, SLOT(actionReplaceFrame_triggered()));
    if (this->gfx->getFrameCount() == 0) {
        action2.setEnabled(false);
    }
    contextMenu.addAction(&action2);

    QAction action3("Del Frame", this);
    action3.setToolTip("Delete the current frame");
    QObject::connect(&action3, SIGNAL(triggered()), mw, SLOT(actionDelFrame_triggered()));
    if (this->gfx->getFrameCount() == 0) {
        action3.setEnabled(false);
    }
    contextMenu.addAction(&action3);

    contextMenu.exec(mapToGlobal(pos));
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
    // don't do anything if there's only one group
    if (this->gfx->getGroupCount() == 1)
        return;

    this->on_firstFrameButton_clicked();
}

void CelView::on_previousGroupButton_clicked()
{
    // don't do anything if there's only one group
    if (this->gfx->getGroupCount() == 1)
        return;

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
    // don't do anything if there's only one group
    if (this->gfx->getGroupCount() == 1)
        return;

    if (this->currentGroupIndex < (this->gfx->getGroupCount() - 1))
        this->currentGroupIndex++;
    else
        this->currentGroupIndex = 0;

    this->currentFrameIndex = this->gfx->getGroupFrameIndices(this->currentGroupIndex).first;
    this->displayFrame();
}

void CelView::on_lastGroupButton_clicked()
{
    // don't do anything if there's only one group
    if (this->gfx->getGroupCount() == 1)
        return;

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
    this->ui->playDelayEdit->setReadOnly(true);
    this->ui->playComboBox->setEnabled(false);
    // enable the stop button
    this->ui->stopButton->setEnabled(true);

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
    this->ui->playDelayEdit->setReadOnly(false);
    this->ui->playComboBox->setEnabled(true);
}

void CelView::dragEnterEvent(QDragEnterEvent *event)
{
    this->dragMoveEvent(event);
}

void CelView::dragMoveEvent(QDragMoveEvent *event)
{
    if (MainWindow::hasImageUrl(event->mimeData())) {
        event->acceptProposedAction();
    }
}

void CelView::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();

    QStringList filePaths;
    for (const QUrl &url : event->mimeData()->urls()) {
        filePaths.append(url.toLocalFile());
    }
    // try to insert as frames
    ((MainWindow *)this->window())->openImageFiles(IMAGE_FILE_MODE::AUTO, filePaths, false);
}
