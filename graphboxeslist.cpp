#include <QPainter>
#include <QMouseEvent>
#include "mainwindow.h"
#include "qrealpointvaluedialog.h"
#include "keysview.h"
#include "updatescheduler.h"
#include "qrealpoint.h"
#include "global.h"
#include "qrealkey.h"

void KeysView::graphSetSmoothCtrl() {
    graphSetTwoSideCtrlForSelected();
    graphSetCtrlsModeForSelected(CTRLS_SMOOTH);
    mMainWindow->callUpdateSchedulers();
}

void KeysView::graphSetSymmetricCtrl() {
    graphSetTwoSideCtrlForSelected();
    graphSetCtrlsModeForSelected(CTRLS_SYMMETRIC);
    mMainWindow->callUpdateSchedulers();
}

void KeysView::graphSetCornerCtrl() {
    graphSetTwoSideCtrlForSelected();
    graphSetCtrlsModeForSelected(CTRLS_CORNER);
    mMainWindow->callUpdateSchedulers();
}

void KeysView::graphPaint(QPainter *p) {
    p->setBrush(Qt::NoBrush);

    /*qreal maxX = width();
    int currAlpha = 75;
    qreal lineWidth = 1.;
    QList<int> incFrameList = { 1, 5, 10, 100 };
    Q_FOREACH(int incFrame, incFrameList) {
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
    QPen pen = QPen(QColor(255, 255, 255), 1.);
    p->setPen(pen);
    qreal incY = mValueInc*mPixelsPerValUnit;
    qreal yL = height() + fmod(mMinShownVal, mValueInc)*mPixelsPerValUnit + incY;
    qreal currValue = mMinShownVal - fmod(mMinShownVal, mValueInc) - mValueInc;
    int nLines = qCeil(yL / incY);
    QLine *lines = new QLine[nLines];
    int currLine = 0;
    while(yL > 0) {
        p->drawText(QRectF(-MIN_WIDGET_HEIGHT/4, yL - incY,
                           2*MIN_WIDGET_HEIGHT, 2*incY),
                    Qt::AlignCenter,
                    QString::number(currValue, 'f', mValuePrec));
        lines[currLine] = QLine(2*MIN_WIDGET_HEIGHT, yL, width(), yL);
        currLine++;
        yL -= incY;
        currValue += mValueInc;
    }
    pen.setColor(QColor(200, 200, 200));
    p->setPen(pen);
    p->drawLines(lines, nLines);
    delete[] lines;
    p->setRenderHint(QPainter::Antialiasing);

    QMatrix transform;
    transform.translate(-mPixelsPerFrame*(mMinViewedFrame - 0.5),
                        height() + mPixelsPerValUnit*mMinShownVal);
    transform.scale(mPixelsPerFrame, -mPixelsPerValUnit);
    p->setTransform(QTransform(transform), true);

    for(int i = 0; i < mAnimators.count(); i++) {
        const QColor &col = ANIMATORCOLORS.at(i % ANIMATORCOLORS.length());
        QrealAnimator *animator = mAnimators.at(i);
        animator->drawKeysPath(p, col);
    }
    p->setRenderHint(QPainter::Antialiasing, false);

    if(mSelecting) {
        pen.setColor(Qt::blue);
        pen.setWidthF(2.);
        pen.setStyle(Qt::DotLine);
        pen.setCosmetic(true);
        p->setPen(pen);
        p->setBrush(Qt::NoBrush);
        p->drawRect(mSelectionRect);
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

void KeysView::graphGetAnimatorsMinMaxValue(qreal *minVal, qreal *maxVal) {
    qreal minValT;
    qreal maxValT;
    if(mAnimators.isEmpty()) {
        minValT = 0.;
        maxValT = 0.;
    } else {
        minValT = 1000000;
        maxValT = -1000000;

        Q_FOREACH(QrealAnimator *animator, mAnimators) {
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
    }
    if(qAbs(minValT - maxValT) < 0.1 ) {
        minValT -= 0.05;
        maxValT += 0.05;
    }
    qreal valRange = maxValT - minValT;
    maxValT += valRange*0.05;
    minValT -= valRange*0.05;

    *minVal = minValT;
    *maxVal = maxValT;
}

const QList<qreal> validIncs = {7.5, 5., 2.5, 1.};
void KeysView::graphUpdateDimensions() {

    qreal incMulti = 10000.;
    int currIncId = 0;
    int nDiv = 0;
    mValueInc = validIncs.first()*incMulti;
    while(true) {
        mValueInc = validIncs.at(currIncId)*incMulti;
        if(mValueInc*mPixelsPerValUnit < 50.) {
            break;
        }
        currIncId++;
        if(currIncId >= validIncs.count()) {
            currIncId = 0;
            incMulti *= 0.1;
            nDiv++;
        }
    }
    mValuePrec = qMax(nDiv - 3, 0);

    graphIncMinShownVal(0.);
    updatePixelsPerFrame();
}

void KeysView::graphResizeEvent(QResizeEvent *) {
    graphUpdateDimensions();
}

void KeysView::graphIncMinShownVal(const qreal &inc) {
    graphSetMinShownVal((MIN_WIDGET_HEIGHT/2)*inc/mPixelsPerValUnit +
                        mMinShownVal);
}

void KeysView::graphSetMinShownVal(const qreal &newMinShownVal) {
    mMinShownVal = newMinShownVal;
}

void KeysView::graphGetValueAndFrameFromPos(const QPointF &pos,
                                            qreal *value, qreal *frame) {
    *value = (height() - pos.y())/mPixelsPerValUnit + mMinShownVal;
    *frame = mMinViewedFrame + pos.x()/mPixelsPerFrame - 0.5;
}

void KeysView::graphMousePress(const QPointF &pressPos) {
    mFirstMove = true;
    qreal value;
    qreal frame;
    graphGetValueAndFrameFromPos(pressPos, &value, &frame);

    Q_FOREACH(QrealAnimator *animator, mAnimators) {
        mCurrentPoint = animator->qra_getPointAt(value, frame,
                                                 mPixelsPerFrame,
                                                 mPixelsPerValUnit);
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
            qreal value;
            qreal frame;
            graphGetValueAndFrameFromPos(pressPos, &value, &frame);
            mSelectionRect.setBottomRight(QPointF(frame, value));
            mSelectionRect.setTopLeft(QPointF(frame, value));
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

        QList<QrealKey*> keysList;
        Q_FOREACH(QrealAnimator *animator, mAnimators) {
            animator->addKeysInRectToList(mSelectionRect, &keysList);
        }
        Q_FOREACH(QrealKey *key, keysList) {
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
            } else {
                QrealKey *key; Q_FOREACHQK(key, mSelectedKeys)
                    key->finishFrameTransform();
                    key->finishValueTransform();
                }
            }
            graphMergeKeysIfNeeded();
        } else {
               mCurrentPoint->setSelected(false);
        }
        mCurrentPoint = NULL;

        Q_FOREACH(QrealAnimator *animator, mAnimators) {
            animator->qra_constrainCtrlsFrameValues();
        }
        //graphConstrainAnimatorCtrlsFrameValues();

        // needed ?
        graphUpdateDimensions();
    }
}

void KeysView::graphMiddlePress(const QPointF &pressPos) {
    mSavedMinViewedFrame = mMinViewedFrame;
    mSavedMaxViewedFrame = mMaxViewedFrame;
    mSavedMinShownValue = mMinShownVal;
    mMiddlePressPos = pressPos;
}

void KeysView::graphMiddleMove(const QPointF &movePos) {
    QPointF diffFrameValue = (movePos - mMiddlePressPos);
    diffFrameValue.setX(diffFrameValue.x()/mPixelsPerFrame);
    diffFrameValue.setY(diffFrameValue.y()/mPixelsPerValUnit);
    int roundX = qRound(diffFrameValue.x() );
    setFramesRange(mSavedMinViewedFrame - roundX,
                   mSavedMaxViewedFrame - roundX );
    graphSetMinShownVal(mSavedMinShownValue + diffFrameValue.y());
}

void KeysView::graphMiddleRelease() {

}

void KeysView::graphConstrainAnimatorCtrlsFrameValues() {
    Q_FOREACH(QrealAnimator *animator, mAnimators) {
        animator->qra_constrainCtrlsFrameValues();
    }
}

void KeysView::graphSetCtrlsModeForSelected(const CtrlsMode &mode) {
    if(mSelectedKeys.isEmpty()) return;
    QrealKey *key; Q_FOREACHQK(key, mSelectedKeys)
        key->setCtrlsMode(mode);
    }
    graphConstrainAnimatorCtrlsFrameValues();
}

void KeysView::graphSetTwoSideCtrlForSelected() {
    if(mSelectedKeys.isEmpty()) return;
    QrealKey *key; Q_FOREACHQK(key, mSelectedKeys)
        key->setEndEnabled(true);
        key->setStartEnabled(true);
    }
    graphConstrainAnimatorCtrlsFrameValues();
    mMainWindow->callUpdateSchedulers();
}

void KeysView::graphSetRightSideCtrlForSelected() {
    if(mSelectedKeys.isEmpty()) return;
    QrealKey *key; Q_FOREACHQK(key, mSelectedKeys)
        key->setEndEnabled(true);
        key->setStartEnabled(false);
    }
    graphConstrainAnimatorCtrlsFrameValues();
    mMainWindow->callUpdateSchedulers();
}

void KeysView::graphSetLeftSideCtrlForSelected() {
    if(mSelectedKeys.isEmpty()) return;
    QrealKey *key; Q_FOREACHQK(key, mSelectedKeys)
        key->setEndEnabled(false);
        key->setStartEnabled(true);
    }
    graphConstrainAnimatorCtrlsFrameValues();
    mMainWindow->callUpdateSchedulers();
}

void KeysView::graphSetNoSideCtrlForSelected() {
    if(mSelectedKeys.isEmpty()) return;
    QrealKey *key; Q_FOREACHQK(key, mSelectedKeys)
        key->setEndEnabled(false);
        key->setStartEnabled(false);
    }
    graphConstrainAnimatorCtrlsFrameValues();
    mMainWindow->callUpdateSchedulers();
}

void KeysView::graphClearAnimatorSelection() {
    graphClearKeysSelection();

//    Q_FOREACH(QrealAnimator *animator, mAnimators) {
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
        QrealKey *key; Q_FOREACHQK(key, mSelectedKeys)
            key->removeFromAnimator();
            key->setSelected(false);
        }
        mSelectedKeys.clear();

        Q_FOREACH(QrealAnimator *animator, mAnimators) {
            animator->anim_sortKeys();
            animator->qra_updateKeysPath();
        }
    }
}

void KeysView::graphClearKeysSelection() {
    QrealKey *key; Q_FOREACHQK(key, mSelectedKeys)
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

void KeysView::graphMouseMove(const QPointF &mousePos) {
    if(mSelecting) {
        qreal value;
        qreal frame;
        graphGetValueAndFrameFromPos(mousePos, &value, &frame);
        mSelectionRect.setBottomRight(QPointF(frame, value));
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
                QrealKey *key; Q_FOREACHQK(key, mSelectedKeys)
                    key->saveCurrentFrameAndValue();
                    key->changeFrameAndValueBy(currentFrameAndValue -
                                               mPressFrameAndValue);
                }
            } else {
                QrealKey *key; Q_FOREACHQK(key, mSelectedKeys)
                    key->changeFrameAndValueBy(currentFrameAndValue -
                                               mPressFrameAndValue);
                }
            }
            Q_FOREACH(QrealAnimator *animator, mAnimators) {
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
        Q_FOREACH(QrealAnimator *animator, mAnimators) {
            animator->qra_updateKeysPath();
        }
    }
    mFirstMove = false;
}

void KeysView::graphMousePressEvent(const QPoint &eventPos,
                                    const Qt::MouseButton &eventButton) {
    if(eventButton == Qt::RightButton) {
        qreal value;
        qreal frame;
        graphGetValueAndFrameFromPos(eventPos, &value, &frame);
        QrealPoint *point = NULL;
        Q_FOREACH(QrealAnimator *animator, mAnimators) {
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

void KeysView::graphMouseMoveEvent(const QPoint &eventPos,
                                   const Qt::MouseButtons &eventButtons) {
    if(eventButtons == Qt::MiddleButton) {
        graphMiddleMove(eventPos);
        emit changedViewedFrames(mMinViewedFrame,
                                 mMaxViewedFrame);
    } else {
        graphMouseMove(eventPos);
    }
}

void KeysView::graphMouseReleaseEvent(const Qt::MouseButton &eventButton) {
    if(eventButton == Qt::MiddleButton) {
        graphMiddleRelease();
    } else {
        graphMouseRelease();
    }
}

void KeysView::graphWheelEvent(QWheelEvent *event) {
    if(event->modifiers() & Qt::ControlModifier) {
        qreal valUnderMouse;
        qreal frame;
        graphGetValueAndFrameFromPos(event->posF(),
                                     &valUnderMouse,
                                     &frame);
        qreal graphScaleInc;
        if(event->delta() > 0) {
            graphScaleInc = 0.1;
        } else {
            graphScaleInc = -0.1;
        }
        graphSetMinShownVal(mMinShownVal +
                            (valUnderMouse - mMinShownVal)*graphScaleInc);
        mPixelsPerValUnit += graphScaleInc*mPixelsPerValUnit;
        graphUpdateDimensions();
    } else {
        if(event->delta() > 0) {
            graphIncMinShownVal(1.);
        } else {
            graphIncMinShownVal(-1.);
        }
    }

    update();
}

bool KeysView::graphProcessFilteredKeyEvent(QKeyEvent *event)
{
    if(!hasFocus() ) return false;
    if(event->key() == Qt::Key_Delete) {
        graphDeletePressed();
        mMainWindow->callUpdateSchedulers();
    } else {
        return false;
    }
    return true;
}

void KeysView::graphResetValueScaleAndMinShownAction() {
    graphResetValueScaleAndMinShown();
    mMainWindow->callUpdateSchedulers();
}

void KeysView::graphResetValueScaleAndMinShown() {
    qreal minVal;
    qreal maxVal;
    graphGetAnimatorsMinMaxValue(&minVal, &maxVal);
    graphSetMinShownVal(minVal);
    mPixelsPerValUnit = height()/(maxVal - minVal);
    graphUpdateDimensions();
}

#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"
void KeysView::updateAnimatorsColors() {
    int i = 0;
    Q_FOREACH(QrealAnimator *animator, mAnimators) {
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

    mMainWindow->callUpdateSchedulers();
}

void KeysView::graphRemoveViewedAnimator(QrealAnimator *animator) {
    mAnimators.removeOne(animator);
    animator->removeAnimatorColor(mBoxesListVisible);
    updateAnimatorsColors();
    graphUpdateDimensions();
    graphResetValueScaleAndMinShown();

    mMainWindow->callUpdateSchedulers();
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
    Q_FOREACH(QrealAnimator *animator, mAnimators) {
        animator->anim_sortKeys();
        animator->qra_updateKeysPath();
    }
    graphResetValueScaleAndMinShown();
    graphUpdateDimensions();
}

void KeysView::graphMergeKeysIfNeeded() {
    Q_FOREACH(QrealAnimator *animator, mAnimators) {
        animator->anim_mergeKeysIfNeeded();
    }
}
