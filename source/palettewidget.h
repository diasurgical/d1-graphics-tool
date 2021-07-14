#ifndef PALETTEWIDGET_H
#define PALETTEWIDGET_H

#include <QWidget>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QFileInfo>
#include <QDirIterator>
#include <QMouseEvent>

#include "celview.h"
#include "levelcelview.h"
#include "d1pal.h"
#include "d1trn.h"

#define PALETTE_WIDTH 224
#define PALETTE_COLORS_PER_LINE 16
#define PALETTE_COLOR_SPACING 1
#define PALETTE_SELECTION_WIDTH 2

namespace Ui {
class PaletteWidget;
}

class PaletteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PaletteWidget(QWidget *parent = nullptr);
    ~PaletteWidget();

    void initialize( D1Pal*, CelView* );
    void initialize( D1Pal*, LevelCelView* );

    void initialize( D1Pal*, D1Trn*, CelView* );
    void initialize( D1Pal*, D1Trn*, LevelCelView* );

    // Coordinates functions
    QRectF getColorCoordinates( quint8 );

    // Mouse event filter
    bool eventFilter( QObject*, QEvent* );

    // Display functions
    void displayColors();
    void displaySelection();

    void refresh();

signals:
    void refreshed();

private:
    Ui::PaletteWidget *ui;
    bool isCelLevel;
    bool isTrn;

    CelView *celView;
    LevelCelView *levelCelView;

    QGraphicsScene *scene;
    quint8 selectedColorIndex;

    D1Pal *pal;
    D1Trn *trn;

    QMap<QString,QString> paths;
};

#endif // PALETTEWIDGET_H
