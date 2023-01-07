#include "saveasdialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <optional>

#include "ui_saveasdialog.h"

#include "d1amp.h"
#include "d1gfx.h"
#include "d1min.h"
#include "d1sol.h"
#include "d1til.h"
#include "mainwindow.h"

std::optional<QFile *> SaveAsParam::getValidSaveOutput(QString filePath, QString selectedPath)
{
    if (!selectedPath.isEmpty()) {
        filePath = selectedPath;
        if (QFile::exists(filePath)) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(nullptr, "Confirmation", "Are you sure you want to overwrite " + filePath + "?", QMessageBox::Yes | QMessageBox::No);
            if (reply != QMessageBox::Yes) {
                return {};
            }
        }
    }

    QFile *outFile = new QFile(filePath);
    if (!outFile->open(QIODevice::WriteOnly | QFile::Truncate)) {
        QMessageBox::critical(nullptr, "Error", "Failed open file: " + filePath);
        return {};
    }

    return outFile;
}

SaveAsDialog::SaveAsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SaveAsDialog)
{
    ui->setupUi(this);
}

SaveAsDialog::~SaveAsDialog()
{
    delete ui;
}

void SaveAsDialog::initialize(QJsonObject *cfg, D1Gfx *g, D1Min *min, D1Til *til, D1Sol *sol, D1Amp *amp)
{
    // initialize the configuration pointer
    this->configuration = cfg;
    this->gfx = g;
    this->isTileset = this->gfx->getType() == D1CEL_TYPE::V1_LEVEL;

    // reset fields
    this->ui->outputCelFileEdit->setText(this->gfx->getFilePath());

    this->ui->celClippedAutoRadioButton->setChecked(true);
    this->ui->celGroupEdit->setText("0");

    this->ui->outputMinFileEdit->setText(min == nullptr ? "" : min->getFilePath());
    this->ui->outputTilFileEdit->setText(til == nullptr ? "" : til->getFilePath());
    this->ui->outputSolFileEdit->setText(sol == nullptr ? "" : sol->getFilePath());
    this->ui->outputAmpFileEdit->setText(amp == nullptr ? "" : amp->getFilePath());

    this->update();
}

void SaveAsDialog::update()
{
    if (this->isTileset) {
        this->ui->celSettingsGroupBox->setEnabled(false);
        this->ui->tilSettingsGroupBox->setEnabled(true);
    } else {
        this->ui->celSettingsGroupBox->setEnabled(true);
        this->ui->tilSettingsGroupBox->setEnabled(false);
    }
}

void SaveAsDialog::on_outputCelFileBrowseButton_clicked()
{
    QString filePath = this->gfx->getFilePath();
    const char *filter;
    if (this->isTileset) {
        filter = "CEL Files (*.cel *.CEL)";
    } else {
        filter = "CEL/CL2/CLX Files (*.cel *.CEL *.cl2 *.CL2 *.clx *.CLX)";
    }

    MainWindow *qw = (MainWindow *)this->parentWidget();
    QString saveFilePath = qw->fileDialog(true, "Save Graphics as...", filter);

    if (saveFilePath.isEmpty()) {
        return;
    }

    this->ui->outputCelFileEdit->setText(saveFilePath);

    if (this->isTileset) {
        int extPos = saveFilePath.lastIndexOf('.', saveFilePath.length() - 1);
        if (extPos >= 0 && extPos < saveFilePath.length() - 1) {
            bool upperCase = saveFilePath.at(extPos + 1).isUpper();
            saveFilePath.chop(saveFilePath.length() - 1 - extPos);
            this->ui->outputMinFileEdit->setText(saveFilePath + (upperCase ? "MIN" : "min"));
            this->ui->outputTilFileEdit->setText(saveFilePath + (upperCase ? "TIL" : "til"));
            this->ui->outputSolFileEdit->setText(saveFilePath + (upperCase ? "SOL" : "sol"));
            this->ui->outputAmpFileEdit->setText(saveFilePath + (upperCase ? "AMP" : "amp"));
        }
    }
    this->update();
}

void SaveAsDialog::on_outputMinFileBrowseButton_clicked()
{
    MainWindow *qw = (MainWindow *)this->parentWidget();
    QString saveFilePath = qw->fileDialog(true, "Save MIN as...", "MIN Files (*.min *.MIN)");

    if (saveFilePath.isEmpty())
        return;

    this->ui->outputMinFileEdit->setText(saveFilePath);

    this->update();
}

void SaveAsDialog::on_outputTilFileBrowseButton_clicked()
{
    MainWindow *qw = (MainWindow *)this->parentWidget();
    QString saveFilePath = qw->fileDialog(true, "Save TIL as...", "TIL Files (*.til *.TIL)");

    if (saveFilePath.isEmpty())
        return;

    this->ui->outputTilFileEdit->setText(saveFilePath);

    this->update();
}

void SaveAsDialog::on_outputSolFileBrowseButton_clicked()
{
    MainWindow *qw = (MainWindow *)this->parentWidget();
    QString saveFilePath = qw->fileDialog(true, "Save SOL as...", "SOL Files (*.sol *.SOL)");

    if (saveFilePath.isEmpty())
        return;

    this->ui->outputSolFileEdit->setText(saveFilePath);

    this->update();
}

void SaveAsDialog::on_outputAmpFileBrowseButton_clicked()
{
    MainWindow *qw = (MainWindow *)this->parentWidget();
    QString saveFilePath = qw->fileDialog(true, "Save AMP as...", "AMP Files (*.amp *.AMP)");

    if (saveFilePath.isEmpty())
        return;

    this->ui->outputAmpFileEdit->setText(saveFilePath);

    this->update();
}

void SaveAsDialog::on_saveButton_clicked()
{
    SaveAsParam params;
    // main cel file
    params.celFilePath = this->ui->outputCelFileEdit->text();
    if (params.celFilePath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Output file is missing, please choose an output file.");
        return;
    }
    // celSettingsGroupBox: groupNum, clipped
    params.groupNum = this->ui->celGroupEdit->text().toUShort();
    if (this->ui->celClippedYesRadioButton->isChecked()) {
        params.clipped = SAVE_CLIPPING_TYPE::CLIPPED_TRUE;
    } else if (this->ui->celClippedNoRadioButton->isChecked()) {
        params.clipped = SAVE_CLIPPING_TYPE::CLIPPED_FALSE;
    } else {
        params.clipped = SAVE_CLIPPING_TYPE::CLIPPED_AUTODETECT;
    }
    // tilSettingsGroupBox: min, til, sol and amp files
    params.minFilePath = this->ui->outputMinFileEdit->text();
    params.tilFilePath = this->ui->outputTilFileEdit->text();
    params.solFilePath = this->ui->outputSolFileEdit->text();
    params.ampFilePath = this->ui->outputAmpFileEdit->text();

    MainWindow *qw = (MainWindow *)this->parentWidget();
    this->close();

    qw->saveFile(params);
}

void SaveAsDialog::on_saveCancelButton_clicked()
{
    this->close();
}
