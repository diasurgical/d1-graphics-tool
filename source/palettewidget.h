#ifndef PALETTEWIDGET_H
#define PALETTEWIDGET_H

#include <QWidget>

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
