#include "palview.h"
#include "ui_palview.h"

PalView::PalView(QWidget *parent) :
    QWidget(parent),
    ui( new Ui::PalView ),
    isCelLevel( false ),
    palScene( new QGraphicsScene(0,0,PALETTE_DEFAULT_WIDTH,PALETTE_DEFAULT_WIDTH) ),
    trn1Scene( new QGraphicsScene(0,0,PALETTE_DEFAULT_WIDTH,PALETTE_DEFAULT_WIDTH) ),
    trn2Scene( new QGraphicsScene(0,0,PALETTE_DEFAULT_WIDTH,PALETTE_DEFAULT_WIDTH) ),
    buildingPalComboBox( false ),
    buildingTrnComboBox( false )
{
    ui->setupUi(this);
    ui->palGraphicsView->setScene( this->palScene );
    ui->trn1GraphicsView->setScene( this->trn1Scene );
    ui->trn2GraphicsView->setScene( this->trn2Scene );
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
        QPen pen( Qt::black );
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
    QPen pen( Qt::black );
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
    this->palettesPaths.clear();
    this->palettesPaths["Default town.pal"] = ":/town.pal";

    if( this->pal->getFilePath() != ":/town.pal" )
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

void PalView::refreshTranslationsPathsAndNames()
{
    this->translationsPaths.clear();
    this->translationsPaths["Default null.trn"] = ":/null.trn";

    if( this->trn1->getFilePath() != ":/null.trn" )
    {
        QFileInfo trn1FileInfo( this->trn1->getFilePath() );
        this->translationsPaths[ trn1FileInfo.fileName() ] = this->trn1->getFilePath();
    }

    if( this->trn2->getFilePath() != ":/null.trn" )
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
