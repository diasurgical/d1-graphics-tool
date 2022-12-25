#include "settingsdialog.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QJsonDocument>

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

void SettingsDialog::initialize(QJsonObject *cfg)
{
    this->configuration = cfg;

    QColor palDefaultColor = QColor(this->configuration->value("PaletteDefaultColor").toString());
    this->ui->defaultPaletteColorLineEdit->setText(palDefaultColor.name());

    QColor palSelectionBorderColor = QColor(this->configuration->value("PaletteSelectionBorderColor").toString());
    this->ui->paletteSelectionBorderColorLineEdit->setText(palSelectionBorderColor.name());
}

void SettingsDialog::storeConfiguration(QJsonObject *cfg)
{
    QString jsonFilePath = QCoreApplication::applicationDirPath() + "/D1GraphicsTool.config.json";

    QFile saveJson(jsonFilePath);
    saveJson.open(QIODevice::WriteOnly);
    QJsonDocument saveDoc(*cfg);
    saveJson.write(saveDoc.toJson());
    saveJson.close();
}

void SettingsDialog::saveConfiguration()
{
    SettingsDialog::storeConfiguration(this->configuration);

    emit this->configurationSaved();
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
    this->configuration->insert("PaletteDefaultColor", palDefaultColor.name());

    // PaletteSelectionBorderColor
    QColor palSelectionBorderColor = QColor(ui->paletteSelectionBorderColorLineEdit->text());
    this->configuration->insert("PaletteSelectionBorderColor", palSelectionBorderColor.name());

    this->saveConfiguration();

    this->close();
}

void SettingsDialog::on_settingsCancelButton_clicked()
{
    this->close();
}
