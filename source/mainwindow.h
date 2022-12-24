#pragma once

#include <QMainWindow>

#include "celview.h"
#include "d1celbase.h"
#include "d1min.h"
#include "d1pal.h"
#include "d1palhits.h"
#include "d1sol.h"
#include "d1til.h"
#include "d1trn.h"
#include "exportdialog.h"
#include "levelcelview.h"
#include "openasdialog.h"
#include "palettewidget.h"
#include "settingsdialog.h"

#define D1_GRAPHICS_TOOL_VERSION "0.5.0"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setPal(QString);
    void setTrn1(QString);
    void setTrn2(QString);

    void loadConfiguration();

    void pushCommandToUndoStack(QUndoCommand *);

    void openFile(QString openFilePath, OpenAsParam *params = nullptr);

private slots:
    void on_actionOpen_triggered();
    void on_actionOpenAs_triggered();
    void on_actionClose_triggered();
    void on_actionExport_triggered();
    void on_actionSettings_triggered();
    void on_actionQuit_triggered();

    void on_actionNew_PAL_triggered();
    void on_actionOpen_PAL_triggered();
    void on_actionSave_PAL_triggered();
    void on_actionSave_PAL_as_triggered();
    void on_actionClose_PAL_triggered();

    void on_actionNew_Translation_1_triggered();
    void on_actionOpen_Translation_1_triggered();
    void on_actionSave_Translation_1_triggered();
    void on_actionSave_Translation_1_as_triggered();
    void on_actionClose_Translation_1_triggered();

    void on_actionNew_Translation_2_triggered();
    void on_actionOpen_Translation_2_triggered();
    void on_actionSave_Translation_2_triggered();
    void on_actionSave_Translation_2_as_triggered();
    void on_actionClose_Translation_2_triggered();

    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    Ui::MainWindow *ui;
    QJsonObject *configuration = new QJsonObject;

    QUndoStack *undoStack;
    QAction *undoAction;
    QAction *redoAction;

    QPointer<CelView> celView;
    QPointer<LevelCelView> levelCelView;

    QPointer<PaletteWidget> palWidget;
    QPointer<PaletteWidget> trn1Widget;
    QPointer<PaletteWidget> trn2Widget;

    QPointer<OpenAsDialog> openAsDialog = new OpenAsDialog(this);
    QPointer<SettingsDialog> settingsDialog = new SettingsDialog(this);
    QPointer<ExportDialog> exportDialog = new ExportDialog(this);

    QPointer<D1Pal> pal;
    QPointer<D1Trn> trn1;
    QPointer<D1Trn> trn2;
    QPointer<D1CelBase> cel;
    QPointer<D1Min> min;
    QPointer<D1Til> til;
    QPointer<D1Sol> sol;
    QPointer<D1Amp> amp;

    QMap<QString, D1Pal *> pals;  // key: path, value: pointer to palette
    QMap<QString, D1Trn *> trn1s; // key: path, value: pointer to translation
    QMap<QString, D1Trn *> trn2s; // key: path, value: pointer to translation

    // Palette hits are instantiated in main window to make them available to the three PaletteWidgets
    QPointer<D1PalHits> palHits;
};
