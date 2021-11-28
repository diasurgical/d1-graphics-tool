#ifndef BATCHEXPORTDIALOG_H
#define BATCHEXPORTDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include <QFileDialog>
#include <QMessageBox>

#include "export.h"

namespace Ui {
class BatchExportDialog;
}

class BatchExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BatchExportDialog(QJsonObject *configuration, QWidget *parent = nullptr);
    ~BatchExportDialog();

    void setInputFolder(QString);

    void exportAllDiabloLevelTilesForTiled();
    void exportAllHellfireLevelTilesForTiled();

private slots:
    void on_inputFolderBrowseButton_clicked();
    void on_outputFolderBrowseButton_clicked();
    void on_batchExportButton_clicked();
    void on_batchExportCancelButton_clicked();

private:
    QJsonObject *configuration;

    Ui::BatchExportDialog *ui;
};

#endif // BATCHEXPORTDIALOG_H
