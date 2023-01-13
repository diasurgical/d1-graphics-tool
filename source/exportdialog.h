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

// subtiles per line if the output is groupped, an odd number to ensure it is not recognized as a flat tile
#define EXPORT_SUBTILES_PER_LINE 15

// frames per line if the output of a tileset-frames is groupped, an odd number to ensure it is not recognized as a flat tile or as subtiles
#define EXPORT_LVLFRAMES_PER_LINE 31

class ExportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    ~ExportDialog();

    void initialize(D1Gfx *gfx, D1Min *min, D1Til *til, D1Sol *sol, D1Amp *amp);

private slots:
    void on_outputFolderBrowseButton_clicked();
    void on_exportButton_clicked();
    void on_exportCancelButton_clicked();

private:
    QString getFileFormatExtension();

    bool exportLevelTiles25D(QProgressDialog &progress);
    bool exportLevelTiles(QProgressDialog &progress);
    bool exportLevelSubtiles(QProgressDialog &progress);
    bool exportFrames(QProgressDialog &progress);

    Ui::ExportDialog *ui;

    D1Gfx *gfx = nullptr;
    D1Min *min = nullptr;
    D1Til *til = nullptr;
    D1Sol *sol = nullptr;
    D1Amp *amp = nullptr;

    QString outputFolder;
};
