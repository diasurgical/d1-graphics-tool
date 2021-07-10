#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>

#include "d1pal.h"
#include "d1trn.h"
#include "d1cel.h"
#include "d1cl2.h"
#include "d1min.h"
#include "d1til.h"

#include "palview.h"
#include "palettewidget.h"
#include "celview.h"
#include "levelcelview.h"
#include "settingsdialog.h"
#include "exportdialog.h"

#define D1_GRAPHICS_TOOL_VERSION "0.5.0"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow( QWidget *parent = 0 );
    ~MainWindow();

    void loadConfiguration();

private slots:
    void on_actionOpen_triggered();
    void on_actionClose_triggered();
    void on_actionExport_triggered();
    void on_actionSettings_triggered();
    void on_actionQuit_triggered();

    void on_actionLoad_PAL_triggered();
    void on_actionLoad_Translation_1_triggered();
    void on_actionLoad_Translation_2_triggered();
    void on_actionReset_PAL_triggered();
    void on_actionReset_Translation_1_triggered();
    void on_actionReset_Translation_2_triggered();

    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();

private:
    Ui::MainWindow *ui;
    QJsonObject *configuration;

    QPointer<CelView> celView;
    QPointer<LevelCelView> levelCelView;

    QPointer<PalView> palView;

    QPointer<PaletteWidget> palWidget;
    QPointer<PaletteWidget> trn1Widget;
    QPointer<PaletteWidget> trn2Widget;

    QPointer<SettingsDialog> settingsDialog;
    QPointer<ExportDialog> exportDialog;

    QPointer<D1Pal> pal;
    QPointer<D1Trn> trn1;
    QPointer<D1Trn> trn2;
    QPointer<D1CelBase> cel;
    QPointer<D1Min> min;
    QPointer<D1Til> til;
};

#endif // MAINWINDOW_H
