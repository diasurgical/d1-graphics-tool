#include "palettewidget.h"
#include "ui_palettewidget.h"

PaletteWidget::PaletteWidget(QWidget *parent) :
    QWidget(parent),
    isCelLevel( false ),
    isTrn( false ),
    scene( new QGraphicsScene(0,0,PALETTE_DEFAULT_WIDTH,PALETTE_DEFAULT_WIDTH) ),
    ui(new Ui::PaletteWidget)
{
    ui->setupUi(this);
    ui->graphicsView->setScene( this->scene );
}

PaletteWidget::~PaletteWidget()
{
    delete ui;
}

void PaletteWidget::initialize( D1Pal *p, CelView *c )
{
    this->pal = p;
    this->celView = c;

    this->ui->translationGroupBox->hide();

    this->displayColors();
    //this->refresh();
}

void PaletteWidget::initialize( D1Pal *p, LevelCelView *lc )
{
    this->pal = p;
    this->isCelLevel = true;
    this->levelCelView = lc;

    this->ui->translationGroupBox->hide();

    this->displayColors();
    //this->refresh();
}

void PaletteWidget::initialize( D1Pal *p, D1Trn *t, CelView *c )
{
    this->isTrn = true;
    this->pal = p;
    this->trn = t;
    this->celView = c;

    this->ui->colorGroupBox->hide();

    this->displayColors();
    //this->refresh();
}

void PaletteWidget::initialize( D1Pal *p, D1Trn *t, LevelCelView *lc )
{
    this->isTrn = true;
    this->pal = p;
    this->trn = t;
    this->isCelLevel = true;
    this->levelCelView = lc;

    this->ui->colorGroupBox->hide();

    this->displayColors();
    //this->refresh();
}

void PaletteWidget::displayColors()
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
    this->scene->clear();

    // Setting background color
    this->scene->setBackgroundBrush( Qt::white );

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
        this->scene->addRect(x,y,w,w,pen,brush);

        x += dx;
    }
/*
    // This boolean is used to avoid infinite loop when adding items to the combo box
    // because adding items calls on_palComboBox_currentIndexChanged() which itself calls
    // displayPal() which calls on_palComboBox_currentIndexChanged(), ...
    this->buildingPalComboBox = true;
    ui->palComboBox->clear();
    for( int i = 0; i < this->palettesPaths.keys().size(); i++ )
        ui->palComboBox->addItem( this->palettesPaths.keys().at(i) );
    ui->palComboBox->setCurrentText( this->palettesPaths.key(this->pal->getFilePath()) );
    this->buildingPalComboBox = false;
*/
}

void PaletteWidget::refresh()
{
    this->displayColors();
    emit refreshed();
}
