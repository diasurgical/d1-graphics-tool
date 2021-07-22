#ifndef PALETTEWIDGET_H
#define PALETTEWIDGET_H

#include <QWidget>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QFileInfo>
#include <QDirIterator>
#include <QComboBox>
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
    explicit PaletteWidget(QWidget *parent = nullptr, QString title = "Palette");
    ~PaletteWidget();

    void initialize( D1Pal*, CelView* );
    void initialize( D1Pal*, LevelCelView* );

    void initialize( D1Pal*, D1Trn*, CelView* );
    void initialize( D1Pal*, D1Trn*, LevelCelView* );

    void initializeUi();
    void initializePathComboBox();

    void selectColor( quint8 );
    void addPath( QString, QString );

    // Coordinates functions
    QRectF getColorCoordinates( quint8 );
    quint8 getColorIndexFromCoordinates( QPointF );

    // Mouse event filter
    bool eventFilter( QObject*, QEvent* );

    // Display functions
    void displayColors();
    void displaySelection();

    void refreshPathComboBox();
    void refreshColorLineEdit();
    void refreshIndexLineEdit();
    void refreshTranslationIndexLineEdit();

    void refresh();

signals:
    void colorSelected( quint8 );
    void modified();
    void refreshed();

private slots:
    void pathComboBox_currentIndexChanged( int );
    void on_colorLineEdit_returnPressed();

    void on_translationIndexLineEdit_returnPressed();

private:
    Ui::PaletteWidget *ui;
    bool isCelLevel;
    bool isTrn;

    CelView *celView;
    LevelCelView *levelCelView;

    QGraphicsScene *scene;
    quint8 selectedColorIndex;
    QColor selectedColor;
    quint8 selectedTranslationIndex;

    D1Pal *pal;
    D1Trn *trn;

    bool buildingPathComboBox;
    QMap<QString,QString> paths;
};

#endif // PALETTEWIDGET_H
