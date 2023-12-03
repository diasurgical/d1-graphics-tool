#pragma once

#include <QGraphicsView>
#include <QMouseEvent>
#include <QObject>

class View : public QGraphicsView {
    Q_OBJECT

public:
    View(QWidget *parent = nullptr);

private slots:
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
};
