#ifndef PALETTEWIDGET_H
#define PALETTEWIDGET_H

#include <QWidget>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QFileInfo>
#include <QDirIterator>

#include "celview.h"
#include "levelcelview.h"
#include "d1pal.h"
#include "d1trn.h"

#define PALETTE_DEFAULT_WIDTH 224

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

    D1Pal *pal;
    D1Trn *trn;

    QMap<QString,QString> paths;


};

#endif // PALETTEWIDGET_H
