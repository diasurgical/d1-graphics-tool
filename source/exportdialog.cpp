#include "exportdialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <algorithm>

#include "ui_exportdialog.h"

ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExportDialog)
{
    ui->setupUi(this);
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::initialize(QJsonObject *cfg, D1Gfx *g, D1Min *m, D1Til *t, D1Sol *s, D1Amp *a)
{
    this->configuration = cfg;

    this->gfx = g;
    this->min = m;
    this->til = t;
    this->sol = s;
    this->amp = a;

    bool multiFrame = this->gfx->getFrameCount() > 1;
    // disable if there's only one frame
    ui->filesSettingWidget->setEnabled(multiFrame);

    // disable if there is only one frame or not all frames have the same width/height
    ui->spritesSettingsWidget->setEnabled(multiFrame && this->gfx->isFrameSizeConstant() && ui->oneFileForAllFramesRadioButton->isChecked());

    // If there's only one group
    if (this->gfx->getGroupCount() == 1) {
        ui->allFramesOnOneLineRadioButton->setChecked(true);
        ui->oneFrameGroupPerLineRadioButton->setEnabled(false);
    } else {
        ui->oneFrameGroupPerLineRadioButton->setChecked(true);
        ui->oneFrameGroupPerLineRadioButton->setEnabled(true);
    }

    // disable if not a CEL level file or data is missing
    ui->levelFramesSettingsWidget->setEnabled(this->gfx->getType() == D1CEL_TYPE::V1_LEVEL && this->min != nullptr && this->til != nullptr);
}

QString ExportDialog::getFileFormatExtension()
{
    if (ui->pngRadioButton->isChecked())
        return ".png";
    if (ui->bmpRadioButton->isChecked())
        return ".bmp";

    return QString();
}

void ExportDialog::on_outputFolderBrowseButton_clicked()
{
    QString selectedDirectory = QFileDialog::getExistingDirectory(
        this, "Select Output Folder", QString(), QFileDialog::ShowDirsOnly);

    if (selectedDirectory.isEmpty())
        return;

    ui->outputFolderEdit->setText(selectedDirectory);
}

bool ExportDialog::exportLevelTiles(QProgressDialog &progress)
{
    if (this->min == nullptr || this->til == nullptr) {
        return true;
    }

    progress.setLabelText("Exporting " + QFileInfo(this->til->getFilePath()).fileName() + " level tiles...");

    QString outputFilePathBase = ui->outputFolderEdit->text() + "/"
        + QFileInfo(this->til->getFilePath()).fileName().replace(".", "_");

    unsigned tileWidth = this->min->getSubtileWidth() * 2 * MICRO_WIDTH;
    unsigned tileHeight = this->min->getSubtileHeight() * MICRO_HEIGHT + 32;
    unsigned n = this->til->getTileCount();

    // If only one file will contain all tiles
    constexpr unsigned tiles_per_line = 8;
    QImage tempOutputImage;
    unsigned tempOutputImageWidth = 0;
    unsigned tempOutputImageHeight = 0;
    bool oneFileForAll = ui->oneFileForAllFramesRadioButton->isChecked();
    if (oneFileForAll) {
        tempOutputImageWidth = tileWidth * tiles_per_line;
        tempOutputImageHeight = tileHeight * ((n + (tiles_per_line - 1)) / tiles_per_line);
        tempOutputImage = QImage(tempOutputImageWidth, tempOutputImageHeight, QImage::Format_ARGB32);
        tempOutputImage.fill(Qt::transparent);
    }

    QPainter painter(&tempOutputImage);
    unsigned dx = 0, dy = 0;
    for (unsigned i = 0; i < n; i++) {
        if (progress.wasCanceled()) {
            return false;
        }
        progress.setValue(100 * i / n);

        // If only one file will contain all tiles
        if (oneFileForAll) {
            painter.drawImage(dx, dy, this->til->getTileImage(i));

            dx += tileWidth;
            if (dx >= tempOutputImageWidth) {
                dx = 0;
                dy += tileHeight;
            }
        } else {
            QString outputFilePath = outputFilePathBase + "_tile"
                + QString("%1").arg(i, 4, 10, QChar('0')) + this->getFileFormatExtension();

            this->til->getTileImage(i).save(outputFilePath);
        }
    }

    if (oneFileForAll) {
        painter.end();
        QString outputFilePath = outputFilePathBase + this->getFileFormatExtension();
        tempOutputImage.save(outputFilePath);
    }
    return true;
}

bool ExportDialog::exportLevelSubtiles(QProgressDialog &progress)
{
    if (this->min == nullptr) {
        return true;
    }

    progress.setLabelText("Exporting " + QFileInfo(this->min->getFilePath()).fileName() + " level sub-tiles...");

    QString outputFilePathBase = ui->outputFolderEdit->text() + "/"
        + QFileInfo(this->min->getFilePath()).fileName().replace(".", "_");

    unsigned subtileWidth = this->min->getSubtileWidth() * MICRO_WIDTH;
    unsigned subtileHeight = this->min->getSubtileHeight() * MICRO_HEIGHT;
    unsigned n = this->min->getSubtileCount();

    // If only one file will contain all sub-tiles
    constexpr unsigned subtiles_per_line = 16;
    QImage tempOutputImage;
    unsigned tempOutputImageWidth = 0;
    unsigned tempOutputImageHeight = 0;
    bool oneFileForAll = ui->oneFileForAllFramesRadioButton->isChecked();
    if (oneFileForAll) {
        tempOutputImageWidth = subtileWidth * subtiles_per_line;
        tempOutputImageHeight = subtileHeight * ((n + (subtiles_per_line - 1)) / subtiles_per_line);
        tempOutputImage = QImage(tempOutputImageWidth, tempOutputImageHeight, QImage::Format_ARGB32);
        tempOutputImage.fill(Qt::transparent);
    }

    QPainter painter(&tempOutputImage);
    unsigned dx = 0, dy = 0;
    for (unsigned i = 0; i < n; i++) {
        if (progress.wasCanceled()) {
            return false;
        }
        progress.setValue(100 * i / n);

        // If only one file will contain all sub-tiles
        if (oneFileForAll) {
            painter.drawImage(dx, dy, this->min->getSubtileImage(i));

            dx += subtileWidth;
            if (dx >= tempOutputImageWidth) {
                dx = 0;
                dy += subtileHeight;
            }
        } else {
            QString outputFilePath = outputFilePathBase + "_sub-tile"
                + QString("%1").arg(i, 4, 10, QChar('0')) + this->getFileFormatExtension();

            this->min->getSubtileImage(i).save(outputFilePath);
        }
    }

    if (oneFileForAll) {
        painter.end();
        QString outputFilePath = outputFilePathBase + this->getFileFormatExtension();
        tempOutputImage.save(outputFilePath);
    }
    return true;
}

bool ExportDialog::exportLevel(QProgressDialog &progress)
{
    if (ui->exportLevelTiles->isChecked()) {
        return this->exportLevelTiles(progress);
    } else if (ui->exportLevelSubtiles->isChecked()) {
        return this->exportLevelSubtiles(progress);
    }
    return true;
}

bool ExportDialog::exportSprites(QProgressDialog &progress)
{
    progress.setLabelText("Exporting " + QFileInfo(this->gfx->getFilePath()).fileName() + " frames...");

    QString outputFilePathBase = ui->outputFolderEdit->text() + "/"
        + QFileInfo(this->gfx->getFilePath()).fileName().replace(".", "_");
    // single frame
    if (this->gfx->getFrameCount() == 1) {
        // one file for each frame (not indexed)
        QString outputFilePath = outputFilePathBase + this->getFileFormatExtension();
        this->gfx->getFrameImage(0).save(outputFilePath);
        return true;
    }
    // multiple frames
    if (!ui->oneFileForAllFramesRadioButton->isChecked()) {
        // one file for each frame (indexed)
        for (int i = 0; i < this->gfx->getFrameCount(); i++) {
            if (progress.wasCanceled()) {
                return false;
            }

            progress.setValue(100 * i / this->gfx->getFrameCount());

            QString outputFilePath = outputFilePathBase + "_frame"
                + QString("%1").arg(i, 4, 10, QChar('0')) + this->getFileFormatExtension();

            this->gfx->getFrameImage(i).save(outputFilePath);
        }
        return true;
    }
    // one file for all frames
    QImage tempOutputImage;
    quint16 tempOutputImageWidth = 0;
    quint16 tempOutputImageHeight = 0;
    // If only one file will contain all frames
    if (ui->oneFrameGroupPerLineRadioButton->isChecked()) {
        for (int i = 0; i < this->gfx->getGroupCount(); i++) {
            quint16 groupImageWidth = 0;
            quint16 groupImageHeight = 0;
            for (unsigned int j = this->gfx->getGroupFrameIndices(i).first;
                 j <= this->gfx->getGroupFrameIndices(i).second; j++) {
                groupImageWidth += this->gfx->getFrameWidth(j);
                groupImageHeight = std::max(this->gfx->getFrameHeight(j), groupImageHeight);
            }
            tempOutputImageWidth = std::max(groupImageWidth, tempOutputImageWidth);
            tempOutputImageHeight += groupImageHeight;
        }

    } else if (ui->allFramesOnOneColumnRadioButton->isChecked()) {
        for (int i = 0; i < this->gfx->getGroupCount(); i++) {
            tempOutputImageWidth = std::max(this->gfx->getFrameWidth(i), tempOutputImageWidth);
            tempOutputImageHeight += this->gfx->getFrameHeight(i);
        }
    } else if (ui->allFramesOnOneLineRadioButton->isChecked()) {
        for (int i = 0; i < this->gfx->getGroupCount(); i++) {
            tempOutputImageWidth += this->gfx->getFrameWidth(i);
            tempOutputImageHeight = std::max(this->gfx->getFrameHeight(i), tempOutputImageHeight);
        }
    }
    tempOutputImage = QImage(tempOutputImageWidth, tempOutputImageHeight, QImage::Format_ARGB32);
    tempOutputImage.fill(Qt::transparent);

    QPainter painter(&tempOutputImage);

    if (ui->oneFrameGroupPerLineRadioButton->isChecked()) {
        quint32 cursorY = 0;
        for (int i = 0; i < this->gfx->getGroupCount(); i++) {
            quint32 cursorX = 0;
            quint16 groupImageHeight = 0;
            for (unsigned int j = this->gfx->getGroupFrameIndices(i).first;
                 j <= this->gfx->getGroupFrameIndices(i).second; j++) {
                if (progress.wasCanceled()) {
                    return false;
                }
                progress.setValue(100 * j / this->gfx->getFrameCount());

                painter.drawImage(cursorX, cursorY, this->gfx->getFrameImage(j));
                cursorX += this->gfx->getFrameWidth(j);
                groupImageHeight = std::max(this->gfx->getFrameHeight(j), groupImageHeight);
            }
            cursorY += groupImageHeight;
        }
    } else {
        quint32 cursor = 0;
        for (int i = 0; i < this->gfx->getFrameCount(); i++) {
            if (progress.wasCanceled()) {
                return false;
            }
            progress.setValue(100 * i / this->gfx->getFrameCount());

            if (ui->allFramesOnOneColumnRadioButton->isChecked()) {
                painter.drawImage(0, cursor, this->gfx->getFrameImage(i));
                cursor += this->gfx->getFrameHeight(i);
            } else {
                painter.drawImage(cursor, 0, this->gfx->getFrameImage(i));
                cursor += this->gfx->getFrameWidth(i);
            }
        }
    }

    painter.end();

    QString outputFilePath = outputFilePathBase + this->getFileFormatExtension();
    tempOutputImage.save(outputFilePath);
    return true;
}

void ExportDialog::on_exportButton_clicked()
{
    if (ui->outputFolderEdit->text() == "") {
        QMessageBox::warning(this, "Warning", "Output folder is missing, please choose an output folder.");
        return;
    }

    if (this->gfx == nullptr) {
        QMessageBox::critical(this, "Warning", "No graphics loaded.");
        return;
    }

    bool result;
    try {
        // Displaying the progress dialog box
        QProgressDialog progress("Exporting...", "Cancel", 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setMinimumDuration(0);
        progress.setWindowTitle("Export");
        progress.setLabelText("Exporting");
        progress.setValue(0);
        progress.show();

        if (this->gfx->getType() == D1CEL_TYPE::V1_LEVEL && !ui->exportLevelFrames->isChecked()) {
            result = this->exportLevel(progress);
        } else {
            result = this->exportSprites(progress);
        }
    } catch (...) {
        QMessageBox::critical(this, "Error", "Export Failed.");
        return;
    }
    if (result) {
        QMessageBox::information(this, "Information", "Export successful.");
        this->close();
    } else {
        QMessageBox::warning(this, "Export Canceled", "Export was canceled.");
    }
}

void ExportDialog::on_exportCancelButton_clicked()
{
    this->close();
}

void ExportDialog::on_oneFileForAllFramesRadioButton_toggled(bool checked)
{
    if (checked && !ui->levelFramesSettingsWidget->isEnabled())
        ui->spritesSettingsWidget->setEnabled(true);
    else
        ui->spritesSettingsWidget->setEnabled(false);
}
