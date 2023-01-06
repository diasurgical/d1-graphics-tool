#pragma once

#include <QMainWindow>
#include <QStringList>
#include <QUndoCommand>

#include "celview.h"
#include "d1gfx.h"
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
#include "saveasdialog.h"
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

    void pushCommandToUndoStack(QUndoCommand *);

    void openFile(const OpenAsParam &params);
    void openImageFiles(QStringList filePaths, bool append);
    void saveFile(const SaveAsParam &params);
    void addFrames(bool append);

    void paletteWidget_callback(PaletteWidget *widget, PWIDGET_CALLBACK_TYPE type);

    void initPaletteCycle();
    void nextPaletteCycle(D1PAL_CYCLE_TYPE type);
    void resetPaletteCycle();

    QString getLastFilePath();
    QString fileDialog(bool save, const char *title, const char *filter);
    QStringList filesDialog(const char *title, const char *filter);

private:
    void loadConfiguration();
    void updateView();

public slots:
    void on_actionInsert_Frame_triggered();
    void on_actionAdd_Frame_triggered();
    void on_actionReplace_Frame_triggered();
    void on_actionDel_Frame_triggered();
    void on_actionCreate_Subtile_triggered();
    void on_actionClone_Subtile_triggered();
    void on_actionDel_Subtile_triggered();
    void on_actionCreate_Tile_triggered();
    void on_actionClone_Tile_triggered();
    void on_actionDel_Tile_triggered();

private slots:
    void on_actionOpen_triggered();
    void on_actionOpenAs_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
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
    QJsonObject *configuration = new QJsonObject();
    QString lastFilePath;

    QUndoStack *undoStack;
    QAction *undoAction;
    QAction *redoAction;

    QPointer<CelView> celView;
    QPointer<LevelCelView> levelCelView;

    QPointer<PaletteWidget> palWidget;
    QPointer<PaletteWidget> trn1Widget;
    QPointer<PaletteWidget> trn2Widget;

    QPointer<OpenAsDialog> openAsDialog = new OpenAsDialog(this);
    QPointer<SaveAsDialog> saveAsDialog = new SaveAsDialog(this);
    QPointer<SettingsDialog> settingsDialog = new SettingsDialog(this);
    QPointer<ExportDialog> exportDialog = new ExportDialog(this);

    QPointer<D1Pal> pal;
    QPointer<D1Trn> trn1;
    QPointer<D1Trn> trn2;
    QPointer<D1Gfx> gfx;
    QPointer<D1Min> min;
    QPointer<D1Til> til;
    QPointer<D1Sol> sol;
    QPointer<D1Amp> amp;

    QMap<QString, D1Pal *> pals;  // key: path, value: pointer to palette
    QMap<QString, D1Trn *> trn1s; // key: path, value: pointer to translation
    QMap<QString, D1Trn *> trn2s; // key: path, value: pointer to translation

    // Palette hits are instantiated in main window to make them available to the three PaletteWidgets
    QPointer<D1PalHits> palHits;
    // buffer to store the original colors in case of color cycling
    QColor origCyclePalette[32];
};
