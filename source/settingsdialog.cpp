#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    configurationChanged(false)
{
    ui->setupUi(this);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::initialize( QJsonObject *cfg )
{
    this->configuration = cfg;

    this->workingDirectory = this->configuration->value("WorkingDirectory").toString();
    this->ui->workingDirectoryEdit->setText( this->workingDirectory );

    QColor palDefaultColor = QColor( this->configuration->value("PaletteDefaultColor").toString() );
    this->ui->defaultPaletteColorLineEdit->setText( palDefaultColor.name() );

    QColor palSelectionBorderColor = QColor( this->configuration->value("PaletteSelectionBorderColor").toString() );
    this->ui->paletteSelectionBorderColorLineEdit->setText( palSelectionBorderColor.name() );
}

void SettingsDialog::saveConfiguration()
{
    QString jsonFilePath = QCoreApplication::applicationDirPath() + "/D1GraphicsTool.config.json";

    QFile saveJson( jsonFilePath );
    saveJson.open( QIODevice::WriteOnly );
    QJsonDocument saveDoc( *this->configuration );
    saveJson.write( saveDoc.toJson() );
    saveJson.close();

    emit this->configurationSaved();
}

void SettingsDialog::on_workingDirectoryBrowseButton_clicked()
{
    QString selectedDirectory = QFileDialog::getExistingDirectory(
        this, "Select Working Directory", QString(), QFileDialog::ShowDirsOnly );

    // If cancel button has been clicked in the file dialog
    if( selectedDirectory == "" )
        return;

    this->workingDirectory = selectedDirectory;
    this->configurationChanged = true;

    this->ui->workingDirectoryEdit->setText( this->workingDirectory );
}

void SettingsDialog::on_settingsOkButton_clicked()
{
    // WorkingDirectory
    this->configuration->insert( "WorkingDirectory", this->workingDirectory );

    // PaletteDefaultColor
    QColor palDefaultColor = QColor( ui->defaultPaletteColorLineEdit->text() );
    this->configuration->insert( "PaletteDefaultColor", palDefaultColor.name() );

    // PaletteSelectionBorderColor
    QColor palSelectionBorderColor = QColor( ui->paletteSelectionBorderColorLineEdit->text() );
    this->configuration->insert( "PaletteSelectionBorderColor", palSelectionBorderColor.name() );

    this->saveConfiguration();
    this->configurationChanged = false;

    this->close();
}

void SettingsDialog::on_settingsCancelButton_clicked()
{
    this->close();
}
