#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>

#include "d1celbase.h"
#include "d1min.h"
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
