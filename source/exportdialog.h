#pragma once

#include <QDialog>
#include <QProgressDialog>

#include "d1amp.h"
#include "d1gfx.h"
#include "d1min.h"
#include "d1sol.h"
#include "d1til.h"

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    ~ExportDialog();

    void initialize(QJsonObject *cfg, D1Gfx *gfx, D1Min *min, D1Til *til, D1Sol *sol, D1Amp *amp);

    void setCurrentFrame(quint16);

    QString getFileFormatExtension();

private slots:
    void on_outputFolderBrowseButton_clicked();
    void on_exportButton_clicked();
    void on_exportCancelButton_clicked();
    void on_oneFileForAllFramesRadioButton_toggled(bool checked);

private:
    bool exportLevelTiles(QProgressDialog &progress);
    bool exportLevelSubtiles(QProgressDialog &progress);
    bool exportLevel(QProgressDialog &progress);
    bool exportSprites(QProgressDialog &progress);

    Ui::ExportDialog *ui;
    QJsonObject *configuration;

    D1Gfx *gfx = nullptr;
    D1Min *min = nullptr;
    D1Til *til = nullptr;
    D1Sol *sol = nullptr;
    D1Amp *amp = nullptr;

    QString outputFolder;
};
