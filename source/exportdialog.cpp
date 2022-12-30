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

void ExportDialog::initialize(QJsonObject *cfg, D1CelBase *c, D1Min *m, D1Til *t, D1Sol *s, D1Amp *a)
{
    this->configuration = cfg;

    this->cel = c;
    this->min = m;
    this->til = t;
    this->sol = s;
    this->amp = a;

    // If there's only one frame
    if (this->cel->getFrameCount() == 1)
        ui->filesSettingWidget->setEnabled(false);
    else
        ui->filesSettingWidget->setEnabled(true);

    // If all frames have the same width/height
    if (this->cel->getFrameCount() > 1 && this->cel->isFrameSizeConstant()
        && ui->oneFileForAllFramesRadioButton->isChecked()) {
        ui->spritesSettingsWidget->setEnabled(true);
    }

    // If there's only one group
    if (this->cel->getGroupCount() == 1) {
        ui->allFramesOnOneLineRadioButton->setChecked(true);
        ui->oneFrameGroupPerLineRadioButton->setEnabled(false);
    } else {
        ui->oneFrameGroupPerLineRadioButton->setChecked(true);
        ui->oneFrameGroupPerLineRadioButton->setEnabled(true);
    }

    // If it's a CEL level file
    if (this->cel->getType() == D1CEL_TYPE::V1_LEVEL && this->min != nullptr && this->til != nullptr)
        ui->levelFramesSettingsWidget->setEnabled(true);
    else
        ui->levelFramesSettingsWidget->setEnabled(false);
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

    quint16 tileWidth = this->min->getSubtileWidth() * 2 * 32;
    quint16 tileHeight = this->min->getSubtileHeight() * 32 + 32;

    // If only one file will contain all tiles
    QImage tempOutputImage;
    quint16 tempOutputImageWidth = 0;
    quint16 tempOutputImageHeight = 0;
    if (ui->oneFileForAllFramesRadioButton->isChecked()) {
        tempOutputImageWidth = tileWidth * 8;
        tempOutputImageHeight = tileHeight * (quint32)(this->til->getTileCount() / 8);
        if (this->til->getTileCount() % 8 != 0)
            tempOutputImageHeight += tileHeight;
        tempOutputImage = QImage(tempOutputImageWidth, tempOutputImageHeight, QImage::Format_ARGB32);
        tempOutputImage.fill(Qt::transparent);
    }

    QPainter painter(&tempOutputImage);
    quint8 tileXIndex = 0;
    quint8 tileYIndex = 0;
    for (unsigned int i = 0; i < this->til->getTileCount(); i++) {
        if (progress.wasCanceled()) {
            return false;
        }
        progress.setValue(100 * i / this->til->getTileCount());

        // If only one file will contain all tiles
        if (ui->oneFileForAllFramesRadioButton->isChecked()) {
            painter.drawImage(tileXIndex * tileWidth,
                tileYIndex * tileHeight, this->til->getTileImage(i));

            tileXIndex++;
            if (tileXIndex >= 8) {
                tileXIndex = 0;
                tileYIndex++;
            }
        } else {
            QString outputFilePath = outputFilePathBase + "_tile"
                + QString("%1").arg(i, 4, 10, QChar('0')) + this->getFileFormatExtension();

            this->til->getTileImage(i).save(outputFilePath);
        }
    }

    if (ui->oneFileForAllFramesRadioButton->isChecked()) {
        painter.end();
        QString outputFilePath = outputFilePathBase + this->getFileFormatExtension();
        tempOutputImage.save(outputFilePath);
    }
    return true;
}

bool ExportDialog::exportLevelSubtiles(QProgressDialog &progress)
{
    if (this->min == nullptr || this->sol == nullptr) {
        return true;
    }

    progress.setLabelText("Exporting " + QFileInfo(this->min->getFilePath()).fileName() + " level sub-tiles...");

    QString outputFilePathBase = ui->outputFolderEdit->text() + "/"
        + QFileInfo(this->min->getFilePath()).fileName().replace(".", "_");

    quint16 subtileWidth = this->min->getSubtileWidth() * 32;
    quint16 subtileHeight = this->min->getSubtileHeight() * 32;

    // If only one file will contain all sub-tiles
    QImage tempOutputImage;
    quint16 tempOutputImageWidth = 0;
    quint16 tempOutputImageHeight = 0;
    if (ui->oneFileForAllFramesRadioButton->isChecked()) {
        tempOutputImageWidth = subtileWidth * 16;
        tempOutputImageHeight = subtileHeight * (quint32)(this->min->getSubtileCount() / 16);
        if (this->min->getSubtileCount() % 16 != 0)
            tempOutputImageHeight += subtileHeight;
        tempOutputImage = QImage(tempOutputImageWidth, tempOutputImageHeight, QImage::Format_ARGB32);
        tempOutputImage.fill(Qt::transparent);
    }

    QPainter painter(&tempOutputImage);
    quint8 subtileXIndex = 0;
    quint8 subtileYIndex = 0;
    for (unsigned int i = 0; i < this->min->getSubtileCount(); i++) {
        if (progress.wasCanceled()) {
            return false;
        }
        progress.setValue(100 * i / this->min->getSubtileCount());

        // If only one file will contain all sub-tiles
        if (ui->oneFileForAllFramesRadioButton->isChecked()) {
            painter.drawImage(subtileXIndex * subtileWidth,
                subtileYIndex * subtileHeight, this->min->getSubtileImage(i));

            subtileXIndex++;
            if (subtileXIndex >= 16) {
                subtileXIndex = 0;
                subtileYIndex++;
            }
        } else {
            QString outputFilePath = outputFilePathBase + "_sub-tile"
                + QString("%1").arg(i, 4, 10, QChar('0')) + this->getFileFormatExtension();

            this->min->getSubtileImage(i).save(outputFilePath);
        }
    }

    if (ui->oneFileForAllFramesRadioButton->isChecked()) {
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
    progress.setLabelText("Exporting " + QFileInfo(this->cel->getFilePath()).fileName() + " frames...");

    QString outputFilePathBase = ui->outputFolderEdit->text() + "/"
        + QFileInfo(this->cel->getFilePath()).fileName().replace(".", "_");
    // single frame
    if (this->cel->getFrameCount() == 1) {
        // one file for each frame (not indexed)
        QString outputFilePath = outputFilePathBase + this->getFileFormatExtension();
        this->cel->getFrameImage(0).save(outputFilePath);
        return true;
    }
    // multiple frames
    if (!ui->oneFileForAllFramesRadioButton->isChecked()) {
        // one file for each frame (indexed)
        for (unsigned int i = 0; i < this->cel->getFrameCount(); i++) {
            if (progress.wasCanceled()) {
                return false;
            }

            progress.setValue(100 * i / this->cel->getFrameCount());

            QString outputFilePath = outputFilePathBase + "_frame"
                + QString("%1").arg(i, 4, 10, QChar('0')) + this->getFileFormatExtension();

            this->cel->getFrameImage(i).save(outputFilePath);
        }
        return true;
    }
    // one file for all frames
    QImage tempOutputImage;
    quint16 tempOutputImageWidth = 0;
    quint16 tempOutputImageHeight = 0;
    // If only one file will contain all frames
    if (ui->oneFrameGroupPerLineRadioButton->isChecked()) {
        for (unsigned int i = 0; i < this->cel->getGroupCount(); i++) {
            quint16 groupImageWidth = 0;
            quint16 groupImageHeight = 0;
            for (unsigned int j = this->cel->getGroupFrameIndices(i).first;
                 j <= this->cel->getGroupFrameIndices(i).second; j++) {
                groupImageWidth += this->cel->getFrameWidth(j);
                groupImageHeight = std::max(this->cel->getFrameHeight(j), groupImageHeight);
            }
            tempOutputImageWidth = std::max(groupImageWidth, tempOutputImageWidth);
            tempOutputImageHeight += groupImageHeight;
        }

    } else if (ui->allFramesOnOneColumnRadioButton->isChecked()) {
        for (unsigned int i = 0; i < this->cel->getGroupCount(); i++) {
            tempOutputImageWidth = std::max(this->cel->getFrameWidth(i), tempOutputImageWidth);
            tempOutputImageHeight += this->cel->getFrameHeight(i);
        }
    } else if (ui->allFramesOnOneLineRadioButton->isChecked()) {
        for (unsigned int i = 0; i < this->cel->getGroupCount(); i++) {
            tempOutputImageWidth += this->cel->getFrameWidth(i);
            tempOutputImageHeight = std::max(this->cel->getFrameHeight(i), tempOutputImageHeight);
        }
    }
    tempOutputImage = QImage(tempOutputImageWidth, tempOutputImageHeight, QImage::Format_ARGB32);
    tempOutputImage.fill(Qt::transparent);

    QPainter painter(&tempOutputImage);

    if (ui->oneFrameGroupPerLineRadioButton->isChecked()) {
        quint32 cursorY = 0;
        for (unsigned int i = 0; i < this->cel->getGroupCount(); i++) {
            quint32 cursorX = 0;
            quint16 groupImageHeight = 0;
            for (unsigned int j = this->cel->getGroupFrameIndices(i).first;
                 j <= this->cel->getGroupFrameIndices(i).second; j++) {
                if (progress.wasCanceled()) {
                    return false;
                }
                progress.setValue(100 * j / this->cel->getFrameCount());

                painter.drawImage(cursorX, cursorY, this->cel->getFrameImage(j));
                cursorX += this->cel->getFrameWidth(j);
                groupImageHeight = std::max(this->cel->getFrameHeight(j), groupImageHeight);
            }
            cursorY += groupImageHeight;
        }
    } else {
        quint32 cursor = 0;
        for (unsigned int i = 0; i < this->cel->getFrameCount(); i++) {
            if (progress.wasCanceled()) {
                return false;
            }
            progress.setValue(100 * i / this->cel->getFrameCount());

            if (ui->allFramesOnOneColumnRadioButton->isChecked()) {
                painter.drawImage(0, cursor, this->cel->getFrameImage(i));
                cursor += this->cel->getFrameHeight(i);
            } else {
                painter.drawImage(cursor, 0, this->cel->getFrameImage(i));
                cursor += this->cel->getFrameWidth(i);
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

    if (this->cel == nullptr) {
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

        if (this->cel->getType() == D1CEL_TYPE::V1_LEVEL && !ui->exportLevelFrames->isChecked()) {
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
