#include "boxeslist.h"
#include "canvas.h"
#include <QPainter>
#include <QResizeEvent>
#include "mainwindow.h"

QPixmap *BoxesList::VISIBLE_PIXMAP;
QPixmap *BoxesList::INVISIBLE_PIXMAP;
QPixmap *BoxesList::HIDE_CHILDREN;
QPixmap *BoxesList::SHOW_CHILDREN;
QPixmap *BoxesList::LOCKED_PIXMAP;
QPixmap *BoxesList::UNLOCKED_PIXMAP;

BoxesList::BoxesList(Canvas *canvas, MainWindow *parent) : QWidget(parent)
{
    VISIBLE_PIXMAP = new QPixmap("pixmaps/icons/ink_visible.png");
    INVISIBLE_PIXMAP = new QPixmap("pixmaps/icons/ink_hidden.png");
    HIDE_CHILDREN = new QPixmap("pixmaps/icons/list_hide_children.png");
    SHOW_CHILDREN = new QPixmap("pixmaps/icons/list_show_children.png");
    LOCKED_PIXMAP = new QPixmap("pixmaps/icons/ink_lock_locked.png");
    UNLOCKED_PIXMAP = new QPixmap("pixmaps/icons/ink_lock_unlocked.png");

    mCanvas = canvas;
    mMainWindow = parent;
    setMinimumHeight(200);
}

void BoxesList::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(155, 155, 155));

    mCanvas->drawChildren(&p, 0.f, 0.f, mViewedRect.top(), mViewedRect.bottom());

    p.end();
}

void BoxesList::resizeEvent(QResizeEvent *e)
{
    mViewedRect.setSize(e->size());
}

void BoxesList::wheelEvent(QWheelEvent *event)
{
    if(event->delta() > 0) {
        mViewedRect.translate(0, -LIST_ITEM_HEIGHT);
        if(mViewedRect.top() < 0) {
            mViewedRect.translate(0, -mViewedRect.top());
        }
    } else {
        mViewedRect.translate(0, LIST_ITEM_HEIGHT);
    }
    repaint();
}

void BoxesList::mousePressEvent(QMouseEvent *event)
{
    mCanvas->handleChildListItemMousePress(event->x(), event->y(), 0.f);
    mMainWindow->callUpdateSchedulers();
}

void BoxesList::repaintIfNeeded() {
    if(mRepaintScheduled) {
        repaint();
        mRepaintScheduled = false;
    }
}

void BoxesList::scheduleRepaint() {
    if(mRepaintScheduled) return;
    mRepaintScheduled = true;
}
