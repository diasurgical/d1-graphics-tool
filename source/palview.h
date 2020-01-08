#ifndef PALVIEW_H
#define PALVIEW_H

#include <QWidget>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QFileInfo>
#include <QDirIterator>

#include "celview.h"
#include "levelcelview.h"
#include "d1pal.h"
#include "d1trn.h"

#define PALETTE_DEFAULT_WIDTH 256

namespace Ui {
class PalView;
}

class PalView : public QWidget
{
    Q_OBJECT

public:
    explicit PalView(QWidget *parent = 0);
    ~PalView();

    void initialize( D1Pal*, D1Trn*, D1Trn*, CelView* );
    void initialize( D1Pal*, D1Trn*, D1Trn*, LevelCelView* );

    void refreshPalettesPathsAndNames();
    void refreshTranslationsPathsAndNames();
    void displayPal();
    void displayTrn();

private slots:
    void on_palComboBox_currentIndexChanged(const QString &arg1);
    void on_trn1ComboBox_currentIndexChanged(const QString &arg1);
    void on_trn2ComboBox_currentIndexChanged(const QString &arg1);

private:
    Ui::PalView *ui;
    bool isCelLevel;
    CelView *celView;
    LevelCelView *levelCelView;
    QGraphicsScene *palScene;
    QGraphicsScene *trn1Scene;
    QGraphicsScene *trn2Scene;

    D1Pal *pal;
    D1Trn *trn1;
    D1Trn *trn2;

    bool buildingPalComboBox;
    bool buildingTrnComboBox;
    QMap<QString,QString> palettesPaths;
    QMap<QString,QString> translationsPaths;
};

#endif // PALVIEW_H
