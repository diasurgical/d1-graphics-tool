#pragma once

#include <QDialog>
#include <QProgressDialog>

#include "d1amp.h"
#include "d1celbase.h"
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

    void setCel(D1CelBase *c);
    void setMin(D1Min *m);
    void setTil(D1Til *t);
    void setAmp(D1Amp *a);
    void setSol(D1Sol *s);

    void setCurrentFrame(quint16);

    QString getFileFormatExtension();

private slots:
    void on_outputFolderBrowseButton_clicked();
    void on_exportButton_clicked();
    void on_exportCancelButton_clicked();
    void on_oneFileForAllFramesRadioButton_toggled(bool checked);

    void on_diabloButton_toggled(bool checked);

private:
    bool exportLevelDiablo(QProgressDialog &progress);
    bool exportLevelTiles(QProgressDialog &progress);
    bool exportLevelSubtiles(QProgressDialog &progress);
    bool exportLevel(QProgressDialog &progress);
    bool exportSprites(QProgressDialog &progress);

    Ui::ExportDialog *ui;

    D1CelBase *cel = nullptr;
    D1Min *min = nullptr;
    D1Til *til = nullptr;
    D1Amp *amp = nullptr;
    D1Sol *sol = nullptr;

    QString outputFolder;
};
