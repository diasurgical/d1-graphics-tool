#include "palettewidget.h"
#include "ui_palettewidget.h"

PaletteWidget::PaletteWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaletteWidget)
{
    ui->setupUi(this);
}

PaletteWidget::~PaletteWidget()
{
    delete ui;
}

void PaletteWidget::refresh()
{

}
