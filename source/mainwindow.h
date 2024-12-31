#pragma once

#include <QMainWindow>
#include <QMenu>
#include <QMimeData>
#include <QString>
#include <QStringList>

#include <memory>

#include "d1formats/d1gfx.h"
#include "d1formats/d1min.h"
#include "d1formats/d1sol.h"
#include "d1formats/d1til.h"
#include "d1formats/d1trn.h"
#include "dialogs/exportdialog.h"
#include "dialogs/importdialog.h"
#include "dialogs/openasdialog.h"
#include "dialogs/settingsdialog.h"
#include "palette/d1pal.h"
#include "palette/d1palhits.h"
#include "undostack/undostack.h"
#include "views/celview.h"
#include "views/levelcelview.h"
#include "widgets/palettewidget.h"

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

namespace mw {
    bool QuestionDiscardChanges(bool isModified, QString filePath);
} // namespace mw

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow();
    ~MainWindow();

    void setPal(const QString&);
    void setTrn(const QString&);
    void setTrnUnique(const QString&);

    void openFile(const OpenAsParam &params);
    void openImageFiles(IMAGE_FILE_MODE mode, QStringList filePaths, bool append);
    void openPalFiles(const QStringList& filePaths, PaletteWidget *widget) const;
    void openFontFile(QString filePath, QColor renderColor, int pointSize, uint symbolPrefix);
    void saveFile(const QString &gfxPath);

    void paletteWidget_callback(PaletteWidget *widget, PWIDGET_CALLBACK_TYPE type);

    void nextPaletteCycle(D1PAL_CYCLE_TYPE type);
    void resetPaletteCycle();
    void updateStatusBar(const QString &status, const QString &styleSheet);

    QString getLastFilePath();
    QString fileDialog(FILE_DIALOG_MODE mode, const char *title, const char *filter);
    QStringList filesDialog(const char *title, const char *filter);
    PaletteWidget *trnWidget() { return m_trnWidget; }
    PaletteWidget *uniqTrnWidget() { return m_trnUniqueWidget; }
    PaletteWidget *paletteWidget() { return m_palWidget; }

    static bool hasImageUrl(const QMimeData *mimeData);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void updateWindow();

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

    // slots used for UndoMacro signals
    void setupUndoMacroWidget(std::unique_ptr<UserData> &userData, enum OperationType opType);
    void updateUndoMacroWidget(bool &result);

    void buildRecentFilesMenu();
    void addRecentFile(QString filePath);
    void on_actionClear_History_triggered();

private slots:
    void actionNewSprite_triggered();
    void actionNewTileset_triggered();

    void on_actionOpen_triggered();
    void on_actionOpenAs_triggered();
    void on_actionImport_triggered();
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

    PaletteWidget* m_palWidget = nullptr;
    PaletteWidget* m_trnWidget = nullptr;
    PaletteWidget* m_trnUniqueWidget = nullptr;

    OpenAsDialog openAsDialog = OpenAsDialog(this);
    SettingsDialog settingsDialog = SettingsDialog(this);
    ImportDialog importDialog = ImportDialog(this);
    ExportDialog exportDialog = ExportDialog(this);

    QPointer<D1Gfx> gfx;
    QPointer<D1Min> min;
    QPointer<D1Til> til;
    QPointer<D1Sol> sol;
    QPointer<D1Amp> amp;

    std::unique_ptr<QProgressDialog> m_progressDialog;

    // Palette hits are instantiated in main window to make them available to the three PaletteWidgets
    QPointer<D1PalHits> palHits;

    int m_currProgDialogPos { 0 };
    enum OperationType m_currMacroOpType;
};
