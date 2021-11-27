#ifndef BATCHEXPORTDIALOG_H
#define BATCHEXPORTDIALOG_H

#include <QDialog>

namespace Ui {
class BatchExportDialog;
}

class BatchExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BatchExportDialog(QWidget *parent = nullptr);
    ~BatchExportDialog();

private slots:
    void on_batchExportButton_clicked();

private:
    Ui::BatchExportDialog *ui;
};

#endif // BATCHEXPORTDIALOG_H
