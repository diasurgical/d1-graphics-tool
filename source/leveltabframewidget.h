#pragma once

#include <QWidget>

namespace Ui {
class LevelTabFrameWidget;
} // namespace Ui

class LevelCelView;
class D1Gfx;
class D1GfxFrame;
enum class D1CEL_FRAME_TYPE;

class LevelTabFrameWidget : public QWidget {
    Q_OBJECT

public:
    explicit LevelTabFrameWidget();
    ~LevelTabFrameWidget();

    void initialize(LevelCelView *v, D1Gfx *g);
    void update();

    static D1CEL_FRAME_TYPE altFrameType(D1GfxFrame *frame, int *limit);
    static void selectFrameType(D1GfxFrame *frame);

private slots:
    void on_frameTypeComboBox_activated(int index);

private:
    void validate();

    Ui::LevelTabFrameWidget *ui;
    LevelCelView *levelCelView;
    D1Gfx *gfx;
};
