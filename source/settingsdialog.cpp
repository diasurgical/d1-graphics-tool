#include "settingsdialog.h"

#include <QColorDialog>
#include <QFileDialog>

#include "config.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::initialize()
{
    QColor palDefaultColor = QColor(Config::value("PaletteDefaultColor").toString());
    this->ui->defaultPaletteColorLineEdit->setText(palDefaultColor.name());

    QColor palSelectionBorderColor = QColor(Config::value("PaletteSelectionBorderColor").toString());
    this->ui->paletteSelectionBorderColorLineEdit->setText(palSelectionBorderColor.name());
}

void SettingsDialog::on_defaultPaletteColorPushButton_clicked()
{
    QColor color = QColorDialog::getColor();
    this->ui->defaultPaletteColorLineEdit->setText(color.name());
}

void SettingsDialog::on_paletteSelectionBorderColorPushButton_clicked()
{
    QColor color = QColorDialog::getColor();
    this->ui->paletteSelectionBorderColorLineEdit->setText(color.name());
}

void SettingsDialog::on_settingsOkButton_clicked()
{
    // PaletteDefaultColor
    QColor palDefaultColor = QColor(ui->defaultPaletteColorLineEdit->text());
    Config::insert("PaletteDefaultColor", palDefaultColor.name());

    // PaletteSelectionBorderColor
    QColor palSelectionBorderColor = QColor(ui->paletteSelectionBorderColorLineEdit->text());
    Config::insert("PaletteSelectionBorderColor", palSelectionBorderColor.name());

    Config::storeConfiguration();

    emit this->configurationSaved();

    this->close();
}

void SettingsDialog::on_settingsCancelButton_clicked()
{
    this->close();
}
