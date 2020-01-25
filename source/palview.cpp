#include "palview.h"
#include "ui_palview.h"

PalView::PalView(QWidget *parent) :
    QWidget(parent),
    ui( new Ui::PalView ),
    isCelLevel( false ),
    palScene( new QGraphicsScene(0,0,PALETTE_DEFAULT_WIDTH,PALETTE_DEFAULT_WIDTH) ),
    palHitsScene( new QGraphicsScene(0,0,PALETTE_DEFAULT_WIDTH,PALETTE_DEFAULT_WIDTH) ),
    trn1Scene( new QGraphicsScene(0,0,PALETTE_DEFAULT_WIDTH,PALETTE_DEFAULT_WIDTH) ),
    trn2Scene( new QGraphicsScene(0,0,PALETTE_DEFAULT_WIDTH,PALETTE_DEFAULT_WIDTH) ),
    trn1HitsScene( new QGraphicsScene(0,0,PALETTE_DEFAULT_WIDTH,PALETTE_DEFAULT_WIDTH) ),
    trn2HitsScene( new QGraphicsScene(0,0,PALETTE_DEFAULT_WIDTH,PALETTE_DEFAULT_WIDTH) ),
    buildingPalComboBox( false ),
    buildingTrnComboBox( false )
{
    ui->setupUi(this);
    ui->palGraphicsView->setScene( this->palScene );
    ui->palHitsGraphicsView->setScene( this->palHitsScene );
    ui->trn1GraphicsView->setScene( this->trn1Scene );
    ui->trn2GraphicsView->setScene( this->trn2Scene );
    ui->trn1HitsGraphicsView->setScene( this->trn1HitsScene );
    ui->trn2HitsGraphicsView->setScene( this->trn2HitsScene );
}

PalView::~PalView()
{
    delete ui;
}

void PalView::initialize( D1Pal* p, D1Trn* t1, D1Trn* t2, CelView* c )
{
    this->pal = p;
    this->trn1 = t1;
    this->trn2 = t2;
    this->celView = c;

    this->refreshPalettesPathsAndNames();
    this->refreshTranslationsPathsAndNames();

    this->buildPalHits();

    this->refreshPaletteHitsNames();
}

void PalView::initialize( D1Pal* p, D1Trn* t1, D1Trn* t2, LevelCelView* lc )
{
    this->pal = p;
    this->trn1 = t1;
    this->trn2 = t2;
    this->isCelLevel = true;
    this->levelCelView = lc;

    this->refreshPalettesPathsAndNames();
    this->refreshTranslationsPathsAndNames();

    this->buildPalHits();
    this->buildSubtilePalHits();
    this->buildTilePalHits();

    this->refreshPaletteHitsNames();
}

void PalView::buildPalHits()
{
    D1CelBase* cel = NULL;
    D1CelFrameBase* frame = NULL;
    quint8 paletteIndex = 0;

    // Retrieve the CEL/CL2
    if( this->isCelLevel )
        cel = this->levelCelView->getCel();
    else
        cel = this->celView->getCel();

    // Go through all frames
    for( quint32 i = 0; i < cel->getFrameCount(); i++ )
    {
        QMap<quint8,quint32> frameHits;

        // Get frame pointer
        frame = cel->getFrame(i);

        // Go through every pixels of the frame
        for( int jx = 0; jx < frame->getWidth(); jx++ )
        {
            for( int jy = 0; jy < frame->getHeight(); jy++ )
            {
                // Retrieve the color of the pixel
                paletteIndex = frame->getPixel(jx,jy).getPaletteIndex();

                // Add one hit to the frameHits and allFramesPalHits maps
                if( frameHits.contains(paletteIndex) )
                    frameHits.insert(paletteIndex,frameHits.value(paletteIndex)+1);
                else
                    frameHits.insert(paletteIndex,1);

                if( frameHits.contains(paletteIndex) )
                    this->allFramesPalHits.insert(paletteIndex,frameHits.value(paletteIndex)+1);
                else
                    this->allFramesPalHits.insert(paletteIndex,1);
            }
        }

        this->framePalHits[i] = frameHits;
    }
}

void PalView::buildSubtilePalHits()
{
    D1Min* min = this->levelCelView->getMin();
    QList<quint16> celFrameIndices;
    quint16 frameIndex;

    // Go through all sub-tiles
    for( int i = 0; i < min->getSubtileCount(); i++ )
    {
        QMap<quint8,quint32> subtileHits;

        // Retrieve the CEL frame indices of the current sub-tile
        celFrameIndices = min->getCelFrameIndices( i );

        // Go through the CEL frames
        QListIterator<quint16> it1( celFrameIndices );
        while( it1.hasNext() )
        {
            frameIndex = it1.next() - 1;

            // Go through the hits of the CEL frame and add them to the subtile hits
            QMapIterator<quint8,quint32> it2( this->framePalHits.value(frameIndex) );
            while( it2.hasNext() )
            {
                it2.next();
                subtileHits.insert( it2.key(), it2.value() );
            }
        }

        this->subtilePalHits[i] = subtileHits;
    }
}

void PalView::buildTilePalHits()
{
    D1Til* til = this->levelCelView->getTil();
    QList<quint16> subtileIndices;
    quint16 subtileIndex;

    // Go through all tiles
    for( int i = 0; i < til->getTileCount(); i++ )
    {
        QMap<quint8,quint32> tileHits;

        // Retrieve the sub-tile indices of the current tile
        subtileIndices = til->getSubtileIndices( i );

        // Go through the sub-tiles
        QListIterator<quint16> it1( subtileIndices );
        while( it1.hasNext() )
        {
            subtileIndex = it1.next();

            // Go through the hits of the sub-tile and add them to the tile hits
            QMapIterator<quint8,quint32> it2( this->subtilePalHits.value(subtileIndex) );
            while( it2.hasNext() )
            {
                it2.next();
                tileHits.insert( it2.key(), it2.value() );
            }
        }

        this->tilePalHits[i] = tileHits;
    }
}

void PalView::displayPal()
{
    // Positions
    int x = 0, y = 0;

    // X delta
    int dx = PALETTE_DEFAULT_WIDTH/16;
    // Y delta
    int dy = PALETTE_DEFAULT_WIDTH/16;

    // Color width (-1 is for QRect border)
    int w = PALETTE_DEFAULT_WIDTH/16 - 1;

    // Removing existing items
    this->palScene->clear();

    // Setting background color
    this->palScene->setBackgroundBrush( Qt::white );

    // Displaying palette colors
    for( int i = 0; i < D1PAL_COLORS; i++ )
    {
        if( i%16 == 0 && i != 0 )
        {
            x = 0;
            y += dy;
        }

        QBrush brush( this->pal->getColor(i) );
        QPen pen( Qt::white );
        this->palScene->addRect(x,y,w,w,pen,brush);

        x += dx;
    }

    // This boolean is used to avoid infinite loop when adding items to the combo box
    // because adding items calls on_palComboBox_currentIndexChanged() which itself calls
    // displayPal() which calls on_palComboBox_currentIndexChanged(), ...
    this->buildingPalComboBox = true;
    ui->palComboBox->clear();
    for( int i = 0; i < this->palettesPaths.keys().size(); i++ )
        ui->palComboBox->addItem( this->palettesPaths.keys().at(i) );
    ui->palComboBox->setCurrentText( this->palettesPaths.key(this->pal->getFilePath()) );
    this->buildingPalComboBox = false;
}

void PalView::displayTrn()
{
    // Positions
    int x = 0, y = 0;

    // X delta
    int dx = PALETTE_DEFAULT_WIDTH/16;
    // Y delta
    int dy = PALETTE_DEFAULT_WIDTH/16;

    // Color width (-1 is for QRect border)
    int w = PALETTE_DEFAULT_WIDTH/16 - 1;

    // Removing existing items
    this->trn1Scene->clear();
    this->trn2Scene->clear();

    // Setting background color
    this->trn1Scene->setBackgroundBrush( Qt::white );
    this->trn2Scene->setBackgroundBrush( Qt::white );

    // Displaying palette colors
    QBrush brush;
    QPen pen( Qt::white );
    for( int i = 0; i < D1PAL_COLORS; i++ )
    {
        if( i%16 == 0 && i != 0 )
        {
            x = 0;
            y += dy;
        }

        brush = QBrush( this->trn1->getResultingPalette()->getColor(i) );
        this->trn1Scene->addRect(x,y,w,w,pen,brush);

        brush = QBrush( this->trn2->getResultingPalette()->getColor(i) );
        this->trn2Scene->addRect(x,y,w,w,pen,brush);

        x += dx;
    }
/*
    // Displaying TRN file path information
    QFileInfo trn1FileInfo( this->trn1->getFilePath() );
    this->ui->trn1Label->setText( trn1FileInfo.fileName() );
    QFileInfo trn2FileInfo( this->trn2->getFilePath() );
    this->ui->trn2Label->setText( trn2FileInfo.fileName() );*/

    // This boolean is used to avoid infinite loop when adding items to the combo box
    // because adding items calls on_trnComboBox_currentIndexChanged() which itself calls
    // displayPal() which calls on_trnComboBox_currentIndexChanged(), ...
    this->buildingTrnComboBox = true;
    ui->trn1ComboBox->clear();
    ui->trn2ComboBox->clear();
    for( int i = 0; i < this->translationsPaths.keys().size(); i++ )
    {
        ui->trn1ComboBox->addItem( this->translationsPaths.keys().at(i) );
        ui->trn2ComboBox->addItem( this->translationsPaths.keys().at(i) );
    }
    ui->trn1ComboBox->setCurrentText( this->translationsPaths.key( this->trn1->getFilePath() ) );
    ui->trn2ComboBox->setCurrentText( this->translationsPaths.key( this->trn2->getFilePath() ) );
    this->buildingTrnComboBox = false;
}

void PalView::refreshPalettesPathsAndNames()
{
    // Add builtin palettes from the resource file
    this->palettesPaths.clear();
    this->palettesPaths["_default.pal"] = ":/default.pal";
    this->palettesPaths["_town.pal"] = ":/town.pal";

    // If the selected palette is not a builtin palette get the palette file path
    if( !this->pal->getFilePath().startsWith(":/") )
    {
        QFileInfo palFileInfo( this->pal->getFilePath() );
        this->palettesPaths[ palFileInfo.fileName() ] = this->pal->getFilePath();
    }

    // Look for all palettes in the same folder than the CEL/CL2 file
    QFileInfo celFileInfo;
    if( this->isCelLevel )
        celFileInfo = QFileInfo( this->levelCelView->getCelPath() );
    else
        celFileInfo = QFileInfo( this->celView->getCelPath() );
    QDirIterator it( celFileInfo.absolutePath(), QStringList() << "*.pal", QDir::Files );
    QString firstPaletteFound = QString();
    bool relevantPaletteFound = false;
    while( it.hasNext() )
    {
        QString path = it.next();

        if( path != "1" )
        {
            if( firstPaletteFound.isEmpty() )
                firstPaletteFound = path;

            QFileInfo palFileInfo( path );
            this->palettesPaths[ palFileInfo.fileName() ] = palFileInfo.absoluteFilePath();

            if( !relevantPaletteFound
                && palFileInfo.fileName().toLower().startsWith(
                celFileInfo.fileName().toLower().replace(".cel","").replace(".cl2","") ) )
            {
                if( !this->pal->load( palFileInfo.absoluteFilePath() ) )
                {
                    QMessageBox::critical( this, "Error", "Could not load PAL file." );
                    return;
                }

                this->trn1->refreshResultingPalette();
                this->trn2->refreshResultingPalette();

                relevantPaletteFound = true;
            }

            if( !it.hasNext() && !relevantPaletteFound )
            {
                if( !this->pal->load( firstPaletteFound ) )
                {
                    QMessageBox::critical( this, "Error", "Could not load PAL file." );
                    return;
                }
                this->trn1->refreshResultingPalette();
                this->trn2->refreshResultingPalette();
            }
        }
    }
}

void PalView::refreshPaletteHitsNames()
{
    // Add items to the palette hits combo box
    this->ui->palHitsComboBox->clear();
    this->ui->palHitsComboBox->addItem("All frames");

    if( this->isCelLevel )
    {
        this->ui->palHitsComboBox->addItem("Current tile");
        this->ui->palHitsComboBox->addItem("Current sub-tile");
    }

    this->ui->palHitsComboBox->addItem("Current frame");
    this->ui->palHitsComboBox->setCurrentText("All frames");
}

void PalView::refreshTranslationsPathsAndNames()
{
    this->translationsPaths.clear();
    this->translationsPaths["_null.trn"] = ":/null.trn";

    if( !this->trn1->getFilePath().startsWith(":/") )
    {
        QFileInfo trn1FileInfo( this->trn1->getFilePath() );
        this->translationsPaths[ trn1FileInfo.fileName() ] = this->trn1->getFilePath();
    }

    if( !this->trn2->getFilePath().startsWith(":/") )
    {
        QFileInfo trn2FileInfo( this->trn2->getFilePath() );
        this->translationsPaths[ trn2FileInfo.fileName() ] = this->trn2->getFilePath();
    }

    // Look for all translations in the same folder than the CEL/CL2 file
    QFileInfo celFileInfo;
    if( this->isCelLevel )
        celFileInfo = QFileInfo( this->levelCelView->getCelPath() );
    else
        celFileInfo = QFileInfo( this->celView->getCelPath() );
    QDirIterator it( celFileInfo.absolutePath(), QStringList() << "*.trn", QDir::Files );
    QString firstTranslationFound = QString();
    while( it.hasNext() )
    {
        QString path = it.next();
        if( firstTranslationFound.isEmpty() )
            firstTranslationFound = path;

        if( path != "1" )
        {
            QFileInfo trnFileInfo( path );
            this->translationsPaths[ trnFileInfo.fileName() ] = trnFileInfo.absoluteFilePath();
        }
    }
}

void PalView::displayPalHits()
{
    if( this->ui->palHitsComboBox->currentText() == "All frames" )
        this->displayAllFramesPalHits();
    else if( this->ui->palHitsComboBox->currentText() == "Current tile" )
        this->displayCurrentTilePalHits();
    else if( this->ui->palHitsComboBox->currentText() == "Current sub-tile" )
        this->displayCurrentSubtilePalHits();
    else if( this->ui->palHitsComboBox->currentText() == "Current frame" )
        this->displayCurrentFramePalHits();
}

void PalView::displayAllFramesPalHits()
{
    // Positions
    int x = 0, y = 0;

    // X delta
    int dx = PALETTE_DEFAULT_WIDTH/16;
    // Y delta
    int dy = PALETTE_DEFAULT_WIDTH/16;

    // Color width (-1 is for QRect border)
    int w = PALETTE_DEFAULT_WIDTH/16 - 1;

    // Palette index
    quint8 paletteIndex = 0;

    // Removing existing items
    this->palHitsScene->clear();
    // Setting background color
    this->palHitsScene->setBackgroundBrush( Qt::black );

    // Go through all hits of the current frame
    QMapIterator<quint8,quint32> it(this->allFramesPalHits);
    while( it.hasNext() )
    {
        it.next();

        paletteIndex = it.key();

        // Compute coordinates
        if( paletteIndex == 0 )
        {
            x = 0;
            y = 0;
        }
        else
        {
            x = (paletteIndex%16) * dx;
            y = (paletteIndex/16) * dy;
        }

        QBrush brush( this->pal->getColor(paletteIndex) );
        QPen pen( Qt::white );
        this->palHitsScene->addRect(x,y,w,w,pen,brush);
    }
}

void PalView::displayCurrentTilePalHits()
{
    // Positions
    int x = 0, y = 0;

    // X delta
    int dx = PALETTE_DEFAULT_WIDTH/16;
    // Y delta
    int dy = PALETTE_DEFAULT_WIDTH/16;

    // Color width (-1 is for QRect border)
    int w = PALETTE_DEFAULT_WIDTH/16 - 1;

    // Palette index
    quint8 paletteIndex = 0;

    // Frame hits map
    QMap<quint8,quint32> currentTileHits;

    // Removing existing items
    this->palHitsScene->clear();
    // Setting background color
    this->palHitsScene->setBackgroundBrush( Qt::black );

    // Get the current frame hits
    currentTileHits = this->tilePalHits[this->levelCelView->getCurrentTileIndex()];

    // Go through all hits of the current frame
    QMapIterator<quint8,quint32> it(currentTileHits);
    while( it.hasNext() )
    {
        it.next();

        paletteIndex = it.key();

        // Compute coordinates
        if( paletteIndex == 0 )
        {
            x = 0;
            y = 0;
        }
        else
        {
            x = (paletteIndex%16) * dx;
            y = (paletteIndex/16) * dy;
        }

        QBrush brush( this->pal->getColor(paletteIndex) );
        QPen pen( Qt::white );
        this->palHitsScene->addRect(x,y,w,w,pen,brush);
    }
}

void PalView::displayCurrentSubtilePalHits()
{
    // Positions
    int x = 0, y = 0;

    // X delta
    int dx = PALETTE_DEFAULT_WIDTH/16;
    // Y delta
    int dy = PALETTE_DEFAULT_WIDTH/16;

    // Color width (-1 is for QRect border)
    int w = PALETTE_DEFAULT_WIDTH/16 - 1;

    // Palette index
    quint8 paletteIndex = 0;

    // Frame hits map
    QMap<quint8,quint32> currenSubtileHits;

    // Removing existing items
    this->palHitsScene->clear();
    // Setting background color
    this->palHitsScene->setBackgroundBrush( Qt::black );

    // Get the current frame hits
    currenSubtileHits = this->subtilePalHits[this->levelCelView->getCurrentSubtileIndex()];

    // Go through all hits of the current frame
    QMapIterator<quint8,quint32> it(currenSubtileHits);
    while( it.hasNext() )
    {
        it.next();

        paletteIndex = it.key();

        // Compute coordinates
        if( paletteIndex == 0 )
        {
            x = 0;
            y = 0;
        }
        else
        {
            x = (paletteIndex%16) * dx;
            y = (paletteIndex/16) * dy;
        }

        QBrush brush( this->pal->getColor(paletteIndex) );
        QPen pen( Qt::white );
        this->palHitsScene->addRect(x,y,w,w,pen,brush);
    }
}

void PalView::displayCurrentFramePalHits()
{
    // Positions
    int x = 0, y = 0;

    // X delta
    int dx = PALETTE_DEFAULT_WIDTH/16;
    // Y delta
    int dy = PALETTE_DEFAULT_WIDTH/16;

    // Color width (-1 is for QRect border)
    int w = PALETTE_DEFAULT_WIDTH/16 - 1;

    // Palette index
    quint8 paletteIndex = 0;

    // Frame hits map
    QMap<quint8,quint32> currentFrameHits;

    // Removing existing items
    this->palHitsScene->clear();
    // Setting background color
    this->palHitsScene->setBackgroundBrush( Qt::black );

    // Get the current frame hits
    if( this->isCelLevel )
        currentFrameHits = this->framePalHits[this->levelCelView->getCurrentFrameIndex()];
    else
        currentFrameHits = this->framePalHits[this->celView->getCurrentFrameIndex()];

    // Go through all hits of the current frame
    QMapIterator<quint8,quint32> it(currentFrameHits);
    while( it.hasNext() )
    {
        it.next();

        paletteIndex = it.key();

        // Compute coordinates
        if( paletteIndex == 0 )
        {
            x = 0;
            y = 0;
        }
        else
        {
            x = (paletteIndex%16) * dx;
            y = (paletteIndex/16) * dy;
        }

        QBrush brush( this->pal->getColor(paletteIndex) );
        QPen pen( Qt::white );
        this->palHitsScene->addRect(x,y,w,w,pen,brush);
    }
}

void PalView::displayTrnHits()
{
    if( this->ui->palHitsComboBox->currentText() == "All frames" )
        this->displayAllFramesTrnHits();
    else if( this->ui->palHitsComboBox->currentText() == "Current tile" )
        this->displayCurrentTileTrnHits();
    else if( this->ui->palHitsComboBox->currentText() == "Current sub-tile" )
        this->displayCurrentSubtileTrnHits();
    else if( this->ui->palHitsComboBox->currentText() == "Current frame" )
        this->displayCurrentFrameTrnHits();
}

void PalView::displayAllFramesTrnHits()
{
    // Positions
    int x = 0, y = 0;

    // X delta
    int dx = PALETTE_DEFAULT_WIDTH/16;
    // Y delta
    int dy = PALETTE_DEFAULT_WIDTH/16;

    // Color width (-1 is for QRect border)
    int w = PALETTE_DEFAULT_WIDTH/16 - 1;

    // Palette index
    quint8 paletteIndex = 0;

    // Removing existing items
    this->trn1HitsScene->clear();
    this->trn2HitsScene->clear();
    // Setting background color
    this->trn1HitsScene->setBackgroundBrush( Qt::black );
    this->trn2HitsScene->setBackgroundBrush( Qt::black );

    // Go through all hits of the current frame
    QMapIterator<quint8,quint32> it(this->allFramesPalHits);
    while( it.hasNext() )
    {
        it.next();

        paletteIndex = it.key();

        // Compute coordinates
        if( paletteIndex == 0 )
        {
            x = 0;
            y = 0;
        }
        else
        {
            x = (paletteIndex%16) * dx;
            y = (paletteIndex/16) * dy;
        }

        QPen pen( Qt::white );

        QBrush brush1( this->trn1->getResultingPalette()->getColor(paletteIndex) );
        this->trn1HitsScene->addRect(x,y,w,w,pen,brush1);

        QBrush brush2( this->trn2->getResultingPalette()->getColor(paletteIndex) );
        this->trn2HitsScene->addRect(x,y,w,w,pen,brush2);
    }
}

void PalView::displayCurrentTileTrnHits()
{

}

void PalView::displayCurrentSubtileTrnHits()
{

}

void PalView::displayCurrentFrameTrnHits()
{
    // Positions
    int x = 0, y = 0;

    // X delta
    int dx = PALETTE_DEFAULT_WIDTH/16;
    // Y delta
    int dy = PALETTE_DEFAULT_WIDTH/16;

    // Color width (-1 is for QRect border)
    int w = PALETTE_DEFAULT_WIDTH/16 - 1;

    // Palette index
    quint8 paletteIndex = 0;

    // Frame hits map
    QMap<quint8,quint32> currentFrameHits;

    // Removing existing items
    this->trn1HitsScene->clear();
    this->trn2HitsScene->clear();
    // Setting background color
    this->trn1HitsScene->setBackgroundBrush( Qt::black );
    this->trn2HitsScene->setBackgroundBrush( Qt::black );

    // Get the current frame hits
    if( this->isCelLevel )
        currentFrameHits = this->framePalHits[this->levelCelView->getCurrentFrameIndex()];
    else
        currentFrameHits = this->framePalHits[this->celView->getCurrentFrameIndex()];

    // Go through all hits of the current frame
    QMapIterator<quint8,quint32> it(currentFrameHits);
    while( it.hasNext() )
    {
        it.next();

        paletteIndex = it.key();

        // Compute coordinates
        if( paletteIndex == 0 )
        {
            x = 0;
            y = 0;
        }
        else
        {
            x = (paletteIndex%16) * dx;
            y = (paletteIndex/16) * dy;
        }

        QPen pen( Qt::white );

        QBrush brush1( this->trn1->getResultingPalette()->getColor(paletteIndex) );
        this->trn1HitsScene->addRect(x,y,w,w,pen,brush1);

        QBrush brush2( this->trn2->getResultingPalette()->getColor(paletteIndex) );
        this->trn2HitsScene->addRect(x,y,w,w,pen,brush2);
    }
}

void PalView::on_palComboBox_currentIndexChanged(const QString &arg1)
{
    if( this->palettesPaths.isEmpty() || this->buildingPalComboBox )
        return;

    QString palFilePath = this->palettesPaths[arg1];
    if( !palFilePath.isEmpty() )
    {
        if( !this->pal->load( palFilePath ) )
        {
            QMessageBox::critical( this, "Error", "Could not load PAL file." );
            return;
        }
    }

    this->trn1->refreshResultingPalette();
    this->trn2->refreshResultingPalette();

    this->displayPal();
    this->displayTrn();

    this->displayPalHits();

    if( this->isCelLevel )
        this->levelCelView->displayFrame();
    else
        this->celView->displayFrame();
}

void PalView::on_trn1ComboBox_currentIndexChanged(const QString &arg1)
{
    if( this->translationsPaths.isEmpty() || this->buildingTrnComboBox )
        return;

    QString trnFilePath = this->translationsPaths[arg1];
    if( !trnFilePath.isEmpty() )
    {
        if( !this->trn1->load( trnFilePath ) )
        {
            QMessageBox::critical( this, "Error", "Could not load TRN file." );
            return;
        }
    }

    this->trn1->refreshResultingPalette();
    this->trn2->refreshResultingPalette();

    this->displayTrn();

    if( this->isCelLevel )
        this->levelCelView->displayFrame();
    else
        this->celView->displayFrame();
}

void PalView::on_trn2ComboBox_currentIndexChanged(const QString &arg1)
{
    if( this->translationsPaths.isEmpty() || this->buildingTrnComboBox )
        return;

    QString trnFilePath = this->translationsPaths[arg1];
    if( !trnFilePath.isEmpty() )
    {
        if( !this->trn2->load( trnFilePath ) )
        {
            QMessageBox::critical( this, "Error", "Could not load TRN file." );
            return;
        }
    }

    this->trn2->refreshResultingPalette();

    this->displayTrn();

    if( this->isCelLevel )
        this->levelCelView->displayFrame();
    else
        this->celView->displayFrame();
}

void PalView::on_palHitsComboBox_currentIndexChanged()
{
    this->displayPalHits();
    this->displayTrnHits();
}
