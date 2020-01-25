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

    this->workingFolder = this->configuration->value("WorkingFolder").toString();
    this->ui->workingFolderEdit->setText( this->workingFolder );
}

void SettingsDialog::saveConfiguration()
{
    QString jsonFilePath = QCoreApplication::applicationDirPath() + "/D1GraphicsTool.config.json";

    QFile saveJson( jsonFilePath );
    saveJson.open( QIODevice::WriteOnly );
    QJsonDocument saveDoc( *this->configuration );
    saveJson.write( saveDoc.toJson() );
    saveJson.close();
}

void SettingsDialog::on_workingFolderBrowseButton_clicked()
{
    QString selectedFolder = QFileDialog::getExistingDirectory(
        this, "Select Working Folder", QString(), QFileDialog::ShowDirsOnly );

    // If cancel button has been clicked in the file dialog
    if( selectedFolder == "" )
        return;

    this->workingFolder = selectedFolder;
    this->configurationChanged = true;

    this->ui->workingFolderEdit->setText( this->workingFolder );
}

void SettingsDialog::on_settingsOkButton_clicked()
{
    if( this->configurationChanged )
    {
        this->configuration->insert( "WorkingFolder", this->workingFolder );
        this->saveConfiguration();
        this->configurationChanged = false;
    }

    this->close();
}

void SettingsDialog::on_settingsCancelButton_clicked()
{
    this->close();
}
