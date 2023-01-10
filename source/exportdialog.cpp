#include "exportdialog.h"

#include <QFileDialog>
#include <QImageWriter>
#include <QMessageBox>
#include <QPainter>
#include <algorithm>

#include "ui_exportdialog.h"

ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExportDialog())
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

    // initialize the format combobox
    QList<QByteArray> formats = QImageWriter::supportedImageFormats();
    QStringList formatTxts;
    for (QByteArray &format : formats) {
        QString fmt = format;
        formatTxts.append(fmt.toUpper());
    }
    // - remember the last selected format
    QComboBox *fmtBox = this->ui->formatComboBox;
    QString lastFmt = fmtBox->currentText();
    if (lastFmt.isEmpty()) {
        lastFmt = "PNG";
    }
    fmtBox->clear();
    fmtBox->addItems(formatTxts);
    fmtBox->setCurrentIndex(fmtBox->findText(lastFmt));

    // initialize files count
    /*this->ui->filesCountComboBox->setEnabled(multiFrame);
    if (!multiFrame) {
        this->ui->filesCountComboBox->setCurrentIndex(0);
    }*/

    // initialize content type
    bool isTileset = this->gfx->getType() == D1CEL_TYPE::V1_LEVEL;
    this->ui->contentTypeComboBox->setEnabled(isTileset);
    if (!isTileset) {
        this->ui->contentTypeComboBox->setCurrentIndex(0);
    }

    // initialize content placement
    /*this->ui->contentPlacementComboBox->setEnabled(multiFrame);
    if (!multiFrame) {
        this->ui->contentPlacementComboBox->setCurrentIndex(0);
    }*/
}

QString ExportDialog::getFileFormatExtension()
{
    return "." + this->ui->formatComboBox->currentText().toLower();
}

void ExportDialog::on_outputFolderBrowseButton_clicked()
{
    QString selectedDirectory = QFileDialog::getExistingDirectory(
        this, "Select Output Folder", QString(), QFileDialog::ShowDirsOnly);

    if (selectedDirectory.isEmpty())
        return;

    ui->outputFolderEdit->setText(selectedDirectory);
}

bool ExportDialog::exportLevelTiles25D(QProgressDialog &progress)
{
    progress.setLabelText("Exporting " + QFileInfo(this->til->getFilePath()).fileName() + " 2.5d level tiles...");

    QString outputFilePathBase = ui->outputFolderEdit->text() + "/"
        + QFileInfo(this->til->getFilePath()).fileName().replace(".", "_25d_");

    int count = this->til->getTileCount();
    int tileFrom = this->ui->contentRangeFromEdit->text().toUInt();
    if (tileFrom != 0) {
        tileFrom--;
    }
    int tileTo = this->ui->contentRangeToEdit->text().toUInt();
    if (tileTo == 0 || tileTo > count) {
        tileTo = count;
    }
    tileTo--;
    int amount = tileTo - tileFrom + 1;
    // nothing to export
    if (amount == 0) {
        return true;
    }
    // single tile
    if (amount == 1 && tileFrom == 0) {
        // one file for the only tile (not indexed)
        QString outputFilePath = outputFilePathBase + this->getFileFormatExtension();
        this->til->getTileImage(0).save(outputFilePath);
        return true;
    }

    // multiple tiles
    if (amount == 1 || this->ui->filesCountComboBox->currentIndex() != 0) {
        // one file for each tile (indexed)
        for (int i = tileFrom; i <= tileTo; i++) {
            if (progress.wasCanceled()) {
                return false;
            }

            progress.setValue(100 * (i - tileFrom) / amount);

            QString outputFilePath = outputFilePathBase
                + QString("%1").arg(i, 4, 10, QChar('0')) + this->getFileFormatExtension();

            this->til->getTileImage(i).save(outputFilePath);
        }
        return true;
    }
    // one file for all tiles
    if (tileFrom != 0 || tileTo < count - 1) {
        outputFilePathBase += QString::number(tileFrom + 1) + "_" + QString::number(tileTo + 1);
    }
    QString outputFilePath = outputFilePathBase + this->getFileFormatExtension();

    unsigned tileWidth = this->min->getSubtileWidth() * 2 * MICRO_WIDTH;
    unsigned tileHeight = this->min->getSubtileHeight() * MICRO_HEIGHT + 32;

    constexpr unsigned TILES_PER_LINE = 8;
    QImage tempOutputImage;
    unsigned tempOutputImageWidth = 0;
    unsigned tempOutputImageHeight = 0;
    int placement = this->ui->contentPlacementComboBox->currentIndex();
    if (placement == 0) { // grouped
        tempOutputImageWidth = tileWidth * TILES_PER_LINE;
        tempOutputImageHeight = tileHeight * ((amount + (TILES_PER_LINE - 1)) / TILES_PER_LINE);
    } else if (placement == 2) { // tiles on one column
        tempOutputImageWidth = tileWidth;
        tempOutputImageHeight = tileHeight * amount;
    } else { // placement == 1 -- tiles on one line
        tempOutputImageWidth = tileWidth * amount;
        tempOutputImageHeight = tileHeight;
    }

    tempOutputImage = QImage(tempOutputImageWidth, tempOutputImageHeight, QImage::Format_ARGB32);
    tempOutputImage.fill(Qt::transparent);

    QPainter painter(&tempOutputImage);

    if (placement == 0) { // grouped
        unsigned dx = 0, dy = 0;
        for (int i = tileFrom; i <= tileTo; i++) {
            if (progress.wasCanceled()) {
                return false;
            }
            progress.setValue(100 * (i - tileFrom) / amount);

            const QImage image = this->til->getTileImage(i);

            painter.drawImage(dx, dy, image);

            dx += image.width();
            if (dx >= tempOutputImageWidth) {
                dx = 0;
                dy += image.height();
            }
        }
    } else {
        int cursor = 0;
        for (int i = tileFrom; i <= tileTo; i++) {
            if (progress.wasCanceled()) {
                return false;
            }
            progress.setValue(100 * (i - tileFrom) / amount);

            const QImage image = this->til->getTileImage(i);
            if (placement == 2) { // tiles on one column
                painter.drawImage(0, cursor, image);
                cursor += image.height();
            } else { // placement == 1 -- tiles on one line
                painter.drawImage(cursor, 0, image);
                cursor += image.width();
            }
        }
    }

    painter.end();

    tempOutputImage.save(outputFilePath);
    return true;
}

bool ExportDialog::exportLevelTiles(QProgressDialog &progress)
{
    progress.setLabelText("Exporting " + QFileInfo(this->min->getFilePath()).fileName() + " flat level tiles...");

    QString outputFilePathBase = ui->outputFolderEdit->text() + "/"
        + QFileInfo(this->til->getFilePath()).fileName().replace(".", "_flat_");

    int count = this->til->getTileCount();
    int tileFrom = this->ui->contentRangeFromEdit->text().toUInt();
    if (tileFrom != 0) {
        tileFrom--;
    }
    int tileTo = this->ui->contentRangeToEdit->text().toUInt();
    if (tileTo == 0 || tileTo > count) {
        tileTo = count;
    }
    tileTo--;
    int amount = tileTo - tileFrom + 1;
    // nothing to export
    if (amount <= 0) {
        return true;
    }
    // single tile
    if (amount == 1 && tileFrom == 0) {
        // one file for the only tile (not indexed)
        QString outputFilePath = outputFilePathBase + this->getFileFormatExtension();
        this->til->getFlatTileImage(0).save(outputFilePath);
        return true;
    }
    // multiple tiles
    if (amount == 1 || this->ui->filesCountComboBox->currentIndex() != 0) {
        // one file for each tile (indexed)
        for (int i = tileFrom; i <= tileTo; i++) {
            if (progress.wasCanceled()) {
                return false;
            }

            progress.setValue(100 * (i - tileFrom) / amount);

            QString outputFilePath = outputFilePathBase
                + QString("%1").arg(i, 4, 10, QChar('0')) + this->getFileFormatExtension();

            this->til->getFlatTileImage(i).save(outputFilePath);
        }
        return true;
    }
    // one file for all tiles
    if (tileFrom != 0 || tileTo < count - 1) {
        outputFilePathBase += QString::number(tileFrom + 1) + "_" + QString::number(tileTo + 1);
    }
    QString outputFilePath = outputFilePathBase + this->getFileFormatExtension();

    unsigned tileWidth = this->min->getSubtileWidth() * MICRO_WIDTH * TILE_WIDTH * TILE_HEIGHT;
    unsigned tileHeight = this->min->getSubtileHeight() * MICRO_HEIGHT;

    // If only one file will contain all tiles
    constexpr unsigned TILES_PER_LINE = 4;
    QImage tempOutputImage;
    unsigned tempOutputImageWidth = 0;
    unsigned tempOutputImageHeight = 0;
    int placement = this->ui->contentPlacementComboBox->currentIndex();
    if (placement == 0) { // grouped
        tempOutputImageWidth = tileWidth * TILES_PER_LINE;
        tempOutputImageHeight = tileHeight * ((amount + (TILES_PER_LINE - 1)) / TILES_PER_LINE);
    } else if (placement == 2) { // tiles on one column
        tempOutputImageWidth = tileWidth;
        tempOutputImageHeight = tileHeight * amount;
    } else { // placement == 1 -- tiles on one line
        tempOutputImageWidth = tileWidth * amount;
        tempOutputImageHeight = tileHeight;
    }

    tempOutputImage = QImage(tempOutputImageWidth, tempOutputImageHeight, QImage::Format_ARGB32);
    tempOutputImage.fill(Qt::transparent);

    QPainter painter(&tempOutputImage);

    if (placement == 0) { // grouped
        unsigned dx = 0, dy = 0;
        for (int i = tileFrom; i <= tileTo; i++) {
            if (progress.wasCanceled()) {
                return false;
            }
            progress.setValue(100 * (i - tileFrom) / amount);

            const QImage image = this->til->getFlatTileImage(i);

            painter.drawImage(dx, dy, image);

            dx += image.width();
            if (dx >= tempOutputImageWidth) {
                dx = 0;
                dy += image.height();
            }
        }
    } else {
        int cursor = 0;
        for (int i = tileFrom; i <= tileTo; i++) {
            if (progress.wasCanceled()) {
                return false;
            }
            progress.setValue(100 * (i - tileFrom) / amount);

            const QImage image = this->til->getFlatTileImage(i);
            if (placement == 2) { // tiles on one column
                painter.drawImage(0, cursor, image);
                cursor += image.height();
            } else { // placement == 1 -- tiles on one line
                painter.drawImage(cursor, 0, image);
                cursor += image.width();
            }
        }
    }

    painter.end();

    tempOutputImage.save(outputFilePath);
    return true;
}

bool ExportDialog::exportLevelSubtiles(QProgressDialog &progress)
{
    progress.setLabelText("Exporting " + QFileInfo(this->min->getFilePath()).fileName() + " level subtiles...");

    QString outputFilePathBase = ui->outputFolderEdit->text() + "/"
        + QFileInfo(this->min->getFilePath()).fileName().replace(".", "_");

    int count = this->min->getSubtileCount();
    int subtileFrom = this->ui->contentRangeFromEdit->text().toUInt();
    if (subtileFrom != 0) {
        subtileFrom--;
    }
    int subtileTo = this->ui->contentRangeToEdit->text().toUInt();
    if (subtileTo == 0 || subtileTo > count) {
        subtileTo = count;
    }
    subtileTo--;
    int amount = subtileTo - subtileFrom + 1;
    // nothing to export
    if (amount <= 0) {
        return true;
    }
    // single subtile
    if (amount == 1 && subtileFrom == 0) {
        // one file for the only subtile (not indexed)
        QString outputFilePath = outputFilePathBase + this->getFileFormatExtension();
        this->min->getSubtileImage(0).save(outputFilePath);
        return true;
    }
    // multiple subtiles
    if (amount == 1 || this->ui->filesCountComboBox->currentIndex() != 0) {
        // one file for each subtile (indexed)
        for (int i = subtileFrom; i <= subtileTo; i++) {
            if (progress.wasCanceled()) {
                return false;
            }

            progress.setValue(100 * (i - subtileFrom) / amount);

            QString outputFilePath = outputFilePathBase + "_subtile"
                + QString("%1").arg(i, 4, 10, QChar('0')) + this->getFileFormatExtension();

            this->min->getSubtileImage(i).save(outputFilePath);
        }
        return true;
    }
    // one file for all subtiles
    if (subtileFrom != 0 || subtileTo < count - 1) {
        outputFilePathBase += QString::number(subtileFrom + 1) + "_" + QString::number(subtileTo + 1);
    }
    QString outputFilePath = outputFilePathBase + this->getFileFormatExtension();

    unsigned subtileWidth = this->min->getSubtileWidth() * MICRO_WIDTH;
    unsigned subtileHeight = this->min->getSubtileHeight() * MICRO_HEIGHT;

    QImage tempOutputImage;
    unsigned tempOutputImageWidth = 0;
    unsigned tempOutputImageHeight = 0;
    int placement = this->ui->contentPlacementComboBox->currentIndex();
    if (placement == 0) { // grouped
        tempOutputImageWidth = subtileWidth * EXPORT_SUBTILES_PER_LINE;
        tempOutputImageHeight = subtileHeight * ((amount + (EXPORT_SUBTILES_PER_LINE - 1)) / EXPORT_SUBTILES_PER_LINE);
    } else if (placement == 2) { // subtiles on one column
        tempOutputImageWidth = subtileWidth;
        tempOutputImageHeight = subtileHeight * amount;
    } else { // placement == 1 -- subtiles on one line
        tempOutputImageWidth = subtileWidth * amount;
        tempOutputImageHeight = subtileHeight;
        if ((amount % (TILE_WIDTH * TILE_HEIGHT)) == 0) {
            tempOutputImageWidth += subtileWidth; // add an extra subtile to ensure it is not recognized as a flat tile
        }
    }

    tempOutputImage = QImage(tempOutputImageWidth, tempOutputImageHeight, QImage::Format_ARGB32);
    tempOutputImage.fill(Qt::transparent);

    QPainter painter(&tempOutputImage);

    if (placement == 0) { // grouped
        unsigned dx = 0, dy = 0;
        for (int i = subtileFrom; i <= subtileTo; i++) {
            if (progress.wasCanceled()) {
                return false;
            }
            progress.setValue(100 * (i - subtileFrom) / amount);

            const QImage image = this->min->getSubtileImage(i);

            painter.drawImage(dx, dy, image);

            dx += image.width();
            if (dx >= tempOutputImageWidth) {
                dx = 0;
                dy += image.height();
            }
        }
    } else {
        int cursor = 0;
        for (int i = subtileFrom; i <= subtileTo; i++) {
            if (progress.wasCanceled()) {
                return false;
            }
            progress.setValue(100 * (i - subtileFrom) / amount);

            const QImage image = this->min->getSubtileImage(i);
            if (placement == 2) { // subtiles on one column
                painter.drawImage(0, cursor, image);
                cursor += image.height();
            } else { // placement == 1 -- subtiles on one line
                painter.drawImage(cursor, 0, image);
                cursor += image.width();
            }
        }
    }

    painter.end();

    tempOutputImage.save(outputFilePath);
    return true;
}

bool ExportDialog::exportFrames(QProgressDialog &progress)
{
    progress.setLabelText("Exporting " + QFileInfo(this->gfx->getFilePath()).fileName() + " frames...");

    QString outputFilePathBase = ui->outputFolderEdit->text() + "/"
        + QFileInfo(this->gfx->getFilePath()).fileName().replace(".", "_");

    int count = this->gfx->getFrameCount();
    int frameFrom = this->ui->contentRangeFromEdit->text().toUInt();
    if (frameFrom != 0) {
        frameFrom--;
    }
    int frameTo = this->ui->contentRangeToEdit->text().toUInt();
    if (frameTo == 0 || frameTo > count) {
        frameTo = count;
    }
    frameTo--;
    int amount = frameTo - frameFrom + 1;
    // nothing to export
    if (amount <= 0) {
        return true;
    }
    // single frame
    if (amount == 1 && frameFrom == 0) {
        // one file for the only frame (not indexed)
        QString outputFilePath = outputFilePathBase + this->getFileFormatExtension();
        this->gfx->getFrameImage(0).save(outputFilePath);
        return true;
    }
    // multiple frames
    if (amount == 1 || this->ui->filesCountComboBox->currentIndex() != 0) {
        // one file for each frame (indexed)
        for (int i = frameFrom; i <= frameTo; i++) {
            if (progress.wasCanceled()) {
                return false;
            }

            progress.setValue(100 * (i - frameFrom) / amount);

            QString outputFilePath = outputFilePathBase + "_frame"
                + QString("%1").arg(i, 4, 10, QChar('0')) + this->getFileFormatExtension();

            this->gfx->getFrameImage(i).save(outputFilePath);
        }
        return true;
    }
    // one file for all frames
    if (frameFrom != 0 || frameTo < count - 1) {
        outputFilePathBase += QString::number(frameFrom + 1) + "_" + QString::number(frameTo + 1);
    }
    QString outputFilePath = outputFilePathBase + this->getFileFormatExtension();

    QImage tempOutputImage;
    int tempOutputImageWidth = 0;
    int tempOutputImageHeight = 0;

    int placement = this->ui->contentPlacementComboBox->currentIndex();
    if (placement == 0) { // grouped
        if (this->gfx->getType() == D1CEL_TYPE::V1_LEVEL) {
            // artifical grouping of a tileset
            int groupImageWidth = 0;
            int groupImageHeight = 0;
            for (int i = frameFrom; i <= frameTo; i++) {
                if (((i - frameFrom) % EXPORT_LVLFRAMES_PER_LINE) == 0) {
                    tempOutputImageWidth = std::max(groupImageWidth, tempOutputImageWidth);
                    tempOutputImageHeight += groupImageHeight;
                    groupImageWidth = 0;
                    groupImageHeight = 0;
                }
                groupImageWidth += this->gfx->getFrameWidth(i);
                groupImageHeight = std::max(this->gfx->getFrameHeight(i), groupImageHeight);
            }
            tempOutputImageWidth = std::max(groupImageWidth, tempOutputImageWidth);
            tempOutputImageHeight += groupImageHeight;
        } else {
            for (int i = 0; i < this->gfx->getGroupCount(); i++) {
                int groupImageWidth = 0;
                int groupImageHeight = 0;
                for (unsigned int j = this->gfx->getGroupFrameIndices(i).first;
                     j <= this->gfx->getGroupFrameIndices(i).second; j++) {
                    if (j < (unsigned)frameFrom || j > (unsigned)frameTo) {
                        continue;
                    }
                    groupImageWidth += this->gfx->getFrameWidth(j);
                    groupImageHeight = std::max(this->gfx->getFrameHeight(j), groupImageHeight);
                }
                tempOutputImageWidth = std::max(groupImageWidth, tempOutputImageWidth);
                tempOutputImageHeight += groupImageHeight;
            }
        }
    } else if (placement == 2) { // frames on one column
        for (int i = frameFrom; i <= frameTo; i++) {
            tempOutputImageWidth = std::max(this->gfx->getFrameWidth(i), tempOutputImageWidth);
            tempOutputImageHeight += this->gfx->getFrameHeight(i);
        }
    } else { // placement == 1 -- frames on one line
        for (int i = frameFrom; i <= frameTo; i++) {
            tempOutputImageWidth += this->gfx->getFrameWidth(i);
            tempOutputImageHeight = std::max(this->gfx->getFrameHeight(i), tempOutputImageHeight);
        }
    }
    tempOutputImage = QImage(tempOutputImageWidth, tempOutputImageHeight, QImage::Format_ARGB32);
    tempOutputImage.fill(Qt::transparent);

    QPainter painter(&tempOutputImage);

    if (placement == 0) { // grouped
        if (this->gfx->getType() == D1CEL_TYPE::V1_LEVEL) {
            // artifical grouping of a tileset
            int cursorY = 0;
            int cursorX = 0;
            int groupImageHeight = 0;
            for (int i = frameFrom; i <= frameTo; i++) {
                if (progress.wasCanceled()) {
                    return false;
                }
                progress.setValue(100 * (i - frameFrom) / amount);

                if (((i - frameFrom) % EXPORT_LVLFRAMES_PER_LINE) == 0) {
                    cursorY += groupImageHeight;
                    cursorX = 0;
                    groupImageHeight = 0;
                }

                const QImage image = this->gfx->getFrameImage(i);
                painter.drawImage(cursorX, cursorY, image);

                cursorX += image.width();
                groupImageHeight = std::max(image.height(), groupImageHeight);
            }
        } else {
            int cursorY = 0;
            for (int i = 0; i < this->gfx->getGroupCount(); i++) {
                int cursorX = 0;
                int groupImageHeight = 0;
                for (unsigned int j = this->gfx->getGroupFrameIndices(i).first;
                     j <= this->gfx->getGroupFrameIndices(i).second; j++) {
                    if (j < (unsigned)frameFrom || j > (unsigned)frameTo) {
                        continue;
                    }
                    if (progress.wasCanceled()) {
                        return false;
                    }
                    progress.setValue(100 * (j - frameFrom) / amount);

                    const QImage image = this->gfx->getFrameImage(j);
                    painter.drawImage(cursorX, cursorY, image);
                    cursorX += image.width();
                    groupImageHeight = std::max(image.height(), groupImageHeight);
                }
                cursorY += groupImageHeight;
            }
        }
    } else {
        int cursor = 0;
        for (int i = frameFrom; i <= frameTo; i++) {
            if (progress.wasCanceled()) {
                return false;
            }
            progress.setValue(100 * (i - frameFrom) / amount);

            const QImage image = this->gfx->getFrameImage(i);
            if (placement == 2) { // frames on one column
                painter.drawImage(0, cursor, image);
                cursor += image.height();
            } else { // placement == 1 -- frames on one line
                painter.drawImage(cursor, 0, image);
                cursor += image.width();
            }
        }
    }

    painter.end();

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

        switch (this->ui->contentTypeComboBox->currentIndex()) {
        case 0:
            result = this->exportFrames(progress);
            break;
        case 1:
            result = this->exportLevelSubtiles(progress);
            break;
        case 2:
            result = this->exportLevelTiles(progress);
            break;
        default: // case 3:
            result = this->exportLevelTiles25D(progress);
            break;
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
