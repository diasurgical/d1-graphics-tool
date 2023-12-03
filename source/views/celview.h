#pragma once

#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPoint>
#include <QStringList>
#include <QTimer>
#include <QWidget>

#include <memory>
#include <stack>

#include "d1formats/d1gfx.h"
#include "undostack/undostack.h"

#define CEL_SCENE_SPACING 8

namespace Ui {
class CelView;
} // namespace Ui

enum class IMAGE_FILE_MODE;

class CelScene : public QGraphicsScene {
    Q_OBJECT

public:
    CelScene(QWidget *view);

private slots:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

signals:
    void framePixelClicked(unsigned x, unsigned y);
    void showContextMenu(const QPoint &pos);

private:
    QWidget *view;
};

class CelView : public QWidget {
    Q_OBJECT

public:
    explicit CelView(std::shared_ptr<UndoStack> us, QWidget *parent = nullptr);
    ~CelView();

    void initialize(D1Gfx *gfx);
    void sendRemoveFrameCmd();
    void sendAddFrameCmd(IMAGE_FILE_MODE mode, int index, const QString &imagefilePath);
    void updateCurrentFrameIndex(int frameIdx);
    int getCurrentFrameIndex();
    void framePixelClicked(unsigned x, unsigned y);
    void insertImageFiles(IMAGE_FILE_MODE mode, const QStringList &imagefilePaths, bool append);
    void sendReplaceCurrentFrameCmd(const QString &imagefilePath);
    void replaceCurrentFrame(int frameIdx, const QImage &image);
    void removeCurrentFrame(int frameIdx);
    void regroupFrames(int numGroups);
    void updateGroupIndex();

    void displayFrame();

signals:
    void frameRefreshed();
    void colorIndexClicked(quint8);

private:
    void update();
    void removeFrames(int startingIndex, int endingIndex);
    void insertFrames(int startingIndex, const std::vector<QImage> &images);
    void setGroupIndex();

private slots:
    void on_firstFrameButton_clicked();
    void on_previousFrameButton_clicked();
    void on_nextFrameButton_clicked();
    void on_lastFrameButton_clicked();
    void on_frameIndexEdit_returnPressed();

    void on_firstGroupButton_clicked();
    void on_previousGroupButton_clicked();
    void on_groupIndexEdit_returnPressed();
    void on_nextGroupButton_clicked();
    void on_lastGroupButton_clicked();

    void on_zoomOutButton_clicked();
    void on_zoomInButton_clicked();
    void on_zoomEdit_returnPressed();

    void on_playDelayEdit_textChanged(const QString &text);
    void on_playButton_clicked();
    void on_stopButton_clicked();
    void playGroup();

    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    void ShowContextMenu(const QPoint &pos);
    void insertImageFile(int frameIdx, const QImage img);

private:
    std::stack<int> removedGroupIdxs;      // holds indexes of groups that have been removed, used for undo ops
    std::stack<int> removedFrameGroupIdxs; // holds group indexes of frames that got removed, used for undo ops

    std::shared_ptr<UndoStack> undoStack;
    Ui::CelView *ui;
    CelScene *celScene;

    D1Gfx *gfx;
    int currentGroupIndex = 0;
    int currentFrameIndex = 0;
    quint8 currentZoomFactor = 1;
    quint16 currentPlayDelay = 50;

    QTimer playTimer;
};
