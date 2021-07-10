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

    this->refresh();
}

void PaletteWidget::initialize( D1Pal *p, LevelCelView *lc )
{
    this->pal = p;
    this->isCelLevel = true;
    this->levelCelView = lc;

    this->ui->translationGroupBox->hide();

    this->refresh();
}

void PaletteWidget::initialize( D1Pal *p, D1Trn *t, CelView *c )
{
    this->isTrn = true;
    this->pal = p;
    this->trn = t;
    this->celView = c;

    this->ui->colorGroupBox->setDisabled(true);

    this->refresh();
}

void PaletteWidget::initialize( D1Pal *p, D1Trn *t, LevelCelView *lc )
{
    this->isTrn = true;
    this->pal = p;
    this->trn = t;
    this->isCelLevel = true;
    this->levelCelView = lc;

    this->ui->colorGroupBox->setDisabled(true);

    this->refresh();
}

void PaletteWidget::refresh()
{

    emit refreshed();
}
