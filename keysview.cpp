#include "keysview.h"
#include "qrealkey.h"
#include <QPainter>
#include "mainwindow.h"
#include "boxeslist.h"

KeysView::KeysView(BoxesListWidget *boxesList, QWidget *parent) : QWidget(parent)
{
    mMainWindow = MainWindow::getInstance();
    mCanvas = mMainWindow->getCanvas();
    mBoxesList = boxesList;

    setFocusPolicy(Qt::StrongFocus);
}

void KeysView::setAnimationDockWidget(AnimationDockWidget *graphControls)
{
    mGraphControls = graphControls;
}

void KeysView::setGraphViewed(bool bT) {
    mGraphViewed = bT;
    mGraphControls->setVisible(bT);
}

void KeysView::middlePress(QPointF pressPos)
{
    mSavedMinViewedFrame = mMinViewedFrame;
    mSavedMaxViewedFrame = mMaxViewedFrame;
    mMiddlePressPos = pressPos;
}

void KeysView::setViewedRange(int top, int bottom)
{
    mViewedTop = top;
    mViewedBottom = bottom;
    update();
}

void KeysView::middleMove(QPointF movePos)
{
    qreal diffFrame = (movePos.x() - mMiddlePressPos.x() ) / mPixelsPerFrame;
    int roundX = qRound(diffFrame );
    setFramesRange(mSavedMinViewedFrame - roundX,
                   mSavedMaxViewedFrame - roundX );
}

void KeysView::deleteSelectedKeys()
{
    foreach(QrealKey *key, mSelectedKeys) {
        key->deleteKey();
        key->decNumberPointers();
    }
    mSelectedKeys.clear();
}

void KeysView::selectKeysInSelectionRect() {
    QList<QrealKey*> listKeys;
    mBoxesList->getKeysInRect(mSelectionRect, mViewedTop,
                              mPixelsPerFrame, mMinViewedFrame,
                              &listKeys);
    foreach(QrealKey *key, listKeys) {
        addKeyToSelection(key);
    }
}

void KeysView::resizeEvent(QResizeEvent *e) {
    updatePixelsPerFrame();

    if(mGraphViewed) {
        graphResizeEvent(e);
    }
}

void KeysView::wheelEvent(QWheelEvent *e) {
    if(mGraphViewed) {
        graphWheelEvent(e);
    } else {
        emit wheelEventSignal(e);
        //mBoxesList->handleWheelEvent(e);
    }
}

void KeysView::mousePressEvent(QMouseEvent *e) {
    if(mGraphViewed) {
        graphMousePressEvent(e->pos(),
                             e->button());
    } else {
        if(e->button() == Qt::MiddleButton) {
            middlePress(e->pos() );
        } else {
            mFirstMove = true;
            mLastPressPos = e->pos();

            mLastPressedKey = mBoxesList->getKeyAtPos(e->x(), e->y(),
                                                      mPixelsPerFrame,
                                                      mViewedTop, mMinViewedFrame);
            if(mLastPressedKey == NULL) {
                mSelecting = true;
                mSelectionRect.setTopLeft(e->pos() );
                mSelectionRect.setBottomRight(e->pos() );
            }
            else {
                if(!mMainWindow->isShiftPressed() &&
                        !mLastPressedKey->isSelected()) {
                    clearKeySelection();
                }
                if(mMainWindow->isShiftPressed() &&
                        mLastPressedKey->isSelected()) {
                    removeKeyFromSelection(mLastPressedKey);
                } else {
                    addKeyToSelection(mLastPressedKey);

                    mMovingKeys = true;
                }
            }
        }
    }

    mMainWindow->callUpdateSchedulers();
}

bool KeysView::processFilteredKeyEvent(QKeyEvent *event) {
    if(!hasFocus() ) return false;
    if(mGraphViewed) {
        return graphProcessFilteredKeyEvent(event);
    } else {
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
}

void KeysView::paintEvent(QPaintEvent *) {
    QPainter p(this);

    p.fillRect(rect(), QColor(200, 155, 155));

    if(!mGraphViewed) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(165, 125, 125));
        for(int i = BoxesList::getListItemHeight(); i < height(); i += 2*BoxesList::getListItemHeight()) {
            p.drawRect(0, i, width(), BoxesList::getListItemHeight() );
        }
    }

    p.setPen(QPen(QColor(75, 75, 75), 1.));
    qreal xT = mPixelsPerFrame*0.5;
    int iInc = 1;
    bool mult5 = true;
    while(iInc*mPixelsPerFrame < 10.) {
        if(mult5) {
            iInc *= 5;
        } else {
            iInc *= 2;
        }
    }
    int minFrame = mMainWindow->getMinFrame();
    if(mGraphViewed) {
        while(xT + minFrame*mPixelsPerFrame < 38.) {
            minFrame++;
        }
    }
    for(int i = minFrame; i <= mMainWindow->getMaxFrame(); i += iInc) {
        qreal xTT = xT + i*mPixelsPerFrame;
        p.drawLine(QPointF(xTT, 0.), QPointF(xTT, height()) );
    }



    if(mMainWindow->getCurrentFrame() <= mMaxViewedFrame &&
            mMainWindow->getCurrentFrame() >= mMinViewedFrame) {
        xT = (mMainWindow->getCurrentFrame() - mMinViewedFrame)*mPixelsPerFrame +
                mPixelsPerFrame*0.5;
        p.setPen(QPen(Qt::green, 2.));
        p.drawLine(QPointF(xT, 0.), QPointF(xT, height()) );
    }

    p.setPen(QPen(Qt::black, 1.));

    if(mGraphViewed) {
        graphPaint(&p);
    } else {
        mBoxesList->drawKeys(&p, mPixelsPerFrame, mViewedTop,
                             mMinViewedFrame, mMaxViewedFrame);
//        mCanvas->drawKeysView(&p,
//                              -mViewedTop,
//                              mViewedBottom,
//                              mPixelsPerFrame,
//                              mMinViewedFrame, mMaxViewedFrame);
    }

    if(mSelecting) {
        p.setPen(QPen(Qt::blue, 2., Qt::DotLine));
        p.setBrush(Qt::NoBrush);
        p.drawRect(mSelectionRect);
    }

    if(hasFocus() ) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(Qt::red, 4.));
        p.drawRect(0, 0,
                   width(), height());
    }

    p.end();
}

void KeysView::mouseMoveEvent(QMouseEvent *event)
{
    if(mGraphViewed) {
        graphMouseMoveEvent(event->pos(),
                            event->buttons());
    } else {
        if(event->buttons() == Qt::MiddleButton) {
            middleMove(event->pos() );
            emit changedViewedFrames(mMinViewedFrame,
                                     mMaxViewedFrame);
        } else {
            mFirstMove = false;
            if(mMovingKeys) {
                int dFrame = qRound( (event->x() - mLastPressPos.x())/mPixelsPerFrame );
                int dDFrame = dFrame - mMoveDFrame;
                if(dDFrame == 0) return;
                mMoveDFrame = dFrame;
                foreach(QrealKey *key, mSelectedKeys) {
                    key->incFrameAndUpdateParentAnimator(dDFrame);
                }
            } else if(mSelecting) {
                mSelectionRect.setBottomRight(event->pos() );
            }
        }
    }


    mMainWindow->callUpdateSchedulers();
}

void KeysView::mouseReleaseEvent(QMouseEvent *e)
{
    if(mGraphViewed) {
        graphMouseReleaseEvent(e->button());
    } else {
        if(mSelecting) {
            if(mFirstMove) {
                if(!mMainWindow->isShiftPressed()) {
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
                if(!mMainWindow->isShiftPressed()) {
                    clearKeySelection();
                }
                selectKeysInSelectionRect();
            }
            mSelecting = false;
        } else if(mMovingKeys) {
            if(mFirstMove) {
                if(mMainWindow->isShiftPressed()) {
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
    }


    mMainWindow->callUpdateSchedulers();
}

void KeysView::setFramesRange(int startFrame, int endFrame)
{
    mMinViewedFrame = startFrame;
    mMaxViewedFrame = endFrame;
    updatePixelsPerFrame();
    if(mGraphViewed) {
        graphUpdateDimensions();
        graphUpdateDrawPathIfNeeded();
    }
    repaint();
}

int KeysView::getMinViewedFrame()
{
    return mMinViewedFrame;
}

int KeysView::getMaxViewedFrame()
{
    return mMaxViewedFrame;
}

qreal KeysView::getPixelsPerFrame()
{
    return mPixelsPerFrame;
}

void KeysView::updatePixelsPerFrame()
{
    qreal animWidth = width();
    qreal dFrame = mMaxViewedFrame - mMinViewedFrame + 1;
    mPixelsPerFrame = animWidth/dFrame;
}

void KeysView::addKeyToSelection(QrealKey *key)
{
    if(key->isSelected()) return;
    key->setSelected(true);
    mSelectedKeys << key;
    key->incNumberPointers();
}

void KeysView::removeKeyFromSelection(QrealKey *key)
{
    if(key->isSelected()) {
        key->setSelected(false);
        if(mSelectedKeys.removeOne(key) ) {
            key->decNumberPointers();
        }
    }
}

void KeysView::clearKeySelection()
{
    foreach(QrealKey *key, mSelectedKeys) {
        key->setSelected(false);
    }
    mSelectedKeys.clear();
}
