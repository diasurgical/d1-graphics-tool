#pragma once

#include <QDialog>
#include <QJsonObject>

enum class OPEN_CLIPPING_TYPE {
    CLIPPED_AUTODETECT,
    CLIPPED_TRUE,
    CLIPPED_FALSE,
};

enum class OPEN_TILESET_TYPE {
    TILESET_AUTODETECT,
    TILESET_TRUE,
    TILESET_FALSE,
};

class OpenAsParam {
public:
    QString celFilePath;
    OPEN_TILESET_TYPE isTileset = OPEN_TILESET_TYPE::TILESET_AUTODETECT;

    quint16 celWidth = 0;
    OPEN_CLIPPING_TYPE clipped = OPEN_CLIPPING_TYPE::CLIPPED_AUTODETECT;

    QString tilFilePath;
    QString minFilePath;
    QString solFilePath;
    QString ampFilePath;
    quint16 minWidth = 0;
    quint16 minHeight = 0;
};

namespace Ui {
class OpenAsDialog;
}

class OpenAsDialog : public QDialog {
    Q_OBJECT

public:
    explicit OpenAsDialog(QWidget *parent = nullptr);
    ~OpenAsDialog();

    void initialize(QJsonObject *);

private:
    void update();

private slots:
    void on_inputFileBrowseButton_clicked();
    void on_isTilesetYesRadioButton_toggled(bool checked);
    void on_isTilesetNoRadioButton_toggled(bool checked);
    void on_isTilesetAutoRadioButton_toggled(bool checked);
    void on_tilFileBrowseButton_clicked();
    void on_minFileBrowseButton_clicked();
    void on_solFileBrowseButton_clicked();
    void on_ampFileBrowseButton_clicked();
    void on_openButton_clicked();
    void on_openCancelButton_clicked();

private:
    Ui::OpenAsDialog *ui;
    QJsonObject *configuration;
};
