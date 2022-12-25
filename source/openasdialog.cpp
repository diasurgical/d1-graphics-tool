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
    // - celSettingsGroupBox
    ui->celWidthEdit->setText("0");
    ui->celClippedAutoRadioButton->setChecked(true);

    this->update();
}

void OpenAsDialog::update()
{
    QString filePath = ui->inputFileEdit->text();

    // activate optional fields based on the extension
    if (filePath.toLower().endsWith(".cel") || filePath.toLower().endsWith(".cl2")) {
        ui->celSettingsGroupBox->setEnabled(true);
        // ui->clxSettingsWidget->setEnabled(false);
        // ui->tilSettingsWidget->setEnabled(false);
        return;
    }
    /*if (filePath.toLower().endsWith(".clx")) {
        ui->celSettingsGroupBox->setEnabled(false);
        ui->clxSettingsWidget->setEnabled(true);
        ui->tilSettingsWidget->setEnabled(false);
        return;
    }
    if (filePath.toLower().endsWith(".til")) {
        ui->celSettingsGroupBox->setEnabled(false);
        ui->clxSettingsWidget->setEnabled(false);
        ui->tilSettingsWidget->setEnabled(true);
        return;
    }*/

    // empty or invalid selection -> disable the dialog fields
    ui->celSettingsGroupBox->setEnabled(false);
    // ui->clxSettingsWidget->setEnabled(false);
    // ui->tilSettingsWidget->setEnabled(false);
}

void OpenAsDialog::on_inputFileBrowseButton_clicked()
{
    MainWindow *qw = (MainWindow *)this->parentWidget();
    QString openFilePath = qw->fileDialog(false, "Open Graphics", "CEL/CL2 Files (*.cel *.CEL *.cl2 *.CL2)"); // "CEL/CL2/CLX Files (*.cel *.CEL *.cl2 *.CL2 *.clx *.CLX);;TIL Files (*.til *.TIL)");

    if (openFilePath.isEmpty())
        return;

    ui->inputFileEdit->setText(openFilePath);

    this->update();
}

void OpenAsDialog::on_openButton_clicked()
{
    if (ui->inputFileEdit->text() == "") {
        QMessageBox::warning(this, "Warning", "Input file is missing, please choose an input file.");
        return;
    }

    OpenAsParam params;
    if (ui->celSettingsGroupBox->isEnabled()) {
        // cel/cl2: clipped, width
        params.width = this->ui->celWidthEdit->text().toUShort();
        if (ui->celClippedYesRadioButton->isChecked()) {
            params.clipped = OPEN_CLIPPING_TYPE::CLIPPED_TRUE;
        } else if (ui->celClippedNoRadioButton->isChecked()) {
            params.clipped = OPEN_CLIPPING_TYPE::CLIPPED_FALSE;
        } else {
            params.clipped = OPEN_CLIPPING_TYPE::CLIPPED_AUTODETECT;
        }
    }

    // TODO:
    //  clx: ?
    //  til: ? sol + min + amp + cel path ?

    QString filePath = ui->inputFileEdit->text();
    MainWindow *qw = (MainWindow *)this->parentWidget();
    this->close();

    qw->openFile(filePath, &params);
}

void OpenAsDialog::on_openCancelButton_clicked()
{
    this->close();
}
