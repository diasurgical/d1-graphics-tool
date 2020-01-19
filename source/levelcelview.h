#ifndef LEVELCELVIEW_H
#define LEVELCELVIEW_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QFileInfo>

#include "d1cel.h"
#include "d1min.h"
#include "d1til.h"

#define CEL_SCENE_SPACING 8

namespace Ui {
class LevelCelView;
}

class LevelCelView : public QWidget
{
    Q_OBJECT

public:
    explicit LevelCelView(QWidget *parent = 0);
    ~LevelCelView();

    void initialize( D1CelBase*, D1Min*, D1Til* );
    D1CelBase* getCel();
    QString getCelPath();
    D1Min* getMin();
    D1Til* getTil();

    quint32 getCurrentFrameIndex();
    quint16 getCurrentSubtileIndex();
    quint16 getCurrentTileIndex();

    void displayFrame();

signals:
    void frameChanged();
    void subtileChanges();
    void tileChanged();

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

    void on_zoomOutButton_clicked();
    void on_zoomInButton_clicked();
    void on_zoomEdit_returnPressed();

private:
    Ui::LevelCelView *ui;
    QGraphicsScene *celScene;

    D1CelBase *cel;
    D1Min *min;
    D1Til *til;
    quint32 currentFrameIndex;
    quint16 currentSubtileIndex;
    quint16 currentTileIndex;
    quint8 currentZoomFactor;
};

#endif // LEVELCELVIEW_H
