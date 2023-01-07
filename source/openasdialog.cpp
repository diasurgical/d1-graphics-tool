#include "openasdialog.h"

#include <QFileDialog>
#include <QMessageBox>

#include "ui_openasdialog.h"

#include "mainwindow.h"

OpenAsDialog::OpenAsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OpenAsDialog)
{
    ui->setupUi(this);
}

OpenAsDialog::~OpenAsDialog()
{
    delete ui;
}

void OpenAsDialog::initialize(QJsonObject *cfg)
{
    // initialize the configuration pointer
    this->configuration = cfg;

    // clear the input fields
    ui->inputFileEdit->setText("");
    ui->isTilesetAutoRadioButton->setChecked(true);
    // - celSettingsGroupBox
    ui->celWidthEdit->setText("0");
    ui->celClippedAutoRadioButton->setChecked(true);
    // - tilSettingsGroupBox
    ui->tilFileEdit->setText("");
    ui->minFileEdit->setText("");
    ui->solFileEdit->setText("");
    ui->ampFileEdit->setText("");
    ui->minWidthEdit->setText("0");
    ui->minHeightEdit->setText("0");

    this->update();
}

void OpenAsDialog::update()
{
    bool hasInputFile = !ui->inputFileEdit->text().isEmpty();
    bool isTileset = ui->isTilesetYesRadioButton->isChecked() || (ui->isTilesetAutoRadioButton->isChecked() && !ui->tilFileEdit->text().isEmpty());

    ui->celSettingsGroupBox->setEnabled(hasInputFile && !isTileset);
    ui->tilSettingsGroupBox->setEnabled(hasInputFile && isTileset);
}

void OpenAsDialog::on_inputFileBrowseButton_clicked()
{
    MainWindow *qw = (MainWindow *)this->parentWidget();
    QString openFilePath = qw->fileDialog(false, "Open Graphics", "CEL/CL2 Files (*.cel *.CEL *.cl2 *.CL2)"); // "CEL/CL2/CLX Files (*.cel *.CEL *.cl2 *.CL2 *.clx *.CLX);;TIL Files (*.til *.TIL)");

    if (openFilePath.isEmpty())
        return;

    ui->inputFileEdit->setText(openFilePath);
    // activate optional fields based on the extension
    if (ui->isTilesetAutoRadioButton->isChecked()) {
        bool isTileset = false;
        QString basePath;
        QString tilFilePath;
        QString minFilePath;
        QString solFilePath;
        if (openFilePath.toLower().endsWith(".cel")) {
            // If a SOL, MIN and TIL files exists then preset them
            basePath = openFilePath;
            basePath.chop(3);
            tilFilePath = basePath + "til";
            minFilePath = basePath + "min";
            solFilePath = basePath + "sol";
            isTileset = QFileInfo::exists(tilFilePath) && QFileInfo::exists(minFilePath) && QFileInfo::exists(solFilePath);
        }
        if (isTileset) {
            ui->tilFileEdit->setText(tilFilePath);
            ui->minFileEdit->setText(minFilePath);
            ui->solFileEdit->setText(solFilePath);
            QString ampFilePath = basePath + "amp";
            if (QFileInfo::exists(ampFilePath)) {
                ui->ampFileEdit->setText(ampFilePath);
            } else {
                ui->ampFileEdit->setText("");
            }
        } else {
            ui->tilFileEdit->setText("");
            ui->minFileEdit->setText("");
            ui->solFileEdit->setText("");
            ui->ampFileEdit->setText("");
        }
    }

    this->update();
}

void OpenAsDialog::on_isTilesetYesRadioButton_toggled(bool checked)
{
    this->update();
}

void OpenAsDialog::on_isTilesetNoRadioButton_toggled(bool checked)
{
    this->update();
}

void OpenAsDialog::on_isTilesetAutoRadioButton_toggled(bool checked)
{
    this->update();
}

void OpenAsDialog::on_tilFileBrowseButton_clicked()
{
    MainWindow *qw = (MainWindow *)this->parentWidget();
    QString openFilePath = qw->fileDialog(false, "Open TIL file", "TIL Files (*.til *.TIL)");

    if (openFilePath.isEmpty())
        return;

    ui->tilFileEdit->setText(openFilePath);
}

void OpenAsDialog::on_minFileBrowseButton_clicked()
{
    MainWindow *qw = (MainWindow *)this->parentWidget();
    QString openFilePath = qw->fileDialog(false, "Open MIN file", "MIN Files (*.min *.MIN)");

    if (openFilePath.isEmpty())
        return;

    ui->minFileEdit->setText(openFilePath);
}

void OpenAsDialog::on_solFileBrowseButton_clicked()
{
    MainWindow *qw = (MainWindow *)this->parentWidget();
    QString openFilePath = qw->fileDialog(false, "Open SOL file", "SOL Files (*.sol *.SOL)");

    if (openFilePath.isEmpty())
        return;

    ui->solFileEdit->setText(openFilePath);
}

void OpenAsDialog::on_ampFileBrowseButton_clicked()
{
    MainWindow *qw = (MainWindow *)this->parentWidget();
    QString openFilePath = qw->fileDialog(false, "Open AMP file", "AMP Files (*.amp *.AMP)");

    if (openFilePath.isEmpty())
        return;

    ui->ampFileEdit->setText(openFilePath);
}

void OpenAsDialog::on_openButton_clicked()
{
    OpenAsParam params;
    params.celFilePath = ui->inputFileEdit->text();
    if (params.celFilePath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Input file is missing, please choose an input file.");
        return;
    }
    if (ui->isTilesetYesRadioButton->isChecked()) {
        params.isTileset = OPEN_TILESET_TYPE::TILESET_TRUE;
    } else if (ui->isTilesetNoRadioButton->isChecked()) {
        params.isTileset = OPEN_TILESET_TYPE::TILESET_FALSE;
    } else {
        params.isTileset = OPEN_TILESET_TYPE::TILESET_AUTODETECT;
    }
    // cel/cl2: clipped, width
    params.celWidth = this->ui->celWidthEdit->text().toUShort();
    if (ui->celClippedYesRadioButton->isChecked()) {
        params.clipped = OPEN_CLIPPING_TYPE::CLIPPED_TRUE;
    } else if (ui->celClippedNoRadioButton->isChecked()) {
        params.clipped = OPEN_CLIPPING_TYPE::CLIPPED_FALSE;
    } else {
        params.clipped = OPEN_CLIPPING_TYPE::CLIPPED_AUTODETECT;
    }
    params.tilFilePath = ui->tilFileEdit->text();
    params.minFilePath = ui->minFileEdit->text();
    params.solFilePath = ui->solFileEdit->text();
    params.ampFilePath = ui->ampFileEdit->text();
    params.minWidth = ui->minWidthEdit->text().toUShort();
    params.minHeight = ui->minHeightEdit->text().toUShort();

    MainWindow *qw = (MainWindow *)this->parentWidget();
    this->close();

    qw->openFile(params);
}

void OpenAsDialog::on_openCancelButton_clicked()
{
    this->close();
}
