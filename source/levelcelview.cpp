#include "levelcelview.h"
#include "ui_levelcelview.h"

void LevelCelScene::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    qDebug() << "Clicked: " << event->scenePos().x() << "," << event->scenePos().y();

    quint16 x = (quint16)event->scenePos().x();
    quint16 y = (quint16)event->scenePos().y();

    emit this->framePixelClicked( x, y );
}

LevelCelView::LevelCelView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LevelCelView),
    celScene( new LevelCelScene ),
    currentFrameIndex( 0 ),
    currentSubtileIndex( 0 ),
    currentTileIndex( 0 ),
    currentZoomFactor( 1 )
{
    ui->setupUi(this);
    ui->celGraphicsView->setScene( this->celScene );
    ui->zoomEdit->setText( QString::number( this->currentZoomFactor ) );

    // If a pixel of the frame, subtile or tile was clicked get pixel color index and notify the palette widgets
    QObject::connect( this->celScene, &LevelCelScene::framePixelClicked, this, &LevelCelView::framePixelClicked );
}

LevelCelView::~LevelCelView()
{
    delete ui;
    delete celScene;
}

void LevelCelView::initialize( D1CelBase* c, D1Min* m, D1Til* t, D1Sol* s )
{
    this->cel = c;
    this->min = m;
    this->til = t;
    this->sol = s;

    // Displaying CEL file path information
    QFileInfo celFileInfo( this->cel->getFilePath() );
    QFileInfo minFileInfo( this->min->getFilePath() );
    QFileInfo tilFileInfo( this->til->getFilePath() );
    QFileInfo solFileInfo( this->sol->getFilePath() );
    ui->celLabel->setText( celFileInfo.fileName()+", "+minFileInfo.fileName()+", "+tilFileInfo.fileName()+", "+solFileInfo.fileName() );

    ui->frameNumberEdit->setText(
        QString::number( this->cel->getFrameCount() ) );

    ui->subtileNumberEdit->setText(
        QString::number( this->min->getSubtileCount() ) );

    ui->tileNumberEdit->setText(
        QString::number( this->til->getTileCount() ) );
}

D1CelBase* LevelCelView::getCel()
{
    return this->cel;
}

QString LevelCelView::getCelPath()
{
    return this->cel->getFilePath();
}

D1Min* LevelCelView::getMin()
{
    return this->min;
}

D1Til* LevelCelView::getTil()
{
    return this->til;
}

quint32 LevelCelView::getCurrentFrameIndex()
{
    return this->currentFrameIndex;
}

quint16 LevelCelView::getCurrentSubtileIndex()
{
    return this->currentSubtileIndex;
}

quint16 LevelCelView::getCurrentTileIndex()
{
    return this->currentTileIndex;
}

void LevelCelView::framePixelClicked( quint16 x, quint16 y )
{
    quint8 index = 0;

    quint16 celFrameWidth = this->cel->getFrameWidth( this->currentFrameIndex );
    quint16 subtileWidth = this->min->getSubtileWidth()*32;
    quint16 tileWidth = subtileWidth*2;

    quint16 celFrameHeight = this->cel->getFrameHeight( this->currentFrameIndex );
    quint16 subtileHeight = this->min->getSubtileHeight()*32;
    quint16 tileHeight = subtileHeight+32;

    if( x > CEL_SCENE_SPACING && x < (celFrameWidth+CEL_SCENE_SPACING)
        && y > CEL_SCENE_SPACING && y < (celFrameHeight+CEL_SCENE_SPACING) )
    {
        // If CEL frame color is clicked, select it in the palette widgets
        index = this->cel->getFrame(
            this->currentFrameIndex)->getPixel(x-CEL_SCENE_SPACING,y-CEL_SCENE_SPACING).getPaletteIndex();

        emit this->colorIndexClicked( index );
    }
    else if( x > (celFrameWidth+CEL_SCENE_SPACING*2)
        && x < (celFrameWidth+subtileWidth+CEL_SCENE_SPACING*2)
        && y > CEL_SCENE_SPACING
        && y < (subtileHeight+CEL_SCENE_SPACING) )
    {
        // When a CEL frame is clicked in the subtile, display the corresponding CEL frame

        // Adjust coordinates
        quint16 stx = x - celFrameWidth - CEL_SCENE_SPACING*2;
        quint16 sty = y - CEL_SCENE_SPACING;

        //qDebug() << "Subtile clicked: " << stx << "," << sty;

        quint8 stFrame = (sty/32)*2+(stx/32);
        quint16 frameIndex = this->min->getCelFrameIndices( this->currentSubtileIndex ).at( stFrame );

        //qDebug() << "stFrame: " << stFrame << ", frameIndex: " << frameIndex;

        if( frameIndex > 0 )
            this->currentFrameIndex = frameIndex-1;
        this->displayFrame();
    }
    else if( x > (celFrameWidth+subtileWidth+CEL_SCENE_SPACING*3)
        && x < (celFrameWidth+subtileWidth+tileWidth+CEL_SCENE_SPACING*3)
        && y > CEL_SCENE_SPACING
        && y < tileHeight+CEL_SCENE_SPACING )
    {
        // When a subtile is clicked in the tile, display the corresponding subtile

        // Adjust coordinates
        quint16 tx = x - celFrameWidth - subtileWidth - CEL_SCENE_SPACING*3;
        quint16 ty = y - CEL_SCENE_SPACING;

        //qDebug() << "Tile clicked" << tx << "," << ty;

        // Ground squares must be clicked
        // The four squares can be delimited by the following functions
        // f(x) = 0.5x + (tileHeight - 4*16)
        // g(x) = -0.5x + tileHeight
        // f(tx)
        int ftx = tx/2 + tileHeight - 64;
        // g(tx)
        int gtx = -tx/2 + tileHeight;
        //qDebug() << "fx=" << ftx << ", gx=" << gtx;
        quint8 tSubtile = 0;
        if( ty < ftx )
        {
            if( ty < gtx )
            {
                // tx to allow selecting subtile 1 and 2 if tile is clicked on the bottom left and bottom right side
                if( tx < 32 )
                    tSubtile = 2;
                else if( tx > 96 )
                    tSubtile = 1;
                else
                    tSubtile = 0;
            }
            else
                tSubtile = 1;
        }
        else
        {
            if( ty < gtx )
                tSubtile = 2;
            else
                tSubtile = 3;
        }

        quint16 subtileIndex = this->til->getSubtileIndices( this->currentTileIndex ).at( tSubtile );

        //qDebug() << "tSubtile=" << tSubtile << ", subtileIndex=" << subtileIndex;

        this->currentSubtileIndex = subtileIndex;
        this->displayFrame();
    }
}

void LevelCelView::displayFrame()
{
    quint16 minPosX = 0;
    quint16 tilPosX = 0;

    if( !this->cel )
        return;

    this->celScene->clear();

    // Getting the current frame/sub-tile/tile to display
    QImage celFrame = this->cel->getFrameImage( this->currentFrameIndex );
    QImage subtile = this->min->getSubtileImage( this->currentSubtileIndex );
    QImage tile = this->til->getTileImage( this->currentTileIndex );
    quint8 sol = this->sol->getSubtileProperties( this->currentSubtileIndex );

    this->ui->sol0->setChecked((sol & 1 << 0) != 0);
    this->ui->sol1->setChecked((sol & 1 << 1) != 0);
    this->ui->sol2->setChecked((sol & 1 << 2) != 0);
    this->ui->sol3->setChecked((sol & 1 << 3) != 0);
    this->ui->sol4->setChecked((sol & 1 << 4) != 0);
    this->ui->sol5->setChecked((sol & 1 << 5) != 0);
    this->ui->sol7->setChecked((sol & 1 << 7) != 0);

    // Building a gray background of the width/height of the CEL frame
    QImage celFrameBackground = QImage( celFrame.width(), celFrame.height(), QImage::Format_ARGB32 );
    celFrameBackground.fill( Qt::gray );
    // Building a gray background of the width/height of the MIN subtile
    QImage subtileBackground = QImage( subtile.width(), subtile.height(), QImage::Format_ARGB32 );
    subtileBackground.fill( Qt::gray );
    // Building a gray background of the width/height of the MIN subtile
    QImage tileBackground = QImage( tile.width(), tile.height(), QImage::Format_ARGB32 );
    tileBackground.fill( Qt::gray );

    // Resize the scene rectangle to include some padding around the CEL frame
    // the MIN subtile and the TIL tile
    this->celScene->setSceneRect( 0, 0,
        celFrame.width()+subtile.width()+tile.width()+CEL_SCENE_SPACING*4,
        tile.height()+CEL_SCENE_SPACING*2 );

    // Add the backgrond and CEL frame while aligning it in the center
    this->celScene->addPixmap( QPixmap::fromImage( celFrameBackground ) )
            ->setPos( CEL_SCENE_SPACING, CEL_SCENE_SPACING );
    this->celScene->addPixmap( QPixmap::fromImage( celFrame ) )
            ->setPos( CEL_SCENE_SPACING, CEL_SCENE_SPACING );

    // Set current frame width and height
    this->ui->celFrameWidthEdit->setText( QString::number( celFrame.width() ) );
    this->ui->celFrameHeightEdit->setText( QString::number( celFrame.height() ) );

    // Set current frame text
    this->ui->frameIndexEdit->setText(
        QString::number( this->currentFrameIndex + 1 ) );

    // MIN
    minPosX = celFrame.width() + CEL_SCENE_SPACING*2;
    this->celScene->addPixmap( QPixmap::fromImage( subtileBackground ) )
            ->setPos(minPosX, CEL_SCENE_SPACING );
    this->celScene->addPixmap( QPixmap::fromImage( subtile ) )
            ->setPos(minPosX, CEL_SCENE_SPACING );

    // Set current frame width and height
    this->ui->minFrameWidthEdit->setText( QString::number( subtile.width() ) );
    this->ui->minFrameHeightEdit->setText( QString::number( subtile.height() ) );

    // Set current subtile text
    this->ui->subtileIndexEdit->setText(
        QString::number( this->currentSubtileIndex + 1 ) );

    // TIL
    tilPosX = minPosX + subtile.width() + CEL_SCENE_SPACING;
    this->celScene->addPixmap( QPixmap::fromImage( tileBackground ) )
            ->setPos( tilPosX, CEL_SCENE_SPACING );
    this->celScene->addPixmap( QPixmap::fromImage( tile ) )
            ->setPos( tilPosX, CEL_SCENE_SPACING );

    // Set current frame width and height
    this->ui->tilFrameWidthEdit->setText( QString::number( tile.width() ) );
    this->ui->tilFrameHeightEdit->setText( QString::number( tile.height() ) );

    // Set current tile text
    this->ui->tileIndexEdit->setText(
        QString::number( this->currentTileIndex + 1 ) );

    // Notify PalView that the frame changed (used to refresh palette hits)
    emit frameRefreshed();
}

void LevelCelView::on_firstFrameButton_clicked()
{
    this->currentFrameIndex = 0;
    this->displayFrame();
}

void LevelCelView::on_previousFrameButton_clicked()
{
    if( this->currentFrameIndex >= 1 )
        this->currentFrameIndex--;
    else
        this->currentFrameIndex = this->cel->getFrameCount() - 1;

    this->displayFrame();
}

void LevelCelView::on_nextFrameButton_clicked()
{
    if( this->currentFrameIndex < (this->cel->getFrameCount() - 1) )
        this->currentFrameIndex++;
    else
        this->currentFrameIndex = 0;

    this->displayFrame();
}

void LevelCelView::on_lastFrameButton_clicked()
{
    this->currentFrameIndex = this->cel->getFrameCount() - 1;
    this->displayFrame();
}

void LevelCelView::on_frameIndexEdit_returnPressed()
{
    quint32 frameIndex = this->ui->frameIndexEdit->text().toUInt() - 1;

    if( frameIndex < this->cel->getFrameCount() )
    {
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
    if( this->currentSubtileIndex >= 1 )
        this->currentSubtileIndex--;
    else
        this->currentSubtileIndex = this->min->getSubtileCount() - 1;

    this->displayFrame();
}

void LevelCelView::on_nextSubtileButton_clicked()
{
    if( this->currentSubtileIndex < this->min->getSubtileCount() - 1 )
        this->currentSubtileIndex++;
    else
        this->currentSubtileIndex = 0;

    this->displayFrame();
}

void LevelCelView::on_lastSubtileButton_clicked()
{
    this->currentSubtileIndex = this->min->getSubtileCount() - 1;
    this->displayFrame();
}

void LevelCelView::on_subtileIndexEdit_returnPressed()
{
    quint16 subtileIndex = this->ui->subtileIndexEdit->text().toUShort() - 1;

    if( subtileIndex < this->min->getSubtileCount() )
    {
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
    if( this->currentTileIndex >= 1 )
        this->currentTileIndex--;
    else
        this->currentTileIndex = this->til->getTileCount() - 1;

    this->displayFrame();
}

void LevelCelView::on_nextTileButton_clicked()
{
    if( this->currentTileIndex < this->til->getTileCount() - 1 )
        this->currentTileIndex++;
    else
        this->currentTileIndex = 0;

    this->displayFrame();
}

void LevelCelView::on_lastTileButton_clicked()
{
    this->currentTileIndex = this->til->getTileCount() - 1;
    this->displayFrame();
}

void LevelCelView::on_tileIndexEdit_returnPressed()
{
    quint16 tileIndex = this->ui->tileIndexEdit->text().toUShort() - 1;

    if( tileIndex < this->til->getTileCount() )
    {
        this->currentTileIndex = tileIndex;
        this->displayFrame();
    }
}

void LevelCelView::on_zoomOutButton_clicked()
{
    if( this->currentZoomFactor - 1 >= 1 )
        this->currentZoomFactor -= 1;
    ui->celGraphicsView->resetTransform();
    ui->celGraphicsView->scale( this->currentZoomFactor, this->currentZoomFactor );
    ui->celGraphicsView->show();
    ui->zoomEdit->setText( QString::number( this->currentZoomFactor ) );
}

void LevelCelView::on_zoomInButton_clicked()
{
    if( this->currentZoomFactor + 1 <= 10 )
        this->currentZoomFactor += 1;
    ui->celGraphicsView->resetTransform();
    ui->celGraphicsView->scale( this->currentZoomFactor, this->currentZoomFactor );
    ui->celGraphicsView->show();
    ui->zoomEdit->setText( QString::number( this->currentZoomFactor ) );
}

void LevelCelView::on_zoomEdit_returnPressed()
{
    quint8 zoom = this->ui->zoomEdit->text().toUShort();

    if( zoom >= 1 && zoom <= 10 )
        this->currentZoomFactor = zoom;
    ui->celGraphicsView->resetTransform();
    ui->celGraphicsView->scale( this->currentZoomFactor, this->currentZoomFactor );
    ui->celGraphicsView->show();
    ui->zoomEdit->setText( QString::number( this->currentZoomFactor ) );
}
