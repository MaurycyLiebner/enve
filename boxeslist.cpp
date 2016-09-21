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

BoxesList::BoxesList(MainWindow *mainWindow, QWidget *parent) : QWidget(parent)
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

    mCanvas = mainWindow->getCanvas();
    mMainWindow = mainWindow;
    setMinimumHeight(200);
    setFocusPolicy(Qt::StrongFocus);
}

void BoxesList::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(155, 155, 155));

    p.setPen(QPen(QColor(75, 75, 75), 1.));
    qreal xT = LIST_ITEM_MAX_WIDTH + mPixelsPerFrame*0.5;
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

    p.setPen(QPen(QColor(125, 125, 125), 1.));
    for(int i = LIST_ITEM_HEIGHT; i < height(); i += LIST_ITEM_HEIGHT) {
        p.drawLine(QPointF(0, i), QPointF(width(), i) );
    }

    if(mMainWindow->getCurrentFrame() <= mMaxViewedFrame &&
            mMainWindow->getCurrentFrame() >= mMinViewedFrame) {
        xT = (mMainWindow->getCurrentFrame() - mMinViewedFrame)*mPixelsPerFrame +
                LIST_ITEM_MAX_WIDTH + mPixelsPerFrame*0.5;
        p.setPen(QPen(Qt::green, 2.));
        p.drawLine(QPointF(xT, 0.), QPointF(xT, mViewedRect.height()) );
    }

    p.setPen(QPen(Qt::black, 1.));

    mCanvas->drawChildrenListItems(&p,
                                   0., -mViewedRect.top(),
                                   mViewedRect.bottom(),
                                   mPixelsPerFrame,
                                   mMinViewedFrame, mMaxViewedFrame);

    if(mSelectingKeys) {
        p.setPen(QPen(Qt::blue, 2., Qt::DotLine));
        p.setBrush(Qt::NoBrush);
        p.drawRect(mSelectionRect);
    }

    if(hasFocus() ) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(Qt::red, 4.));
        p.drawRect(rect() );
    }
    p.setPen(QPen(Qt::black, 1.) );
    p.drawLine(LIST_ITEM_MAX_WIDTH, 0, LIST_ITEM_MAX_WIDTH, height());
    p.end();
}

void BoxesList::deleteSelectedKeys()
{
    foreach(QrealKey *key, mSelectedKeys) {
        key->deleteKey();
        key->decNumberPointers();
    }
    mSelectedKeys.clear();
}

bool BoxesList::processFilteredKeyEvent(QKeyEvent *event) {
    if(!hasFocus() ) return false;
    if(event->key() == Qt::Key_Delete) {
        deleteSelectedKeys();
        repaint();
    } else if(event->key() == Qt::Key_Right) {
        foreach(QrealKey *key, mSelectedKeys) {
            key->incFrameAndUpdateParentAnimator(1);
        }
    } else if(event->key() == Qt::Key_Left) {
        foreach(QrealKey *key, mSelectedKeys) {
            key->incFrameAndUpdateParentAnimator(-1);
        }
    } else {
        return false;
    }
    return true;
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
    if(event->x() < LIST_ITEM_MAX_WIDTH) {
        mCanvas->handleChildListItemMousePress(event->x(),
                                               event->y() + mViewedRect.top(),
                                               0.f);
    } else {
        mLastPressedKey = mCanvas->getKeyAtPos(event->x() - LIST_ITEM_MAX_WIDTH,
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
            if(isShiftPressed() && mLastPressedKey->isSelected()) {
                removeKeyFromSelection(mLastPressedKey);
            } else {
                addKeyToSelection(mLastPressedKey);

                mMovingKeys = true;
            }
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
    mCanvas->getKeysInRect(mSelectionRect.translated(-LIST_ITEM_MAX_WIDTH, mViewedRect.top() ),
                           &listKeys);
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
        QList<QrealAnimator*> parentAnimators;
        foreach(QrealKey *key, mSelectedKeys) {
            if(parentAnimators.contains(key->getParentAnimator()) ) continue;
            parentAnimators << key->getParentAnimator();
        }
        foreach(QrealAnimator *animator, parentAnimators) {
            animator->mergeKeysIfNeeded();
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
    qreal animWidth = mViewedRect.width() - LIST_ITEM_MAX_WIDTH;
    qreal dFrame = mMaxViewedFrame - mMinViewedFrame + 1;
    mPixelsPerFrame = animWidth/dFrame;
}

void BoxesList::addKeyToSelection(QrealKey *key)
{
    if(key->isSelected()) return;
    key->setSelected(true);
    mSelectedKeys << key;
    key->incNumberPointers();
}

void BoxesList::removeKeyFromSelection(QrealKey *key)
{
    if(key->isSelected()) {
        key->setSelected(false);
        if(mSelectedKeys.removeOne(key) ) {
            key->decNumberPointers();
        }
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
