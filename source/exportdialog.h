#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>

#include <QThread>

#include "d1cel.h"
#include "d1cl2.h"
#include "d1min.h"
#include "d1til.h"

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = 0);
    ~ExportDialog();

    void setCel(D1CelBase *);
    void setMin(D1Min *);
    void setTil(D1Til *);

    void setCurrentFrame(quint16);

    QString getFileFormatExtension();

private slots:
    void on_outputFolderBrowseButton_clicked();
    void on_exportButton_clicked();
    void on_exportCancelButton_clicked();
    void on_oneFileForAllFramesRadioButton_toggled(bool checked);

private:
    Ui::ExportDialog *ui;

    D1CelBase *cel;
    D1Min *min;
    D1Til *til;

    QString outputFolder;
};

#endif // EXPORTDIALOG_H
