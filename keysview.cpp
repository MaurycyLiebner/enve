#include "keysview.h"
#include "qrealkey.h"
#include <QPainter>
#include "mainwindow.h"
#include "boxeslist.h"

KeysView::KeysView(BoxesList *boxesList, QWidget *parent) : QWidget(parent)
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

void KeysView::ifIsCurrentAnimatorSetNull(QrealAnimator *animator) {
    if(mAnimator == animator) {
        graphSetAnimator(NULL);
    }
}

void KeysView::middlePress(QPointF pressPos)
{
    mSavedMinViewedFrame = mMinViewedFrame;
    mSavedMaxViewedFrame = mMaxViewedFrame;
    mMiddlePressPos = pressPos;
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
    mCanvas->getKeysInRect(mSelectionRect.translated(0., mBoxesList->getViewedTop() ),
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
    graphWheelEvent(e);

    repaint();
}

void KeysView::mousePressEvent(QMouseEvent *e) {
    if(mAnimator == NULL) {
        if(e->button() == Qt::MiddleButton) {
            middlePress(e->pos() );
        } else {
            mFirstMove = true;
            mLastPressPos = e->pos();

            mLastPressedKey = mCanvas->getKeyAtPos(e->x(),
                                                   e->y() + mBoxesList->getViewedTop(),
                                                   0.);
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
    } else {
        graphMousePressEvent(e->pos(),
                             e->button());
    }

    mMainWindow->callUpdateSchedulers();
}

bool KeysView::processFilteredKeyEvent(QKeyEvent *event) {
    if(!hasFocus() ) return false;
    if(mGraphViewed) {
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
    } else {
        return graphProcessFilteredKeyEvent(event);
    }
}

void KeysView::paintEvent(QPaintEvent *) {
    QPainter p(this);

    p.fillRect(rect(), QColor(200, 155, 155));

    if(!mGraphViewed) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(165, 125, 125));
        for(int i = LIST_ITEM_HEIGHT; i < height(); i += 2*LIST_ITEM_HEIGHT) {
            p.drawRect(0, i, width(), LIST_ITEM_HEIGHT );
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
    if(mAnimator != NULL) {
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
        mCanvas->drawChildrenKeysView(&p,
                                      -mBoxesList->getViewedTop(),
                                      mBoxesList->getViewedBottom(),
                                      mPixelsPerFrame,
                                      mMinViewedFrame, mMaxViewedFrame);
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
    if(mAnimator == NULL) {
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
    } else {
        graphMouseMoveEvent(event->pos(),
                            event->buttons());
    }


    mMainWindow->callUpdateSchedulers();
}

void KeysView::mouseReleaseEvent(QMouseEvent *e)
{
    if(mAnimator == NULL) {
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
    } else {
        graphMouseReleaseEvent(e->button());
    }


    mMainWindow->callUpdateSchedulers();
}

void KeysView::setFramesRange(int startFrame, int endFrame)
{
    mMinViewedFrame = startFrame;
    mMaxViewedFrame = endFrame;
    updatePixelsPerFrame();
    if(mAnimator != NULL) {
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
