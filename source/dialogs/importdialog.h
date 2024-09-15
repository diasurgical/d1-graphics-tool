#pragma once

#include <QDialog>

namespace Ui {
class ImportDialog;
}

// subtiles per line if the output is groupped, an odd number to ensure it is not recognized as a flat tile
#define EXPORT_SUBTILES_PER_LINE 15

// frames per line if the output of a tileset-frames is groupped, an odd number to ensure it is not recognized as a flat tile or as subtiles
#define EXPORT_LVLFRAMES_PER_LINE 31

class ImportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ImportDialog(QWidget *parent = nullptr);
    ~ImportDialog();

    void initialize();

private slots:
    void on_inputFileBrowseButton_clicked();
    void on_fontSymbolsEdit_textChanged(const QString &text);
    void on_fontColorButton_clicked();
    void on_importButton_clicked();
    void on_importCancelButton_clicked();

private:
    void setRenderColor(QColor color);
    QString getFileFormatExtension();

    Ui::ImportDialog *ui;
    QColor renderColor = QColor::fromRgb(204, 183, 117);
};
