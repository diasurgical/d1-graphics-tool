#pragma once

#include <QDialog>
#include <QJsonObject>

enum class OPEN_CLIPPING_TYPE {
    CLIPPED_AUTODETECT,
    CLIPPED_TRUE,
    CLIPPED_FALSE,
};

class OpenAsParam {
public:
    quint16 width = 0;
    OPEN_CLIPPING_TYPE clipped = OPEN_CLIPPING_TYPE::CLIPPED_AUTODETECT;
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
    void on_openButton_clicked();
    void on_openCancelButton_clicked();

private:
    Ui::OpenAsDialog *ui;
    QJsonObject *configuration;
};
