#pragma once

#include <QDialog>
#include <QJsonObject>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    void initialize(QJsonObject *);

    static void storeConfiguration(QJsonObject *cfg);

private:
    void saveConfiguration();

signals:
    void configurationSaved();

private slots:
    void on_defaultPaletteColorPushButton_clicked();
    void on_paletteSelectionBorderColorPushButton_clicked();
    void on_settingsOkButton_clicked();
    void on_settingsCancelButton_clicked();

private:
    Ui::SettingsDialog *ui;
    QJsonObject *configuration;
};
