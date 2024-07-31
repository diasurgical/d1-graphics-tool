#include "view.h"
#include "mainwindow.h"

View::View(QWidget *parent)
    : QGraphicsView(parent)
{
    this->setMouseTracking(true);
}

void View::mousePressEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton: {
        // left out for left mouse button events
        QGraphicsView::mousePressEvent(event);
        break;
    }
    case Qt::MiddleButton: {
        this->setDragMode(QGraphicsView::ScrollHandDrag);

        // after middle button has been pressed - send the mouse press event to base
        // class that holds this scene, since it will toggle on dragging on ScrollHandDrag
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QMouseEvent *pressEvent = new QMouseEvent(QEvent::MouseButtonPress,
            event->pos(), event->globalPosition().toPoint(), Qt::MouseButton::LeftButton,
            Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
#else
        QMouseEvent *pressEvent = new QMouseEvent(QEvent::MouseButtonPress,
            event->pos(), event->globalPos(), Qt::MouseButton::LeftButton,
            Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
#endif

        QGraphicsView::mousePressEvent(pressEvent);
        break;
    }
    default: {
        QGraphicsView::mousePressEvent(event);
        break;
    }
    }
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton: {
        break;
    }
    case Qt::MiddleButton: {
        this->setDragMode(QGraphicsView::NoDrag);
        break;
    }
    }
}

void View::leaveEvent(QEvent *event)
{
    dynamic_cast<MainWindow *>(this->window())->updateStatusBar("", "color: rgb(0, 0, 0);");
}
