#include <QPainter>
#include <QMouseEvent>
#include "mainwindow.h"
#include "qrealpointvaluedialog.h"
#include "keysview.h"
#include "Animators/qrealpoint.h"
#include "global.h"
#include "qrealkey.h"

void KeysView::graphSetSmoothCtrlAction() {
    graphSetTwoSideCtrlForSelected();
    graphSetCtrlsModeForSelected(CTRLS_SMOOTH);
    mMainWindow->queScheduledTasksAndUpdate();
}

void KeysView::graphSetSymmetricCtrlAction() {
    graphSetTwoSideCtrlForSelected();
    graphSetCtrlsModeForSelected(CTRLS_SYMMETRIC);
    mMainWindow->queScheduledTasksAndUpdate();
}

void KeysView::graphSetCornerCtrlAction() {
    graphSetTwoSideCtrlForSelected();
    graphSetCtrlsModeForSelected(CTRLS_CORNER);
    mMainWindow->queScheduledTasksAndUpdate();
}

void KeysView::graphMakeSegmentsSmoothAction(const bool& smooth) {
    if(mSelectedKeysAnimators.isEmpty()) return;
    QList<QList<Key*>> segments;
    foreach(const auto& anim, mSelectedKeysAnimators) {
        //if(!mAnimators.contains(anim)) continue;
        anim->getSelectedSegments(segments);
    }

    Q_FOREACH(const auto& segment, segments) {
        Q_ASSERT(segment.length() > 1);
        auto firstKey = segment.first();
        auto lastKey = segment.last();
        firstKey->setEndEnabledForGraph(smooth);
        if(smooth) firstKey->makeStartAndEndSmooth();
        //firstKey->keyChanged();
        for(int i = 1; i < segment.length() - 1; i++) {
            auto innerKey = segment.at(i);
            innerKey->setEndEnabledForGraph(smooth);
            innerKey->setStartEnabledForGraph(smooth);
            if(smooth) innerKey->makeStartAndEndSmooth();
            //innerKey->keyChanged();
        }
        lastKey->setStartEnabledForGraph(smooth);
        if(smooth) lastKey->makeStartAndEndSmooth();
        lastKey->afterKeyChanged();
    }

    graphConstrainAnimatorCtrlsFrameValues();
    mMainWindow->queScheduledTasksAndUpdate();
}

void KeysView::graphMakeSegmentsLinearAction() {
    graphMakeSegmentsSmoothAction(false);
}

void KeysView::graphMakeSegmentsSmoothAction() {
    graphMakeSegmentsSmoothAction(true);
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
    int nLines = qCeil(yL/incY);
    QLine *lines = new QLine[static_cast<uint>(nLines)];
    int currLine = 0;
    while(yL > 0) {
        p->drawText(QRectF(-MIN_WIDGET_HEIGHT/4, yL - incY,
                           2*MIN_WIDGET_HEIGHT, 2*incY),
                    Qt::AlignCenter,
                    QString::number(currValue, 'f', mValuePrec));
        int yLi = qRound(yL);
        lines[currLine] = QLine(2*MIN_WIDGET_HEIGHT, yLi, width(), yLi);
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

    for(int i = 0; i < mGraphAnimators.count(); i++) {
        const QColor &col = ANIMATOR_COLORS.at(i % ANIMATOR_COLORS.length());
        mGraphAnimators.at(i)->drawKeysPath(p, col);
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

void KeysView::graphGetAnimatorsMinMaxValue(qreal &minVal, qreal &maxVal) {
    if(mGraphAnimators.isEmpty()) {
        minVal = 0.;
        maxVal = 0.;
    } else {
        minVal = 1000000;
        maxVal = -1000000;

        Q_FOREACH(const auto& anim, mGraphAnimators) {
            qreal animMinVal;
            qreal animMaxVal;
            if(anim->anim_graphValuesCorrespondToFrames()) {
                animMinVal = mMinViewedFrame;
                animMaxVal = mMaxViewedFrame;
            } else {
                anim->anim_getMinAndMaxValues(animMinVal, animMaxVal);
            }
            if(animMaxVal > maxVal) {
                maxVal = animMaxVal;
            }
            if(animMinVal < minVal) {
                minVal = animMinVal;
            }
        }
    }
    if(qAbs(minVal - maxVal) < 0.1 ) {
        minVal -= 0.05;
        maxVal += 0.05;
    }
    qreal valRange = maxVal - minVal;
    maxVal += valRange*0.05;
    minVal -= valRange*0.05;
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

    Q_FOREACH(const auto& anim, mGraphAnimators) {
        mCurrentPoint = anim->anim_getPointAt(value, frame,
                                              mPixelsPerFrame,
                                              mPixelsPerValUnit);
        if(mCurrentPoint != nullptr) break;
    }
    Key *parentKey = (mCurrentPoint == nullptr) ?
                nullptr :
                mCurrentPoint->getParentKey();
    if(mCurrentPoint == nullptr) {
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
                removeKeyFromSelection(parentKey);
            } else {
                addKeyToSelection(parentKey);
            }
        } else {
            if(!parentKey->isSelected()) {
                clearKeySelection();
                addKeyToSelection(parentKey);
            }
        }
    } else {
        auto parentAnimator =
                mCurrentPoint->getParentKey()->getParentAnimator();
        parentAnimator->
                getMinAndMaxMoveFrame(
                    mCurrentPoint->getParentKey(), mCurrentPoint,
                    mMinMoveFrame, mMaxMoveFrame);
        mCurrentPoint->setSelected(true);
    }
}

void KeysView::graphMouseRelease() {
    if(mSelecting) {
        if(!mMainWindow->isShiftPressed()) {
            clearKeySelection();
        }

        QList<Key*> keysList;
        Q_FOREACH(const auto& anim, mGraphAnimators) {
            anim->addKeysInRectToList(mSelectionRect, keysList);
        }
        Q_FOREACH(Key *const key, keysList) {
            addKeyToSelection(key);
        }

        mSelecting = false;
    } else if(mCurrentPoint != nullptr) {
        if(mCurrentPoint->isKeyPoint()) {
            if(mFirstMove) {
                if(!mMainWindow->isShiftPressed()) {
                    clearKeySelection();
                    addKeyToSelection(mCurrentPoint->getParentKey());
                }
            } else {
                foreach(const auto& anim, mSelectedKeysAnimators) {
                    if(!mGraphAnimators.contains(anim)) continue;
                    anim->finishSelectedKeysTransform();
//                    key->finishValueTransform();
                }
            }
            graphMergeKeysIfNeeded();
        } else {
               mCurrentPoint->setSelected(false);
        }
        mCurrentPoint = nullptr;

        Q_FOREACH(const auto& anim, mGraphAnimators) {
            anim->anim_constrainCtrlsFrameValues();
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
    Q_FOREACH(const auto& anim, mGraphAnimators) {
        anim->anim_constrainCtrlsFrameValues();
    }
}

void KeysView::graphSetCtrlsModeForSelected(const CtrlsMode &mode) {
    if(mSelectedKeysAnimators.isEmpty()) return;

    foreach(const auto& anim, mSelectedKeysAnimators) {
        //if(!mAnimators.contains(anim)) continue;
        anim->setCtrlsModeForSelectedKeys(mode);
    }
    graphConstrainAnimatorCtrlsFrameValues();
}

void KeysView::graphSetTwoSideCtrlForSelected() {
    if(mSelectedKeysAnimators.isEmpty()) return;
    foreach(const auto& anim, mSelectedKeysAnimators) {
        //if(!mAnimators.contains(anim)) continue;
        anim->enableCtrlPtsForSelected();
    }
    graphConstrainAnimatorCtrlsFrameValues();
}

void KeysView::graphClearAnimatorSelection() {
    clearKeySelection();

//    Q_FOREACH(QrealAnimator *animator, mAnimators) {
//        animator->setIsCurrentAnimator(false);
//    }
    mGraphAnimators.clear();
}

void KeysView::graphDeletePressed() {
    if(mCurrentPoint != nullptr) {
        Key *key = mCurrentPoint->getParentKey();
        if(mCurrentPoint->isEndPoint()) {
            key->setEndEnabledForGraph(false);
        } else if(mCurrentPoint->isStartPoint()) {
            key->setStartEnabledForGraph(false);
        }
        key->afterKeyChanged();
        key->getParentAnimator()->anim_updateKeysPath();
        mCurrentPoint = nullptr;
    } else {
        foreach(const auto& anim, mSelectedKeysAnimators) {
            //if(!mAnimators.contains(anim)) continue;
            anim->deleteSelectedKeys();
        }

        Q_FOREACH(const auto& anim, mGraphAnimators) {
            anim->anim_sortKeys();
            anim->anim_updateKeysPath();
        }
    }
}

void KeysView::graphMouseMove(const QPointF &mousePos) {
    if(mSelecting) {
        qreal value;
        qreal frame;
        graphGetValueAndFrameFromPos(mousePos, &value, &frame);
        mSelectionRect.setBottomRight(QPointF(frame, value));
    } else if(mCurrentPoint != nullptr) {
        if(!mCurrentPoint->isEnabled()) {
            Key *parentKey = mCurrentPoint->getParentKey();
            parentKey->setEndEnabledForGraph(true);
            parentKey->setStartEnabledForGraph(true);
            parentKey->setCtrlsMode(CTRLS_SYMMETRIC);
        }
        qreal value;
        qreal frame;
        graphGetValueAndFrameFromPos(mousePos, &value, &frame);
        if(mCurrentPoint->isKeyPoint()) {
            QPointF currentFrameAndValue = QPointF(frame, value);
            QPointF frameValueChange = currentFrameAndValue -
                    mPressFrameAndValue;
            if(mFirstMove) {
                foreach(const auto& anim, mSelectedKeysAnimators) {
                    //if(!mAnimators.contains(anim)) continue;
                    anim->changeSelectedKeysFrameAndValueStart(frameValueChange);
                }
            } else {
                foreach(const auto& anim, mSelectedKeysAnimators) {
                    //if(!mAnimators.contains(anim)) continue;
                    anim->changeSelectedKeysFrameAndValue(frameValueChange);
                }
            }
            Q_FOREACH(const auto& anim, mGraphAnimators) {
                anim->anim_sortKeys();
            }
        } else {
            qreal clampedFrame = clamp(frame, mMinMoveFrame, mMaxMoveFrame);
            auto parentAnimator = mCurrentPoint->getParentKey()->
                                        getParentAnimator();
            mCurrentPoint->moveTo(clampedFrame,
                                  parentAnimator->clampGraphValue(value));
        }
        Q_FOREACH(const auto& anim, mGraphAnimators) {
            anim->anim_updateKeysPath();
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
        QrealPoint *point = nullptr;
        Q_FOREACH(const auto& anim, mGraphAnimators) {
            point = anim->anim_getPointAt(value, frame,
                                         mPixelsPerFrame, mPixelsPerValUnit);
            if(point != nullptr) break;
        }
        if(point == nullptr) return;
        QrealPointValueDialog *dialog = new QrealPointValueDialog(point, this);
        dialog->show();
        connect(dialog, &QrealPointValueDialog::repaintSignal,
                this, &KeysView::graphUpdateAfterKeysChangedAndRepaint);
        connect(dialog, &QrealPointValueDialog::finished,
                this, &KeysView::graphMergeKeysIfNeeded);
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

bool KeysView::graphProcessFilteredKeyEvent(QKeyEvent *event) {
    if(!hasFocus() ) return false;
    if(event->key() == Qt::Key_Delete) {
        graphDeletePressed();
    } else if(event->key() == Qt::Key_0 &&
              event->modifiers() & Qt::KeypadModifier) {
        graphResetValueScaleAndMinShownAction();
    } else {
        return false;
    }
    return true;
}

void KeysView::graphResetValueScaleAndMinShownAction() {
    graphResetValueScaleAndMinShown();
    mMainWindow->queScheduledTasksAndUpdate();
}

void KeysView::graphResetValueScaleAndMinShown() {
    qreal minVal;
    qreal maxVal;
    graphGetAnimatorsMinMaxValue(minVal, maxVal);
    graphSetMinShownVal(minVal);
    mPixelsPerValUnit = height()/(maxVal - minVal);
    graphUpdateDimensions();
}

void KeysView::updateAnimatorsColors() {
    int i = 0;
    Q_FOREACH(const auto& anim, mGraphAnimators) {
        anim->setAnimatorColor(mBoxesListVisible,
                                   ANIMATOR_COLORS.at(
                                       i %
                                       ANIMATOR_COLORS.length()));
        i++;
    }
}

void KeysView::graphAddViewedAnimator(Animator * const animator) {
    mGraphAnimators << animator;
    animator->incSelectedForGraph();
    updateAnimatorsColors();
    graphUpdateDimensions();
    animator->anim_updateKeysPath();
    graphResetValueScaleAndMinShown();

    mMainWindow->queScheduledTasksAndUpdate();
}

void KeysView::graphRemoveViewedAnimator(Animator * const animator) {
    mGraphAnimators.removeOne(animator);
    animator->decSelectedForGraph();
    animator->removeAnimatorColor(mBoxesListVisible);
    updateAnimatorsColors();
    graphUpdateDimensions();
    graphResetValueScaleAndMinShown();

    mMainWindow->queScheduledTasksAndUpdate();
}

void KeysView::graphUpdateAfterKeysChangedAndRepaint() {
    scheduleGraphUpdateAfterKeysChanged();
    
    mMainWindow->queScheduledTasksAndUpdate();
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

void KeysView::graphUpdateAfterKeysChanged() {
    Q_FOREACH(const auto& anim, mGraphAnimators) {
        anim->anim_sortKeys();
        anim->anim_updateKeysPath();
    }
    graphResetValueScaleAndMinShown();
    graphUpdateDimensions();
}

void KeysView::graphMergeKeysIfNeeded() {
    Q_FOREACH(const auto& anim, mGraphAnimators) {
        anim->anim_mergeKeysIfNeeded();
    }
}
