#include "boxeslist.h"
#include "canvas.h"
#include <QPainter>
#include <QResizeEvent>
#include "mainwindow.h"
#include "qrealkey.h"

QPixmap *BoxesList::VISIBLE_PIXMAP;
QPixmap *BoxesList::INVISIBLE_PIXMAP;
QPixmap *BoxesList::HIDE_CHILDREN;
QPixmap *BoxesList::SHOW_CHILDREN;
QPixmap *BoxesList::LOCKED_PIXMAP;
QPixmap *BoxesList::UNLOCKED_PIXMAP;
QPixmap *BoxesList::ANIMATOR_CHILDREN_VISIBLE;
QPixmap *BoxesList::ANIMATOR_CHILDREN_HIDDEN;
QPixmap *BoxesList::ANIMATOR_RECORDING;
QPixmap *BoxesList::ANIMATOR_NOT_RECORDING;

BoxesList::BoxesList(QWidget *parent) : QWidget(parent)
{
    VISIBLE_PIXMAP = new QPixmap("pixmaps/icons/ink_visible.png");
    INVISIBLE_PIXMAP = new QPixmap("pixmaps/icons/ink_hidden.png");
    HIDE_CHILDREN = new QPixmap("pixmaps/icons/list_hide_children.png");
    SHOW_CHILDREN = new QPixmap("pixmaps/icons/list_show_children.png");
    LOCKED_PIXMAP = new QPixmap("pixmaps/icons/ink_lock_locked.png");
    UNLOCKED_PIXMAP = new QPixmap("pixmaps/icons/ink_lock_unlocked.png");
    ANIMATOR_CHILDREN_VISIBLE = new QPixmap(
                "pixmaps/icons/animator_children_visible.png");
    ANIMATOR_CHILDREN_HIDDEN = new QPixmap(
                "pixmaps/icons/animator_children_hidden.png");
    ANIMATOR_RECORDING = new QPixmap(
                "pixmaps/icons/recording.png");
    ANIMATOR_NOT_RECORDING = new QPixmap(
                "pixmaps/icons/not_recording.png");

    mMainWindow = MainWindow::getInstance();
    mCanvas = mMainWindow->getCanvas();

    setFixedWidth(LIST_ITEM_MAX_WIDTH);
    setFocusPolicy(Qt::StrongFocus);
}

void BoxesList::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(155, 155, 155));

    p.setBrush(QColor(125, 125, 125));
    p.setPen(Qt::NoPen);
    for(int i = LIST_ITEM_HEIGHT; i < height(); i += 2*LIST_ITEM_HEIGHT) {
        p.drawRect(0, i, width(), LIST_ITEM_HEIGHT );
    }

    p.setPen(QPen(Qt::black, 1.));

    mCanvas->drawChildrenListItems(&p, 0., -mViewedTop, mViewedBottom);

    p.setPen(QPen(Qt::black, 1.) );
    p.drawLine(LIST_ITEM_MAX_WIDTH - 1., 0, LIST_ITEM_MAX_WIDTH - 1., height());

    if(hasFocus() ) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(Qt::red, 4.));
        p.drawRect(0, 0,
                   LIST_ITEM_MAX_WIDTH, height());
    }

    p.end();
}

bool BoxesList::processFilteredKeyEvent(QKeyEvent *event) {
    if(!hasFocus() ) return false;

    return false;
}


void BoxesList::resizeEvent(QResizeEvent *e)
{
    mViewedBottom = mViewedTop + e->size().width();
}

void BoxesList::translateViewed(qreal translateBy) {
    mViewedBottom += translateBy;
    mViewedTop += translateBy;
}

void BoxesList::handleWheelEvent(QWheelEvent *event) {
    if(event->delta() > 0) {
        translateViewed(-LIST_ITEM_HEIGHT);
        if(mViewedTop < 0) {
            translateViewed(-mViewedTop);
        }
    } else {
        translateViewed(LIST_ITEM_HEIGHT);
    }
    repaint();
    mMainWindow->getKeysView()->repaint();
}

void BoxesList::wheelEvent(QWheelEvent *event)
{
    handleWheelEvent(event);
}

void BoxesList::mousePressEvent(QMouseEvent *event)
{
    mCanvas->handleChildListItemMousePress(event->x(),
                                           event->x(),
                                           event->y() + mViewedTop,
                                           0.f, event);

    mMainWindow->callUpdateSchedulers();
}

qreal BoxesList::getViewedTop() {
    return mViewedTop;
}

qreal BoxesList::getViewedBottom() {
    return mViewedBottom;
}
