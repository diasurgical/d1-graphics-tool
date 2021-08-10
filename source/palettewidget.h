#ifndef PALETTEWIDGET_H
#define PALETTEWIDGET_H

#include <QWidget>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QFileInfo>
#include <QDirIterator>
#include <QComboBox>
#include <QMouseEvent>
#include <QColorDialog>

#include "celview.h"
#include "levelcelview.h"
#include "d1pal.h"
#include "d1trn.h"
#include "d1palhits.h"

#define PALETTE_WIDTH 192
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

    void setPal( D1Pal* );
    void setTrn( D1Trn* );

    void initialize( D1Pal*, CelView*, D1PalHits* );
    void initialize( D1Pal*, LevelCelView*, D1PalHits* );

    void initialize( D1Pal*, D1Trn*, CelView*, D1PalHits* );
    void initialize( D1Pal*, D1Trn*, LevelCelView*, D1PalHits* );

    void initializeUi();
    void initializePathComboBox();
    void initializeDisplayComboBox();

    void selectColor( quint8 );
    void checkTranslationSelection( quint8 );

    QString getPath( QString );
    void setPath( QString, QString );
    void addPath( QString, QString );
    void removePath( QString );
    void selectPath( QString );
    QString getSelectedPath();

    // Coordinates functions
    QRectF getColorCoordinates( quint8 );
    quint8 getColorIndexFromCoordinates( QPointF );

    // Mouse event filter
    bool eventFilter( QObject*, QEvent* );

    // Display functions
    void displayColors();
    void displaySelection();
    void temporarilyDisplayAllColors();
    void displayInfo( QString );
    void clearInfo();
    void displayBorder();
    void clearBorder();

    void refreshPathComboBox();
    void refreshColorLineEdit();
    void refreshIndexLineEdit();
    void refreshTranslationIndexLineEdit();

    void refresh();

signals:
    void pathSelected( QString );
    void colorSelected( quint8 );
    void displayAllRootColors();
    void displayRootInformation( QString );
    void clearRootInformation();
    void displayRootBorder();
    void clearRootBorder();
    void modified();
    void refreshed();

private slots:
    void pathComboBox_currentTextChanged( const QString &arg1 );
    void displayComboBox_currentTextChanged( const QString &arg1 );
    void on_colorLineEdit_returnPressed();
    void on_colorPickPushButton_clicked();
    void on_translationIndexLineEdit_returnPressed();
    void on_indexResetPushButton_clicked();
    void on_indexPickPushButton_clicked();

private:
    Ui::PaletteWidget *ui;
    bool isLevelCel;
    bool isTrn;

    CelView *celView;
    LevelCelView *levelCelView;

    QGraphicsScene *scene;

    quint8 selectedColorIndex;
    QColor selectedColor;
    quint8 selectedTranslationIndex;

    bool pickingTranslationColor;
    bool temporarilyDisplayingAllColors;

    D1Pal *pal;
    D1Trn *trn;

    D1PalHits *palHits;

    bool buildingPathComboBox;
    QMap<QString,QString> paths;

    bool buildingDisplayComboBox;
};

#endif // PALETTEWIDGET_H
