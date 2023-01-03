#pragma once

#include <QWidget>

namespace Ui {
class LevelTabSubTileWidget;
} // namespace Ui

class LevelCelView;
class D1Gfx;
class D1Min;
class D1Sol;

class LevelTabSubTileWidget : public QWidget {
    Q_OBJECT

public:
    explicit LevelTabSubTileWidget();
    ~LevelTabSubTileWidget();

    void initialize(LevelCelView *v, D1Gfx *g, D1Min *m, D1Sol *s);
    void update();

private slots:
    void on_sol0_clicked();
    void on_sol1_clicked();
    void on_sol2_clicked();
    void on_sol3_clicked();
    void on_sol4_clicked();
    void on_sol5_clicked();
    void on_sol7_clicked();

    void on_framesPrevButton_clicked();
    void on_framesComboBox_activated(int index);
    void on_framesComboBox_currentTextChanged(const QString &arg1);
    void on_framesNextButton_clicked();

private:
    void updateFramesSelection(int index);
    void updateSolProperty();
    quint8 readSol();

    Ui::LevelTabSubTileWidget *ui;
    LevelCelView *levelCelView;
    D1Gfx *gfx;
    D1Min *min;
    D1Sol *sol;

    bool onUpdate = false;
    int lastSubtileIndex = -1;
    int lastFrameEntryIndex = 0;
};
