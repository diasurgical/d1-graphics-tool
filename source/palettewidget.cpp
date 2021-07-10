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

void PaletteWidget::refresh()
{

}
