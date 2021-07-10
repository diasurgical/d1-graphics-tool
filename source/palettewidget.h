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

namespace Ui {
class PaletteWidget;
}

class PaletteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PaletteWidget(QWidget *parent = nullptr);
    ~PaletteWidget();

private:
    Ui::PaletteWidget *ui;
};

#endif // PALETTEWIDGET_H
