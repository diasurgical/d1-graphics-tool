#include "batchexportdialog.h"
#include "ui_batchexportdialog.h"

BatchExportDialog::BatchExportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BatchExportDialog)
{
    ui->setupUi(this);
}

BatchExportDialog::~BatchExportDialog()
{
    delete ui;
}

void BatchExportDialog::on_batchExportButton_clicked()
{

}

