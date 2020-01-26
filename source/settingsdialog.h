#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    void initialize( QJsonObject* );

    void saveConfiguration();

private slots:
    void on_workingDirectoryBrowseButton_clicked();

    void on_settingsOkButton_clicked();

    void on_settingsCancelButton_clicked();

private:
    Ui::SettingsDialog *ui;
    QJsonObject *configuration;

    bool configurationChanged;
    QString workingDirectory;

};

#endif // SETTINGSDIALOG_H
