#pragma once

#include <optional>

#include <QDialog>
#include <QFile>
#include <QJsonObject>
#include <QString>

class D1CelBase;

enum class SAVE_CLIPPING_TYPE {
    CLIPPED_AUTODETECT,
    CLIPPED_TRUE,
    CLIPPED_FALSE,
};

class SaveAsParam {
public:
    QString celFilePath;
    QString tilFilePath;
    QString minFilePath;
    QString solFilePath;
    QString ampFilePath;
    quint16 groupNum = 0;
    SAVE_CLIPPING_TYPE clipped = SAVE_CLIPPING_TYPE::CLIPPED_AUTODETECT;

    static std::optional<QFile *> getValidSaveOutput(QString filePath, QString selectedPath);
};

namespace Ui {
class SaveAsDialog;
}

class SaveAsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SaveAsDialog(QWidget *parent = nullptr);
    ~SaveAsDialog();

    void initialize(QJsonObject *cfg, D1CelBase *cel, bool isTileset);

private:
    void update();

private slots:
    void on_outputCelFileBrowseButton_clicked();
    void on_outputMinFileBrowseButton_clicked();
    void on_outputTilFileBrowseButton_clicked();
    void on_outputSolFileBrowseButton_clicked();
    void on_outputAmpFileBrowseButton_clicked();
    void on_saveButton_clicked();
    void on_saveCancelButton_clicked();

private:
    Ui::SaveAsDialog *ui;
    QJsonObject *configuration;
    D1CelBase *cel;
    bool isTileset;
};
