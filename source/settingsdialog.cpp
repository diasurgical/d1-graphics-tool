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
    if( this->configurationChanged )
    {
        this->configuration->insert( "WorkingDirectory", this->workingDirectory );
        this->saveConfiguration();
        this->configurationChanged = false;
    }

    this->close();
}

void SettingsDialog::on_settingsCancelButton_clicked()
{
    this->close();
}
