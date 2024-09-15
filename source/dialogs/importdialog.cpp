#include "importdialog.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_importdialog.h"

ImportDialog::ImportDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ImportDialog())
{
    ui->setupUi(this);
}

ImportDialog::~ImportDialog()
{
    delete ui;
}

void ImportDialog::initialize()
{
    // - remember the last selected type
    QComboBox *typeBox = this->ui->typeComboBox;
    QString lastFmt = typeBox->currentText();
    if (lastFmt.isEmpty()) {
        lastFmt = "Font";
    }
    typeBox->clear();
    typeBox->addItem("Font");
    typeBox->setCurrentIndex(typeBox->findText(lastFmt));

    setRenderColor(this->renderColor);
}

void ImportDialog::setRenderColor(QColor color)
{
    QString red = QString::number(color.red());
    QString green = QString::number(color.green());
    QString blue = QString::number(color.blue());
    QString styleSheet = QString("background: rgb(") + red + QString(",") + green + QString(",") + blue + QString(")");
    ui->fontColorButton->setStyleSheet(styleSheet);
    this->renderColor = color;
}

QString ImportDialog::getFileFormatExtension()
{
    return "." + this->ui->typeComboBox->currentText().toLower();
}

void ImportDialog::on_inputFileBrowseButton_clicked()
{
    QString selectedDirectory = QFileDialog::getOpenFileName(
        this, "Select Font File", QString(), "Fonts (*.ttf *.otf)");

    if (selectedDirectory.isEmpty())
        return;

    ui->inputFileEdit->setText(selectedDirectory);
}

void ImportDialog::on_fontSymbolsEdit_textChanged(const QString &text)
{
    bool ok = false;
    uint test = text.toUInt(&ok, 16);
    if (!ok) {
        ui->fontSymbolsRangeLabel->setText("Error");
        return;
    }

    QString pad = text.toLower();
    while (pad.size() < 2)
        pad = "0" + pad;

    QString start = "U+" + pad + "00";
    QString end = "U+" + pad + "ff";
    ui->fontSymbolsRangeLabel->setText(start + " - " + end);
}

void ImportDialog::on_fontColorButton_clicked()
{
    QColor color = QColorDialog::getColor(this->renderColor);

    if (color.isValid())
        setRenderColor(color);
}

void ImportDialog::on_importButton_clicked()
{
    if (ui->inputFileEdit->text() == "") {
        QMessageBox::warning(this, "Warning", "Input file is missing, please choose an input folder.");
        return;
    }

    try {
        MainWindow *mainWindow = dynamic_cast<MainWindow *>(this->parent());
        if (mainWindow == nullptr) {
            QMessageBox::critical(this, "Error", "Window not found.");
            return;
        }

        QString filePath = ui->inputFileEdit->text();
        int pointSize = ui->fontSizeEdit->text().toInt();
        uint symbolPrefix = ui->fontSymbolsEdit->text().toUInt() << 8;
        mainWindow->openFontFile(filePath, this->renderColor, pointSize, symbolPrefix);
    } catch (...) {
        QMessageBox::critical(this, "Error", "Import Failed.");
        return;
    }

    this->close();
}

void ImportDialog::on_importCancelButton_clicked()
{
    this->close();
}
