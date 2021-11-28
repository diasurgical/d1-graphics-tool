#include "batchexportdialog.h"
#include "ui_batchexportdialog.h"

BatchExportDialog::BatchExportDialog(QJsonObject *config, QWidget *parent) :
    QDialog(parent),
    configuration(config),
    ui(new Ui::BatchExportDialog)
{
    ui->setupUi(this);


    //this->configuration
}

BatchExportDialog::~BatchExportDialog()
{
    delete ui;
}

void BatchExportDialog::setInputFolder(QString path)
{
    ui->inputFolderEdit->setText(path);
}

void BatchExportDialog::exportAllDiabloLevelTilesForTiled()
{
    QString inputFolder = ui->inputFolderEdit->text();
    QString outputFolder = ui->outputFolderEdit->text();

    // Check if cel, special cel, min, til, pal files exist
    if(!Export::checkLevelFilePaths(inputFolder,TOWN_FILE_PATHS)
        || !Export::checkLevelFilePaths(inputFolder,L1_FILE_PATHS)
        || !Export::checkLevelFilePaths(inputFolder,L2_FILE_PATHS)
        || !Export::checkLevelFilePaths(inputFolder,L3_FILE_PATHS)
        || !Export::checkLevelFilePaths(inputFolder,L4_FILE_PATHS))
    {
        QMessageBox::warning( this, "Warning", "Could not find some of the level files." );
        return;
    }



}

void BatchExportDialog::exportAllHellfireLevelTilesForTiled()
{
    QString inputFolder = ui->inputFolderEdit->text();
    QString outputFolder = ui->outputFolderEdit->text();

    // Check if cel, special cel, min, til, pal files exist
    if(!Export::checkLevelFilePaths(inputFolder,TOWN_HELLFIRE_FILE_PATHS)
        || !Export::checkLevelFilePaths(inputFolder,L5_FILE_PATHS)
        || !Export::checkLevelFilePaths(inputFolder,L6_FILE_PATHS))
    {
        QMessageBox::warning( this, "Warning", "Could not find some of the level files." );
        return;
    }

}

void BatchExportDialog::on_inputFolderBrowseButton_clicked()
{
    QString selectedFolder = QFileDialog::getExistingDirectory(
        this, "Select Input Folder", QString(), QFileDialog::ShowDirsOnly );

    ui->inputFolderEdit->setText( selectedFolder );
}

void BatchExportDialog::on_outputFolderBrowseButton_clicked()
{
    QString selectedFolder = QFileDialog::getExistingDirectory(
        this, "Select Output Folder", QString(), QFileDialog::ShowDirsOnly );

    ui->outputFolderEdit->setText( selectedFolder );
}

void BatchExportDialog::on_batchExportButton_clicked()
{
    if(ui->inputFolderEdit->text() == "")
    {
        QMessageBox::warning( this, "Warning", "Input folder is missing, please choose an input folder." );
        return;
    }

    if(ui->outputFolderEdit->text() == "")
    {
        QMessageBox::warning( this, "Warning", "Output folder is missing, please choose an output folder." );
        return;
    }

    // Check radio buttons
    if(ui->exportAllDiabloLevelTilesRadioButton->isChecked())
        exportAllDiabloLevelTilesForTiled();
    else if(ui->exportAllHellfireLevelTilesRadioButton->isChecked())
        exportAllHellfireLevelTilesForTiled();


}

void BatchExportDialog::on_batchExportCancelButton_clicked()
{
    this->close();
}

