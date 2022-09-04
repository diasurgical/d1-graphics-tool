#pragma once

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QWidget>

#include "d1amp.h"
#include "d1celbase.h"
#include "d1min.h"
#include "d1sol.h"
#include "d1til.h"

#define CEL_SCENE_SPACING 8

namespace Ui {
class LevelCelScene;
class LevelCelView;
} // namespace Ui

class LevelCelScene : public QGraphicsScene {
    Q_OBJECT

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

signals:
    void framePixelClicked(quint16, quint16);
};

class LevelCelView : public QWidget {
    Q_OBJECT

public:
    explicit LevelCelView(QWidget *parent = nullptr);
    ~LevelCelView();

    void initialize(D1CelBase *c, D1Min *m, D1Til *t, D1Sol *s, D1Amp *a);
    D1CelBase *getCel();
    QString getCelPath();
    D1Min *getMin();
    D1Til *getTil();

    quint32 getCurrentFrameIndex();
    quint16 getCurrentSubtileIndex();
    quint16 getCurrentTileIndex();
    void framePixelClicked(quint16, quint16);

    void displayFrame();

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

    void on_zoomOutButton_clicked();
    void on_zoomInButton_clicked();
    void on_zoomEdit_returnPressed();

    void on_ampType_activated(int index);

    void on_amp0_clicked();
    void on_amp1_clicked();
    void on_amp2_clicked();
    void on_amp3_clicked();
    void on_amp4_clicked();
    void on_amp5_clicked();
    void on_amp6_clicked();
    void on_amp7_clicked();

    void on_sol0_clicked();
    void on_sol1_clicked();
    void on_sol2_clicked();
    void on_sol3_clicked();
    void on_sol4_clicked();
    void on_sol5_clicked();
    void on_sol7_clicked();

private:
    void updateAmpProperty();
    void updateSolProperty();

    Ui::LevelCelView *ui;
    LevelCelScene *celScene = new LevelCelScene;

    D1CelBase *cel;
    D1Min *min;
    D1Til *til;
    D1Sol *sol;
    D1Amp *amp;
    quint32 currentFrameIndex = 0;
    quint16 currentSubtileIndex = 0;
    quint16 currentTileIndex = 0;
    quint8 currentZoomFactor = 1;
};
