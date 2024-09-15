#pragma once

#include <QDialog>

namespace Ui {
class ImportDialog;
}

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
