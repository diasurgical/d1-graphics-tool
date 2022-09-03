#include "celview.h"
#include "ui_celview.h"

void CelScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "Clicked: " << event->scenePos().x() << "," << event->scenePos().y();

    quint16 x = (quint16)event->scenePos().x();
    quint16 y = (quint16)event->scenePos().y();

    emit this->framePixelClicked(x, y);
}

CelView::CelView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CelView)
    , celScene(new CelScene)
    , currentGroupIndex(0)
    , currentFrameIndex(0)
    , currentZoomFactor(1)
{
    ui->setupUi(this);
    ui->celGraphicsView->setScene(this->celScene);
    ui->zoomEdit->setText(QString::number(this->currentZoomFactor));
    this->playTimer.connect(&this->playTimer, SIGNAL(timeout()), this, SLOT(playGroup()));

    // If a pixel of the frame was clicked get pixel color index and notify the palette widgets
    QObject::connect(this->celScene, &CelScene::framePixelClicked, this, &CelView::framePixelClicked);
}

CelView::~CelView()
{
    delete ui;
    delete celScene;
}

void CelView::initialize(D1CelBase *c)
{
    this->cel = c;

    // Displaying CEL file path information
    QFileInfo celFileInfo(this->cel->getFilePath());
    ui->celLabel->setText(celFileInfo.fileName());

    /*
    if( this->cel->getGroupCount() == 1 )
    {
    }
    */

    if (this->cel->getFrameCount() == 1) {
        ui->stopButton->setDisabled(true);
        ui->playButton->setDisabled(true);
    }

    ui->groupNumberEdit->setText(
        QString::number(this->cel->getGroupCount()));

    ui->frameNumberEdit->setText(
        QString::number(this->cel->getFrameCount()));
}

D1CelBase *CelView::getCel()
{
    return this->cel;
}

QString CelView::getCelPath()
{
    return this->cel->getFilePath();
}

quint32 CelView::getCurrentFrameIndex()
{
    return this->currentFrameIndex;
}

void CelView::framePixelClicked(quint16 x, quint16 y)
{
    quint8 index = 0;

    // If the the click event lands in the scene spacing, ignore
    if (x < CEL_SCENE_SPACING
        || x > (CEL_SCENE_SPACING + this->cel->getFrameWidth(this->currentFrameIndex))
        || y < CEL_SCENE_SPACING
        || y > (CEL_SCENE_SPACING + this->cel->getFrameHeight(this->currentFrameIndex)))
        return;

    index = this->cel->getFrame(
                         this->currentFrameIndex)
                ->getPixel(x - CEL_SCENE_SPACING, y - CEL_SCENE_SPACING)
                .getPaletteIndex();

    emit this->colorIndexClicked(index);
}

void CelView::displayFrame()
{
    if (this->cel == nullptr)
        return;

    this->celScene->clear();

    // Getting the current frame to display
    QImage celFrame = this->cel->getFrameImage(this->currentFrameIndex);

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
        QString::number(this->currentGroupIndex + 1));

    // Set current frame text
    this->ui->frameIndexEdit->setText(
        QString::number(this->currentFrameIndex + 1));

    // Notify PalView that the frame changed (used to refresh palette hits
    emit this->frameRefreshed();
}

bool CelView::checkGroupNumber()
{
    if (this->cel->getGroupCount() == 1)
        return true;

    QPair<quint16, quint16> groupFrameIndices = this->cel->getGroupFrameIndices(this->currentGroupIndex);

    return this->currentFrameIndex >= groupFrameIndices.first
        && this->currentFrameIndex <= groupFrameIndices.second;
}

void CelView::setGroupNumber()
{
    QPair<quint16, quint16> groupFrameIndices;

    for (int i = 0; i < this->cel->getGroupCount(); i++) {
        groupFrameIndices = this->cel->getGroupFrameIndices(i);

        if (this->currentFrameIndex >= groupFrameIndices.first
            && this->currentFrameIndex <= groupFrameIndices.second) {
            this->currentGroupIndex = i;
            break;
        }
    }
}

void CelView::playGroup()
{
    if (this->cel->getGroupCount() == 1) {
        if (this->currentFrameIndex < this->cel->getFrameCount() - 1)
            this->currentFrameIndex++;
        else
            this->currentFrameIndex = 0;
    } else {
        QPair<quint16, quint16> groupFrameIndices = this->cel->getGroupFrameIndices(this->currentGroupIndex);

        if (this->currentFrameIndex < groupFrameIndices.second)
            this->currentFrameIndex++;
        else
            this->currentFrameIndex = groupFrameIndices.first;
    }

    this->displayFrame();
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
        this->currentFrameIndex = this->cel->getFrameCount() - 1;

    if (!this->checkGroupNumber())
        this->setGroupNumber();
    this->displayFrame();
}

void CelView::on_nextFrameButton_clicked()
{
    if (this->currentFrameIndex < (this->cel->getFrameCount() - 1))
        this->currentFrameIndex++;
    else
        this->currentFrameIndex = 0;

    if (!this->checkGroupNumber())
        this->setGroupNumber();
    this->displayFrame();
}

void CelView::on_lastFrameButton_clicked()
{
    this->currentGroupIndex = this->cel->getGroupCount() - 1;
    this->currentFrameIndex = this->cel->getFrameCount() - 1;
    this->displayFrame();
}

void CelView::on_frameIndexEdit_returnPressed()
{
    quint32 frameIndex = this->ui->frameIndexEdit->text().toUInt() - 1;

    if (frameIndex < this->cel->getFrameCount()) {
        this->currentFrameIndex = frameIndex;
        if (!this->checkGroupNumber())
            this->setGroupNumber();
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
        this->currentGroupIndex = this->cel->getGroupCount() - 1;

    this->currentFrameIndex = this->cel->getGroupFrameIndices(
                                           this->currentGroupIndex)
                                  .first;
    this->displayFrame();
}

void CelView::on_groupIndexEdit_returnPressed()
{
    quint16 groupIndex = this->ui->groupIndexEdit->text().toUInt() - 1;

    if (groupIndex < this->cel->getGroupCount()) {
        this->currentGroupIndex = groupIndex;
        this->currentFrameIndex = this->cel->getGroupFrameIndices(
                                               this->currentGroupIndex)
                                      .first;
        this->displayFrame();
    }
}

void CelView::on_nextGroupButton_clicked()
{
    if (this->currentGroupIndex < (this->cel->getGroupCount() - 1))
        this->currentGroupIndex++;
    else
        this->currentGroupIndex = 0;

    this->currentFrameIndex = this->cel->getGroupFrameIndices(
                                           this->currentGroupIndex)
                                  .first;
    this->displayFrame();
}

void CelView::on_lastGroupButton_clicked()
{
    this->currentGroupIndex = this->cel->getGroupCount() - 1;
    this->currentFrameIndex = this->cel->getGroupFrameIndices(
                                           this->cel->getGroupCount() - 1)
                                  .first;
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

void CelView::on_playButton_clicked()
{
    this->playTimer.start(50);
}

void CelView::on_stopButton_clicked()
{
    this->playTimer.stop();
}
