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

BoxesList::BoxesList(MainWindow *mainWindow, QWidget *parent) : QWidget(parent)
{
    VISIBLE_PIXMAP = new QPixmap("pixmaps/icons/ink_visible.png");
    INVISIBLE_PIXMAP = new QPixmap("pixmaps/icons/ink_hidden.png");
    HIDE_CHILDREN = new QPixmap("pixmaps/icons/list_hide_children.png");
    SHOW_CHILDREN = new QPixmap("pixmaps/icons/list_show_children.png");
    LOCKED_PIXMAP = new QPixmap("pixmaps/icons/ink_lock_locked.png");
    UNLOCKED_PIXMAP = new QPixmap("pixmaps/icons/ink_lock_unlocked.png");

    mCanvas = mainWindow->getCanvas();
    mMainWindow = mainWindow;
    setMinimumHeight(200);
}

void BoxesList::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(155, 155, 155));

    p.setPen(QPen(QColor(75, 75, 75), 1.));
    qreal xT = 200 + mPixelsPerFrame*0.5 - mMinViewedFrame*mPixelsPerFrame;
    int iInc = 1;
    bool mult5 = true;
    while(iInc*mPixelsPerFrame < 10.) {
        if(mult5) {
            iInc *= 5;
        } else {
            iInc *= 2;
        }
    }
    for(int i = mMainWindow->getMinFrame(); i <= mMainWindow->getMaxFrame(); i += iInc) {
        qreal xTT = xT + i*mPixelsPerFrame;
        p.drawLine(QPointF(xTT, 0.), QPointF(xTT, mViewedRect.height()) );
    }

    xT = (mMainWindow->getCurrentFrame() - mMinViewedFrame)*mPixelsPerFrame +
            200 + mPixelsPerFrame*0.5;
    p.setPen(QPen(Qt::green, 2.));
    p.drawLine(QPointF(xT, 0.), QPointF(xT, mViewedRect.height()) );

    p.setPen(QPen(Qt::black, 1.));

    mCanvas->drawChildren(&p, 0.f, 0.f, mViewedRect.top(), mViewedRect.bottom(),
                          mPixelsPerFrame, mMinViewedFrame, mMaxViewedFrame);

    if(mSelectingKeys) {
        p.setPen(QPen(Qt::blue, 2., Qt::DotLine));
        p.setBrush(Qt::NoBrush);
        p.drawRect(mSelectionRect);
    }

    p.end();
}

void BoxesList::resizeEvent(QResizeEvent *e)
{
    mViewedRect.setSize(e->size());
    updatePixelsPerFrame();
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
    mFirstMove = true;
    mLastPressPos = event->pos();
    if(event->x() < 200) {
        mCanvas->handleChildListItemMousePress(event->x(),
                                               event->y() + mViewedRect.top(),
                                               0.f);
    } else {
        mLastPressedKey = mCanvas->getKeyAtPos(event->x() - 200.,
                                                event->y() + mViewedRect.top(),
                                                0.);
        if(mLastPressedKey == NULL) {
            mSelectingKeys = true;
            mSelectionRect.setTopLeft(event->pos() );
            mSelectionRect.setBottomRight(event->pos() );
        }
        else {
            if(!isShiftPressed() && !mLastPressedKey->isSelected()) {
                clearKeySelection();
            }
            addKeyToSelection(mLastPressedKey);
            mMovingKeys = true;
        }
    }

    scheduleRepaint();
    mMainWindow->callUpdateSchedulers();
}

void BoxesList::mouseMoveEvent(QMouseEvent *event)
{
    mFirstMove = false;
    if(mMovingKeys) {
        int dFrame = qRound( (event->x() - mLastPressPos.x())/mPixelsPerFrame );
        int dDFrame = dFrame - mMoveDFrame;
        if(dDFrame == 0) return;
        mMoveDFrame = dFrame;
        foreach(QrealKey *key, mSelectedKeys) {
            key->incFrameAndUpdateParentAnimator(dDFrame);
        }
    } else if(mSelectingKeys) {
        mSelectionRect.setBottomRight(event->pos() );
    }

    scheduleRepaint();
    mMainWindow->callUpdateSchedulers();
}

void BoxesList::selectKeysInSelectionRect() {
    QList<QrealKey*> listKeys;
    mCanvas->getKeysInRect(mSelectionRect.translated(-200., mViewedRect.top() ),
                           0., &listKeys);
    foreach(QrealKey *key, listKeys) {
        addKeyToSelection(key);
    }
}

void BoxesList::mouseReleaseEvent(QMouseEvent *)
{
    if(mSelectingKeys) {
        if(mFirstMove) {
            if(!isShiftPressed() ) {
                clearKeySelection();
            }
        } else {
            if(mSelectionRect.left() > mSelectionRect.right()) {
                qreal rightT = mSelectionRect.right();
                qreal leftT = mSelectionRect.left();
                mSelectionRect.setLeft(rightT);
                mSelectionRect.setRight(leftT);
            }
            if(mSelectionRect.top() > mSelectionRect.bottom()) {
                qreal bottomT = mSelectionRect.bottom();
                qreal topT = mSelectionRect.top();
                mSelectionRect.setTop(bottomT);
                mSelectionRect.setBottom(topT);
            }
            if(!isShiftPressed()) {
                clearKeySelection();
            }
            selectKeysInSelectionRect();
        }
        mSelectingKeys = false;
    } else if(mMovingKeys) {
        if(mFirstMove) {
            if(isShiftPressed()) {
                if(mLastPressedKey->isSelected() ) {
                    removeKeyFromSelection(mLastPressedKey);
                } else {
                    addKeyToSelection(mLastPressedKey);
                }
            } else {
                clearKeySelection();
                addKeyToSelection(mLastPressedKey);
            }
        }
        mMoveDFrame = 0;
        mMovingKeys = false;
    }

    scheduleRepaint();
    mMainWindow->callUpdateSchedulers();
}

void BoxesList::setFramesRange(int startFrame, int endFrame)
{
    mMinViewedFrame = startFrame;
    mMaxViewedFrame = endFrame;
    updatePixelsPerFrame();
    repaint();
}

void BoxesList::repaintIfNeeded() {
    if(mRepaintScheduled) {
        repaint();
        mRepaintScheduled = false;
    }
}

int BoxesList::getMinViewedFrame()
{
    return mMinViewedFrame;
}

int BoxesList::getMaxViewedFrame()
{
    return mMaxViewedFrame;
}

qreal BoxesList::getPixelsPerFrame()
{
    return mPixelsPerFrame;
}

void BoxesList::updatePixelsPerFrame()
{
    qreal animWidth = mViewedRect.width() - 200;
    qreal dFrame = mMaxViewedFrame - mMinViewedFrame + 1;
    mPixelsPerFrame = animWidth/dFrame;
}

void BoxesList::addKeyToSelection(QrealKey *key)
{
    if(key->isSelected()) return;
    key->setSelected(true);
    mSelectedKeys << key;
}

void BoxesList::removeKeyFromSelection(QrealKey *key)
{
    if(key->isSelected()) {
        key->setSelected(false);
        mSelectedKeys.removeOne(key);
    }
}

void BoxesList::clearKeySelection()
{
    foreach(QrealKey *key, mSelectedKeys) {
        key->setSelected(false);
    }
    mSelectedKeys.clear();
}

void BoxesList::scheduleRepaint() {
    if(mRepaintScheduled) return;
    mRepaintScheduled = true;
}
