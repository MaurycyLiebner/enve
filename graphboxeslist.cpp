#include <QPainter>
#include <QMouseEvent>
#include "mainwindow.h"
#include "qrealpointvaluedialog.h"
#include "keysview.h"
#include "updatescheduler.h"
#include "qrealpoint.h"

void KeysView::graphSetSmoothCtrl() {
    graphSetTwoSideCtrlForSelected();
    graphSetCtrlsModeForSelected(CTRLS_SMOOTH);
    graphRepaint();
}

void KeysView::graphSetSymmetricCtrl() {
    graphSetTwoSideCtrlForSelected();
    graphSetCtrlsModeForSelected(CTRLS_SYMMETRIC);
    graphRepaint();
}

void KeysView::graphSetCornerCtrl() {
    graphSetTwoSideCtrlForSelected();
    graphSetCtrlsModeForSelected(CTRLS_CORNER);
    graphRepaint();
}

void KeysView::graphPaint(QPainter *p) {
    p->setBrush(Qt::NoBrush);

    /*qreal maxX = width();
    int currAlpha = 75;
    qreal lineWidth = 1.;
    QList<int> incFrameList = { 1, 5, 10, 100 };
    foreach(int incFrame, incFrameList) {
        if(mPixelsPerFrame*incFrame < 15.) continue;
        bool drawText = mPixelsPerFrame*incFrame > 30.;
        p->setPen(QPen(QColor(0, 0, 0, currAlpha), lineWidth));
        int frameL = (mStartFrame >= 0) ? -(mStartFrame%incFrame) :
                                        -mStartFrame;
        int currFrame = mStartFrame + frameL;
        qreal xL = frameL*mPixelsPerFrame;
        qreal inc = incFrame*mPixelsPerFrame;
        while(xL < 40) {
            xL += inc;
            currFrame += incFrame;
        }
        while(xL < maxX) {
            if(drawText) {
                p->drawText(QRectF(xL - inc, 0, 2*inc, 20),
                            Qt::AlignCenter, QString::number(currFrame));
            }
            p->drawLine(xL, 20, xL, height());
            xL += inc;
            currFrame += incFrame;
        }
        currAlpha *= 1.5;
        lineWidth *= 1.5;
    }

    p->setPen(QPen(Qt::green, 2.));
    qreal xL = (mCurrentFrame - mStartFrame)*mPixelsPerFrame;
    //p->drawText(QRectF(xL - 20, 0, 40, 20),
    //            Qt::AlignCenter, QString::number(mCurrentFrame));
    p->drawLine(xL, 20, xL, height());*/

    p->setPen(QPen(QColor(255, 255, 255), 1.));
    qreal incY = mValueInc*mPixelsPerValUnit;
    qreal yL = height() +
            fmod(mMinShownVal, mValueInc)*mPixelsPerValUnit - mMargin + incY;
    qreal currValue = mMinShownVal - fmod(mMinShownVal, mValueInc) - mValueInc;
    while(yL > 0) {
        p->drawText(QRectF(0, yL - incY, 40, 2*incY),
                    Qt::AlignCenter, QString::number(currValue));
        p->drawLine(40, yL, width(), yL);
        yL -= incY;
        currValue += mValueInc;
    }
    p->setRenderHint(QPainter::Antialiasing);

    for(int i = 0; i < mAnimators.count(); i++) {
        const QColor &col = ANIMATORCOLORS.at(i % ANIMATORCOLORS.length());
        QrealAnimator *animator = mAnimators.at(i);
        animator->drawKeysPath(p, col,
                               height(), mMargin,
                               mMinViewedFrame, mMinShownVal,
                               mPixelsPerFrame, mPixelsPerValUnit);
    }
/*

    if(hasFocus() ) {
        p->setPen(QPen(Qt::red, 4.));
    } else {
        p->setPen(Qt::NoPen);
    }
    p->setBrush(Qt::NoBrush);
    p->drawRect(mGraphRect);
*/
}

void KeysView::graphIncScale(qreal inc) {
    qreal newScale = mValueScale + inc;
    graphSetScale(clamp(newScale, 0.1, 10.));
}

void KeysView::graphSetScale(qreal scale) {
    mValueScale = scale;
    graphUpdateDimensions();
}

void KeysView::graphGetAnimatorsMinMaxValue(qreal *minVal, qreal *maxVal) {
    if(mAnimators.isEmpty()) {
        *minVal = 0.;
        *maxVal = 0.;
    } else {
        qreal minValT = 1000000;
        qreal maxValT = -1000000;

        foreach(QrealAnimator *animator, mAnimators) {
            qreal animMinVal;
            qreal animMaxVal;
            animator->qra_getMinAndMaxValues(&animMinVal, &animMaxVal);
            if(animMaxVal > maxValT) {
                maxValT = animMaxVal;
            }
            if(animMinVal < minValT) {
                minValT = animMinVal;
            }
        }

        *minVal = minValT;
        *maxVal = maxValT;
    }
}

void KeysView::graphUpdateDimensions() {
    graphGetAnimatorsMinMaxValue(&mMinVal, &mMaxVal);

    if(qAbs(mMinVal - mMaxVal) < 0.1 ) {
        mMinVal -= 0.05;
        mMaxVal += 0.05;
    }
    mPixelsPerValUnit = mValueScale*(height() - 2*mMargin)/
                                (mMaxVal - mMinVal);
    mValueInc = 10000.;
    qreal incIncValue = 1000.;
    int nIncs = 0;
    while(mValueInc*mPixelsPerValUnit > 50. ) {
        mValueInc -= incIncValue;
        nIncs++;
        if(nIncs == 9) {
            nIncs = 0;
            incIncValue *= 0.1;
        }
    }

    graphIncMinShownVal(0.);
    updatePixelsPerFrame();
}

void KeysView::graphResizeEvent(QResizeEvent *)
{
    graphUpdateDimensions();
}

void KeysView::graphIncMinShownVal(qreal inc) {
    graphSetMinShownVal(inc*(mMaxVal - mMinVal) + mMinShownVal);
}

void KeysView::graphSetMinShownVal(qreal newMinShownVal) {
    qreal halfHeightVal = (height() - 2*mMargin)*0.5/mPixelsPerValUnit;
    mMinShownVal = clamp(newMinShownVal,
                         mMinVal - halfHeightVal,
                         mMaxVal - halfHeightVal);
}

void KeysView::graphGetValueAndFrameFromPos(QPointF pos,
                                            qreal *value, qreal *frame) {
    *value = (height() - pos.y() - mMargin)/mPixelsPerValUnit
            + mMinShownVal;
    *frame = mMinViewedFrame + pos.x()/mPixelsPerFrame - 0.5;
}

void KeysView::graphMousePress(QPointF pressPos) {
    mFirstMove = true;
    qreal value;
    qreal frame;
    graphGetValueAndFrameFromPos(pressPos, &value, &frame);

    foreach(QrealAnimator *animator, mAnimators) {
        mCurrentPoint = animator->qra_getPointAt(value, frame,
                                              mPixelsPerFrame, mPixelsPerValUnit);
        if(mCurrentPoint != NULL) break;
    }
    QrealKey *parentKey = (mCurrentPoint == NULL) ?
                NULL :
                mCurrentPoint->getParentKey();
    if(mCurrentPoint == NULL) {
        if(mMainWindow->isCtrlPressed() ) {
//            graphClearKeysSelection();
//            QrealKey *newKey = new QrealKey(qRound(frame), mAnimator, value);
//            mAnimator->appendKey(newKey);
//            mAnimator->updateKeysPath();
//            mCurrentPoint = newKey->getEndPoint();
//            mAnimator->getMinAndMaxMoveFrame(newKey, mCurrentPoint,
//                                             &mMinMoveFrame, &mMaxMoveFrame);
//            mCurrentPoint->setSelected(true);
        } else {
            mSelecting = true;
            mSelectionRect.setTopLeft(pressPos);
            mSelectionRect.setBottomRight(pressPos);
        }
    } else if(mCurrentPoint->isKeyPoint()) {
        mPressFrameAndValue = QPointF(frame, value);
        if(mMainWindow->isShiftPressed()) {
            if(parentKey->isSelected()) {
                graphRemoveKeyFromSelection(parentKey);
            } else {
                graphAddKeyToSelection(parentKey);
            }
        } else {
            if(!parentKey->isSelected()) {
                graphClearKeysSelection();
                graphAddKeyToSelection(parentKey);
            }
        }
    } else {
        ((QrealAnimator*)mCurrentPoint->getParentKey()->getParentAnimator())->
                getMinAndMaxMoveFrame(
                    mCurrentPoint->getParentKey(), mCurrentPoint,
                    &mMinMoveFrame, &mMaxMoveFrame);
        mCurrentPoint->setSelected(true);
    }
}

void KeysView::graphMouseRelease()
{
    if(mSelecting) {
        if(!mMainWindow->isShiftPressed()) {
            graphClearKeysSelection();
        }
        qreal topValue;
        qreal leftFrame;
        graphGetValueAndFrameFromPos(mSelectionRect.topLeft(),
                                &topValue, &leftFrame);
        qreal bottomValue;
        qreal rightFrame;
        graphGetValueAndFrameFromPos(mSelectionRect.bottomRight(),
                                &bottomValue, &rightFrame);
        QRectF frameValueRect;
        frameValueRect.setTopLeft(QPointF(leftFrame, topValue) );
        frameValueRect.setBottomRight(QPointF(rightFrame, bottomValue) );

        QList<QrealKey*> keysList;
        foreach(QrealAnimator *animator, mAnimators) {
            animator->addKeysInRectToList(frameValueRect, &keysList);
        }
        foreach(QrealKey *key, keysList) {
            graphAddKeyToSelection(key);
        }

        mSelecting = false;
    } else if(mCurrentPoint != NULL) {
        if(mCurrentPoint->isKeyPoint()) {
            if(mFirstMove) {
                if(!mMainWindow->isShiftPressed()) {
                    graphClearKeysSelection();
                    graphAddKeyToSelection(mCurrentPoint->getParentKey());
                }
            }
            graphMergeKeysIfNeeded();
        } else {
               mCurrentPoint->setSelected(false);
        }
        mCurrentPoint = NULL;

        graphConstrainAnimatorCtrlsFrameValues();

        // needed ?
        graphUpdateDimensions();
    }
}

void KeysView::graphMiddlePress(QPointF pressPos)
{
    mSavedMinViewedFrame = mMinViewedFrame;
    mSavedMaxViewedFrame = mMaxViewedFrame;
    mSavedMinShownValue = mMinShownVal;
    mMiddlePressPos = pressPos;
}

void KeysView::graphMiddleMove(QPointF movePos)
{
    QPointF diffFrameValue = (movePos - mMiddlePressPos);
    diffFrameValue.setX(diffFrameValue.x()/mPixelsPerFrame);
    diffFrameValue.setY(diffFrameValue.y()/mPixelsPerValUnit);
    int roundX = qRound(diffFrameValue.x() );
    setFramesRange(mSavedMinViewedFrame - roundX,
                   mSavedMaxViewedFrame - roundX );
    graphSetMinShownVal(mSavedMinShownValue + diffFrameValue.y());
}

void KeysView::graphMiddleRelease()
{

}

void KeysView::graphConstrainAnimatorCtrlsFrameValues() {
    foreach(QrealAnimator *animator, mAnimators) {
        animator->qra_constrainCtrlsFrameValues();
    }
}

void KeysView::graphSetCtrlsModeForSelected(CtrlsMode mode) {
    if(mSelectedKeys.isEmpty()) return;
    QrealKey *key; foreachQK(key, mSelectedKeys)
        key->setCtrlsMode(mode);
    }
    graphConstrainAnimatorCtrlsFrameValues();
}

void KeysView::graphSetTwoSideCtrlForSelected() {
    if(mSelectedKeys.isEmpty()) return;
    QrealKey *key; foreachQK(key, mSelectedKeys)
        key->setEndEnabled(true);
        key->setStartEnabled(true);
    }
    graphConstrainAnimatorCtrlsFrameValues();
    graphRepaint();
}

void KeysView::graphSetRightSideCtrlForSelected() {
    if(mSelectedKeys.isEmpty()) return;
    QrealKey *key; foreachQK(key, mSelectedKeys)
        key->setEndEnabled(true);
        key->setStartEnabled(false);
    }
    graphConstrainAnimatorCtrlsFrameValues();
    graphRepaint();
}

void KeysView::graphSetLeftSideCtrlForSelected() {
    if(mSelectedKeys.isEmpty()) return;
    QrealKey *key; foreachQK(key, mSelectedKeys)
        key->setEndEnabled(false);
        key->setStartEnabled(true);
    }
    graphConstrainAnimatorCtrlsFrameValues();
    graphRepaint();
}

void KeysView::graphSetNoSideCtrlForSelected() {
    if(mSelectedKeys.isEmpty()) return;
    QrealKey *key; foreachQK(key, mSelectedKeys)
        key->setEndEnabled(false);
        key->setStartEnabled(false);
    }
    graphConstrainAnimatorCtrlsFrameValues();
    graphRepaint();
}

void KeysView::graphClearAnimatorSelection() {
    graphClearKeysSelection();

//    foreach(QrealAnimator *animator, mAnimators) {
//        animator->setIsCurrentAnimator(false);
//    }
    mAnimators.clear();
}

void KeysView::graphDeletePressed() {
    if(mCurrentPoint != NULL) {
            QrealKey *key = mCurrentPoint->getParentKey();
            if(mCurrentPoint->isEndPoint()) {
                key->setEndEnabled(false);
            } else if(mCurrentPoint->isStartPoint()) {
                key->setStartEnabled(false);
            }
    } else {
        QrealKey *key; foreachQK(key, mSelectedKeys)
            key->removeFromAnimator();
            key->setSelected(false);
        }
        mSelectedKeys.clear();

        foreach(QrealAnimator *animator, mAnimators) {
            animator->anim_sortKeys();
            animator->qra_updateKeysPath();
        }
    }
}

void KeysView::graphClearKeysSelection() {
    QrealKey *key; foreachQK(key, mSelectedKeys)
        key->setSelected(false);
    }

    mSelectedKeys.clear();
}

void KeysView::graphAddKeyToSelection(QrealKey *key)
{
    if(key->isSelected()) return;
    key->setSelected(true);
    mSelectedKeys << key;
}

void KeysView::graphRemoveKeyFromSelection(QrealKey *key)
{
    if(key->isSelected()) {
        key->setSelected(false);
        mSelectedKeys.removeOne(key);
    }
}

void KeysView::graphMouseMove(QPointF mousePos) {
    if(mSelecting) {
        mSelectionRect.setBottomRight(mousePos);
    } else if(mCurrentPoint != NULL) {
        if(!mCurrentPoint->isEnabled()) {
            QrealKey *parentKey = mCurrentPoint->getParentKey();
            parentKey->setEndEnabled(true);
            parentKey->setStartEnabled(true);
            parentKey->setCtrlsMode(CTRLS_SYMMETRIC);
        }
        qreal value;
        qreal frame;
        graphGetValueAndFrameFromPos(mousePos, &value, &frame);
        if(mCurrentPoint->isKeyPoint()) {
            QPointF currentFrameAndValue = QPointF(frame, value);
            if(mFirstMove) {
                QrealKey *key; foreachQK(key, mSelectedKeys)
                    key->saveCurrentFrameAndValue();
                    key->changeFrameAndValueBy(currentFrameAndValue -
                                               mPressFrameAndValue);
                }
            } else {
                QrealKey *key; foreachQK(key, mSelectedKeys)
                    key->changeFrameAndValueBy(currentFrameAndValue -
                                               mPressFrameAndValue);
                }
            }
            foreach(QrealAnimator *animator, mAnimators) {
                animator->anim_sortKeys();
            }
        } else {
            qreal clampedFrame = clamp(frame, mMinMoveFrame, mMaxMoveFrame);
            QrealAnimator *parentAnimator =
                    (QrealAnimator*)mCurrentPoint->getParentKey()->
                                        getParentAnimator();
            mCurrentPoint->moveTo(clampedFrame,
                                  parentAnimator->qra_clampValue(value));
        }
        foreach(QrealAnimator *animator, mAnimators) {
            animator->qra_updateKeysPath();
        }
    }
    mFirstMove = false;
}

void KeysView::graphMousePressEvent(QPoint eventPos,
                                     Qt::MouseButton eventButton) {
    if(eventButton == Qt::RightButton) {
        qreal value;
        qreal frame;
        graphGetValueAndFrameFromPos(eventPos, &value, &frame);
        QrealPoint *point = NULL;
        foreach(QrealAnimator *animator, mAnimators) {
            point = animator->qra_getPointAt(value, frame,
                                         mPixelsPerFrame, mPixelsPerValUnit);
            if(point != NULL) break;
        }
        if(point == NULL) return;
        QrealPointValueDialog *dialog = new QrealPointValueDialog(point, this);
        dialog->show();
        connect(dialog, SIGNAL(repaintSignal()),
                this, SLOT(graphUpdateAfterKeysChangedAndRepaint()) );
        connect(dialog, SIGNAL(finished(int)),
                this, SLOT(graphMergeKeysIfNeeded()) );
    } else if(eventButton == Qt::MiddleButton) {
        graphMiddlePress(eventPos);
    } else {
        graphMousePress(eventPos);
    }
}

void KeysView::graphMouseMoveEvent(QPoint eventPos,
                                    Qt::MouseButtons eventButtons) {
    if(eventButtons == Qt::MiddleButton) {
        graphMiddleMove(eventPos);
        emit changedViewedFrames(mMinViewedFrame,
                                 mMaxViewedFrame);
    } else {
        graphMouseMove(eventPos);
    }
}

void KeysView::graphMouseReleaseEvent(Qt::MouseButton eventButton) {
    if(eventButton == Qt::MiddleButton) {
        graphMiddleRelease();
    } else {
        graphMouseRelease();
    }
}

void KeysView::graphWheelEvent(QWheelEvent *event) {
    if(mMainWindow->isCtrlPressed()) {
        if(event->delta() > 0) {
            graphIncScale(0.1);
        } else {
            graphIncScale(-0.1);
        }
    } else {
        if(event->delta() > 0) {
            graphIncMinShownVal(0.1);
        } else {
            graphIncMinShownVal(-0.1);
        }
    }

    repaint();
}

bool KeysView::graphProcessFilteredKeyEvent(QKeyEvent *event)
{
    if(!hasFocus() ) return false;
    if(event->key() == Qt::Key_Delete) {
        graphDeletePressed();
        graphRepaint();
    } else {
        return false;
    }
    return true;
}

void KeysView::graphResetValueScaleAndMinShown() {
    qreal minVal;
    qreal maxVal;
    graphGetAnimatorsMinMaxValue(&minVal, &maxVal);
    graphSetMinShownVal(minVal);
    graphSetScale(1.);
}
#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"
void KeysView::updateAnimatorsColors() {
    int i = 0;
    foreach(QrealAnimator *animator, mAnimators) {
        animator->setAnimatorColor(mBoxesListVisible,
                                   ANIMATORCOLORS.at(
                                       i %
                                       ANIMATORCOLORS.length()));
        i++;
    }
}

void KeysView::graphAddViewedAnimator(QrealAnimator *animator) {
    mAnimators << animator;
    updateAnimatorsColors();
    graphUpdateDimensions();
    animator->qra_updateKeysPath();
    graphResetValueScaleAndMinShown();

    graphRepaint();
}

void KeysView::graphRemoveViewedAnimator(QrealAnimator *animator) {
    mAnimators.removeOne(animator);
    animator->removeAnimatorColor(mBoxesListVisible);
    updateAnimatorsColors();
    graphUpdateDimensions();
    graphResetValueScaleAndMinShown();

    graphRepaint();
}

void KeysView::graphRepaint() {
    //repaint();
}

void KeysView::graphUpdateAfterKeysChangedAndRepaint() {
    scheduleGraphUpdateAfterKeysChanged();
    
    mMainWindow->callUpdateSchedulers();
}

void KeysView::scheduleGraphUpdateAfterKeysChanged() {
    if(mGraphUpdateAfterKeysChangedNeeded) return;
    mGraphUpdateAfterKeysChangedNeeded = true;
}

void KeysView::graphUpdateAfterKeysChangedIfNeeded() {
    if(mGraphUpdateAfterKeysChangedNeeded) {
        mGraphUpdateAfterKeysChangedNeeded = false;
        graphUpdateAfterKeysChanged();
    }
}

void KeysView::graphUpdateAfterKeysChanged()
{
    foreach(QrealAnimator *animator, mAnimators) {
        animator->anim_sortKeys();
        animator->qra_updateKeysPath();
    }
    graphResetValueScaleAndMinShown();
    graphUpdateDimensions();
}

void KeysView::graphMergeKeysIfNeeded() {
    foreach(QrealAnimator *animator, mAnimators) {
        animator->anim_mergeKeysIfNeeded();
    }
}
