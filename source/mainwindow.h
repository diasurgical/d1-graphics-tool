#pragma once

#include <QMainWindow>
#include <QMenu>
#include <QMimeData>
#include <QString>
#include <QStringList>

#include <memory>

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
#include "settingsdialog.h"
#include <undostack.h>

#define D1_GRAPHICS_TOOL_TITLE "Diablo 1 Graphics Tool"
#define D1_GRAPHICS_TOOL_VERSION "1.0.1"

enum class FILE_DIALOG_MODE {
    OPEN,         // open existing
    SAVE_CONF,    // save with confirm
    SAVE_NO_CONF, // save without confirm
};

enum class IMAGE_FILE_MODE {
    FRAME,   // open as frames
    SUBTILE, // open as subtiles
    TILE,    // open as tiles
    AUTO,    // auto-detect
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow();
    ~MainWindow();

    void setPal(QString);
    void setTrn(QString);
    void setTrnUnique(QString);

    void openFile(const OpenAsParam &params);
    void openImageFiles(IMAGE_FILE_MODE mode, QStringList filePaths, bool append);
    void openPalFiles(QStringList filePaths, PaletteWidget *widget);
    void saveFile(const QString &gfxPath);

    void paletteWidget_callback(PaletteWidget *widget, PWIDGET_CALLBACK_TYPE type);

    void nextPaletteCycle(D1PAL_CYCLE_TYPE type);
    void resetPaletteCycle();

    QString getLastFilePath();
    QString fileDialog(FILE_DIALOG_MODE mode, const char *title, const char *filter);
    QStringList filesDialog(const char *title, const char *filter);

    static bool hasImageUrl(const QMimeData *mimeData);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void updateWindow();
    bool loadPal(QString palFilePath);
    bool loadTrn(QString trnfilePath);
    bool loadTrnUnique(QString trnfilePath);

    void addFrames(bool append);
    void addSubtiles(bool append);
    void addTiles(bool append);

public slots:
    void actionUndo_triggered();
    void actionRedo_triggered();

    void actionInsertFrame_triggered();
    void actionAddFrame_triggered();
    void actionReplaceFrame_triggered();
    void actionDelFrame_triggered();

    void actionCreateSubtile_triggered();
    void actionInsertSubtile_triggered();
    void actionAddSubtile_triggered();
    void actionReplaceSubtile_triggered();
    void actionCloneSubtile_triggered();
    void actionDelSubtile_triggered();

    void actionCreateTile_triggered();
    void actionCloneTile_triggered();
    void actionInsertTile_triggered();
    void actionAddTile_triggered();
    void actionReplaceTile_triggered();
    void actionDelTile_triggered();

    void buildRecentFilesMenu();
    void addRecentFile(QString filePath);
    void on_actionClear_History_triggered();

private slots:
    void actionNewSprite_triggered();
    void actionNewTileset_triggered();

    void on_actionOpen_triggered();
    void on_actionOpenAs_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    bool isOkToQuit();
    void on_actionClose_triggered();
    void closeAllElements();
    void on_actionExport_triggered();
    void on_actionSettings_triggered();
    void on_actionQuit_triggered();

    void on_actionRegroupFrames_triggered();

    void on_actionReportUse_Tileset_triggered();
    void on_actionResetFrameTypes_Tileset_triggered();
    void on_actionCleanupFrames_Tileset_triggered();
    void on_actionCleanupSubtiles_Tileset_triggered();
    void on_actionCompressTileset_Tileset_triggered();
    void on_actionSortFrames_Tileset_triggered();
    void on_actionSortSubtiles_Tileset_triggered();

    void on_actionNew_PAL_triggered();
    void on_actionOpen_PAL_triggered();
    void on_actionSave_PAL_triggered();
    void on_actionSave_PAL_as_triggered();
    void on_actionClose_PAL_triggered();

    void on_actionNew_Unique_Translation_triggered();
    void on_actionOpen_Unique_Translation_triggered();
    void on_actionSave_Unique_Translation_triggered();
    void on_actionSave_Unique_Translation_as_triggered();
    void on_actionClose_Unique_Translation_triggered();

    void on_actionNew_Translation_triggered();
    void on_actionOpen_Translation_triggered();
    void on_actionSave_Translation_triggered();
    void on_actionSave_Translation_as_triggered();
    void on_actionClose_Translation_triggered();

    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();

    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    Ui::MainWindow *ui;
    QString lastFilePath;

    QMenu newMenu = QMenu("New");
    QMenu frameMenu = QMenu("Frame");
    QMenu subtileMenu = QMenu("Tile");
    QMenu tileMenu = QMenu("MegaTile");

    std::shared_ptr<UndoStack> undoStack;
    QAction *undoAction;
    QAction *redoAction;

    QPointer<CelView> celView;
    QPointer<LevelCelView> levelCelView;

    QPointer<PaletteWidget> palWidget;
    QPointer<PaletteWidget> trnWidget;
    QPointer<PaletteWidget> trnUniqueWidget;

    OpenAsDialog openAsDialog = OpenAsDialog(this);
    SettingsDialog settingsDialog = SettingsDialog(this);
    ExportDialog exportDialog = ExportDialog(this);

    QPointer<D1Pal> pal;
    QPointer<D1Trn> trn;
    QPointer<D1Trn> trnUnique;
    QPointer<D1Gfx> gfx;
    QPointer<D1Min> min;
    QPointer<D1Til> til;
    QPointer<D1Sol> sol;
    QPointer<D1Amp> amp;

    QMap<QString, D1Pal *> pals;       // key: path, value: pointer to palette
    QMap<QString, D1Trn *> trns;       // key: path, value: pointer to translation
    QMap<QString, D1Trn *> trnsUnique; // key: path, value: pointer to translation

    // Palette hits are instantiated in main window to make them available to the three PaletteWidgets
    QPointer<D1PalHits> palHits;
};
