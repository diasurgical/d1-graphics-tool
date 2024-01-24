#pragma once

#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>
#include <QImage>
#include <QPoint>
#include <QTimer>
#include <QWidget>

#include <memory>
#include <stack>
#include <utility>
#include <vector>

#include "celview.h"
#include "d1formats/d1amp.h"
#include "d1formats/d1gfx.h"
#include "d1formats/d1min.h"
#include "d1formats/d1sol.h"
#include "d1formats/d1til.h"
#include "undostack/undostack.h"
#include "widgets/leveltabframewidget.h"
#include "widgets/leveltabsubtilewidget.h"
#include "widgets/leveltabtilewidget.h"

namespace Ui {
class LevelCelView;
} // namespace Ui

enum class IMAGE_FILE_MODE;

enum class TILESET_MODE {
    FREE,    // Free navigation
    SUBTILE, // Subtile editing
    TILE,    // Edit tile
};

class LevelCelView : public QWidget {
    Q_OBJECT

public:
    explicit LevelCelView(std::shared_ptr<UndoStack> us, QWidget *parent = nullptr);
    ~LevelCelView();

    void initialize(D1Gfx *gfx, D1Min *min, D1Til *til, D1Sol *sol, D1Amp *amp);

    int getCurrentFrameIndex();
    int getCurrentSubtileIndex();
    int getCurrentTileIndex();

    void framePixelClicked(unsigned x, unsigned y);

    void insertImageFiles(IMAGE_FILE_MODE mode, const QStringList &imagefilePaths, bool append);

    void sendAddFrameCmd(IMAGE_FILE_MODE mode, int index, const QString &imagefilePath);

    void sendReplaceCurrentFrameCmd(const QString &imagefilePath);
    void replaceCurrentFrame(int frameIdx, const QImage &image);

    void sendRemoveFrameCmd();
    void removeCurrentFrame(int index);

    void createSubtile();
    void cloneSubtile();
    void replaceCurrentSubtile(const QString &imagefilePath);
    void removeCurrentSubtile();

    void createTile();
    void cloneTile();
    void replaceCurrentTile(const QString &imagefilePath);
    void removeCurrentTile();

    void reportUsage();
    void resetFrameTypes();
    void cleanupFrames();
    void cleanupSubtiles();
    void cleanupTileset();
    void compressTileset();
    void sortFrames();
    void sortSubtiles();

    void displayFrame();

private:
    void update();
    void collectFrameUsers(int frameIndex, QList<int> &users) const;
    void collectSubtileUsers(int subtileIndex, QList<int> &users) const;
    void insertFrame(IMAGE_FILE_MODE mode, int index, const QImage &image);
    void insertFrames(IMAGE_FILE_MODE mode, const QStringList &imagefilePaths, bool append);
    void insertFrames(int index, const QImage &image, IMAGE_FILE_MODE mode);
    void insertSubtile(int subtileIndex, const QImage &image);
    void insertSubtiles(IMAGE_FILE_MODE mode, int index, const QImage &image);
    void insertSubtiles(IMAGE_FILE_MODE mode, int index, const QString &imagefilePath);
    void insertSubtiles(IMAGE_FILE_MODE mode, const QStringList &imagefilePaths, bool append);
    void insertTile(int tileIndex, const QImage &image);
    void insertTiles(IMAGE_FILE_MODE mode, int index, const QImage &image);
    void insertTiles(IMAGE_FILE_MODE mode, int index, const QString &imagefilePath);
    void insertTiles(IMAGE_FILE_MODE mode, const QStringList &imagefilePaths, bool append);
    void assignFrames(const QImage &image, int subtileIndex, int frameIndex);
    void assignSubtiles(const QImage &image, int tileIndex, int subtileIndex);
    void removeFrame(int frameIndex);
    void removeSubtile(int subtileIndex);
    void removeUnusedFrames(QString &report);
    void removeUnusedSubtiles(QString &report);
    void reuseFrames(QString &report);
    void reuseSubtiles(QString &report);
    bool sortFrames_impl();
    bool sortSubtiles_impl();

signals:
    void frameRefreshed();
    void colorIndexClicked(quint8);

private slots:
    void on_firstFrameButton_clicked();
    void on_previousFrameButton_clicked();
    void on_nextFrameButton_clicked();
    void on_lastFrameButton_clicked();
    void on_frameIndexEdit_returnPressed();

    void on_firstSubtileButton_clicked();
    void on_previousSubtileButton_clicked();
    void on_nextSubtileButton_clicked();
    void on_lastSubtileButton_clicked();
    void on_subtileIndexEdit_returnPressed();

    void on_firstTileButton_clicked();
    void on_previousTileButton_clicked();
    void on_nextTileButton_clicked();
    void on_lastTileButton_clicked();
    void on_tileIndexEdit_returnPressed();

    void on_minFrameHeightEdit_returnPressed();

    void on_zoomOutButton_clicked();
    void on_zoomInButton_clicked();
    void on_zoomEdit_returnPressed();

    void on_playDelayEdit_textChanged(const QString &text);
    void on_playButton_clicked();
    void on_stopButton_clicked();

    void on_addTileButton_clicked();
    void on_cloneTileButton_clicked();
    void on_addSubTileButton_clicked();
    void on_cloneSubTileButton_clicked();

    void playGroup();

    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    void ShowContextMenu(const QPoint &pos);
    void insertFrame(int index, QImage image);

private:
    std::shared_ptr<UndoStack> undoStack;
    Ui::LevelCelView *ui;
    CelScene *celScene;
    LevelTabTileWidget *tabTileWidget = new LevelTabTileWidget();
    LevelTabSubTileWidget *tabSubTileWidget = new LevelTabSubTileWidget();
    LevelTabFrameWidget *tabFrameWidget = new LevelTabFrameWidget();

    TILESET_MODE mode = TILESET_MODE::FREE;
    int editIndex = 0;

    std::stack<std::vector<std::pair<int, int>>> tilesAndFramesIdxStack; // stores tile index + frame indices list index

    D1Gfx *gfx;
    D1Min *min;
    D1Til *til;
    D1Sol *sol;
    D1Amp *amp;
    int currentFrameIndex = 0;
    int currentSubtileIndex = 0;
    int currentTileIndex = 0;
    quint8 currentZoomFactor = 1;
    quint16 currentPlayDelay = 50;

    QTimer playTimer;
};
