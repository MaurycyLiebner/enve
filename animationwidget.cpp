#include "animationwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include "mainwindow.h"
#include "qrealpointvaluedialog.h"
#include "boxeslist.h"

AnimationWidget::AnimationWidget(QWidget *parent, MainWindow *window) :
    QWidget(parent)
{
    mMainWindow = window;
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(200, 200);
}

void AnimationWidget::setSmoothCtrl()
{
    if(mAnimator == NULL) return;
    setTwoSideCtrlForSelected();
    setCtrlsModeForSelected(CTRLS_SMOOTH);
    repaintWithBoxesList();
}

void AnimationWidget::setSymmetricCtrl()
{
    if(mAnimator == NULL) return;
    setTwoSideCtrlForSelected();
    setCtrlsModeForSelected(CTRLS_SYMMETRIC);
    repaintWithBoxesList();
}

void AnimationWidget::setCornerCtrl()
{
    if(mAnimator == NULL) return;
    setTwoSideCtrlForSelected();
    setCtrlsModeForSelected(CTRLS_CORNER);
    repaintWithBoxesList();
}

void AnimationWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);


    p.fillRect(rect(), QColor(150, 150, 150));
    p.setBrush(Qt::NoBrush);

    p.fillRect(0, 0, width(), 20, Qt::white);
    p.fillRect(0, 0, 40, height(), Qt::white);

    qreal maxX = width();
    int currAlpha = 75;
    qreal lineWidth = 1.;
    QList<int> incFrameList = { 1, 5, 10, 100 };
    foreach(int incFrame, incFrameList) {
        if(mPixelsPerFrame*incFrame < 15.) continue;
        bool drawText = mPixelsPerFrame*incFrame > 30.;
        p.setPen(QPen(QColor(0, 0, 0, currAlpha), lineWidth));
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
                p.drawText(QRectF(xL - inc, 0, 2*inc, 20),
                            Qt::AlignCenter, QString::number(currFrame));
            }
            p.drawLine(xL, 20, xL, height());
            xL += inc;
            currFrame += incFrame;
        }
        currAlpha *= 1.5;
        lineWidth *= 1.5;
    }

    p.setPen(QPen(Qt::green, 2.f));
    qreal xL = (mCurrentFrame - mStartFrame)*mPixelsPerFrame;
    /*p.drawText(QRectF(xL - 20, 0, 40, 20),
                Qt::AlignCenter, QString::number(mCurrentFrame));*/
    p.drawLine(xL, 20, xL, height());


    if(mAnimator != NULL) {
        lineWidth = 2.;
        qreal incValue = 1000.;
        bool by2 = true;
        currAlpha = 255;
        while(mPixelsPerValUnit*incValue > 50.) {
            qreal yL = height() +
                    fmod(mMinShownVal, incValue)*mPixelsPerValUnit - mMargin;
            qreal incY = incValue*mPixelsPerValUnit;
            if( yL > 20 && (yL < height() || yL - incY > 20) ) {
                p.setPen(QPen(QColor(0, 0, 0, currAlpha), lineWidth));
                qreal currValue = mMinShownVal - fmod(mMinShownVal, incValue);
                while(yL > 20) {
                    p.drawText(QRectF(0, yL - incY, 40, 2*incY),
                                Qt::AlignCenter, QString::number(currValue));
                    p.drawLine(40, yL, width(), yL);
                    yL -= incY;
                    currValue += incValue;
                }
                currAlpha *= 0.95;
                lineWidth *= 0.85;
            }
            if(by2) {
                incValue /= 2;
            } else {
                incValue /= 5;
            }
            by2 = !by2;
        }

        p.save();
        p.setClipRect(40, 20, width() - 20, height() - 20);

        mAnimator->drawKeysPath(&p,
                                height(), mMargin,
                                mStartFrame, mMinShownVal,
                                mPixelsPerFrame, mPixelsPerValUnit);
    }

    if(mSelecting) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(Qt::blue, 2.f, Qt::DotLine));
        p.drawRect(mSelectionRect);
    }
    p.restore();



    if(hasFocus() ) {
        p.setPen(QPen(Qt::red, 4.));
    } else {
        p.setPen(Qt::NoPen);
    }
    p.setBrush(Qt::NoBrush);
    p.drawRect(rect());
    p.end();
}

void AnimationWidget::setViewedFramesRange(int startFrame, int endFrame) {
    mStartFrame = startFrame;
    mEndFrame = endFrame;
    updateDimensions();

    repaintWithBoxesList();
}

void AnimationWidget::setCurrentFrame(int frame)
{
    mCurrentFrame = frame;
}

void AnimationWidget::incScale(qreal inc) {
    qreal newScale = mValueScale + inc;
    setScale(clamp(newScale, 0.1, 10.));
}

void AnimationWidget::setScale(qreal scale) {
    mValueScale = scale;
    updateDimensions();
}

void AnimationWidget::updateDimensions() {
    if(mAnimator == NULL) return;
    mAnimator->getMinAndMaxValues(&mMinVal, &mMaxVal);
    if(qAbs(mMinVal - mMaxVal) < 0.1 ) {
        mMinVal -= 0.05;
        mMaxVal += 0.05;
    }
    mPixelsPerValUnit = mValueScale*(height() - 2*mMargin)/
                                (mMaxVal - mMinVal);
    incMinShownVal(0.);
    int dFrame = mEndFrame - mStartFrame;
    mPixelsPerFrame = width()/dFrame;
    mAnimator->setDrawPathUpdateNeeded();
}

void AnimationWidget::resizeEvent(QResizeEvent *)
{
    updateDimensions();
    updateDrawPath();
    if(mAnimator == NULL) return;
    mAnimator->updateDrawPathIfNeeded(height(), mMargin,
                                      mStartFrame, mMinShownVal,
                                      mPixelsPerFrame, mPixelsPerValUnit);
}

void AnimationWidget::incMinShownVal(qreal inc) {
    setMinShownVal(inc*(mMaxVal - mMinVal) + mMinShownVal);
}

void AnimationWidget::setMinShownVal(qreal newMinShownVal) {
    qreal halfHeightVal = (height() - 2*mMargin)*0.5/mPixelsPerValUnit;
    mMinShownVal = clamp(newMinShownVal,
                         mMinVal - halfHeightVal,
                         mMaxVal - halfHeightVal);
    updateDrawPath();
}

void AnimationWidget::getValueAndFrameFromPos(QPointF pos,
                                            qreal *value, qreal *frame) {
    *value = (height() - pos.y() - mMargin)/mPixelsPerValUnit
            + mMinShownVal;
    *frame = mStartFrame + pos.x()/mPixelsPerFrame;
}

void AnimationWidget::mousePress(QPointF pressPos) {
    mFirstMove = true;
    qreal value;
    qreal frame;
    getValueAndFrameFromPos(pressPos, &value, &frame);

    mCurrentPoint = mAnimator->getPointAt(value, frame,
                                          mPixelsPerFrame, mPixelsPerValUnit);
    QrealKey *parentKey = (mCurrentPoint == NULL) ?
                NULL :
                mCurrentPoint->getParentKey();
    if(mCurrentPoint == NULL) {
        if(isCtrlPressed() ) {
            clearKeysSelection();
            QrealKey *newKey = new QrealKey(qRound(frame), mAnimator, value);
            mAnimator->appendKey(newKey);
            addKeyToSelection(newKey);
            mergeKeysIfNeeded();
            mCurrentPoint = newKey->getEndPoint();
            mAnimator->getMinAndMaxMoveFrame(newKey, mCurrentPoint,
                                             &mMinMoveFrame, &mMaxMoveFrame);
            mCurrentPoint->setSelected(true);
            mAnimator->updateKeysPath();
        } else {
            mSelecting = true;
            mSelectionRect.setTopLeft(pressPos);
            mSelectionRect.setBottomRight(pressPos);
        }
    } else if(mCurrentPoint->isKeyPoint()) {
        mPressFrameAndValue = QPointF(frame, value);
        if(isShiftPressed()) {
            if(parentKey->isSelected()) {
                removeKeyFromSelection(parentKey);
            } else {
                addKeyToSelection(parentKey);
            }
        } else {
            if(!parentKey->isSelected()) {
                clearKeysSelection();
                addKeyToSelection(parentKey);
            }
        }
    } else {
        mAnimator->getMinAndMaxMoveFrame(
                    mCurrentPoint->getParentKey(), mCurrentPoint,
                    &mMinMoveFrame, &mMaxMoveFrame);
        mCurrentPoint->setSelected(true);
    }
}

void AnimationWidget::mouseRelease()
{
    if(mSelecting) {
        if(!isShiftPressed()) {
            clearKeysSelection();
        }
        qreal topValue;
        qreal leftFrame;
        getValueAndFrameFromPos(mSelectionRect.topLeft(),
                                &topValue, &leftFrame);
        qreal bottomValue;
        qreal rightFrame;
        getValueAndFrameFromPos(mSelectionRect.bottomRight(),
                                &bottomValue, &rightFrame);
        QRectF frameValueRect;
        frameValueRect.setTopLeft(QPointF(leftFrame, topValue) );
        frameValueRect.setBottomRight(QPointF(rightFrame, bottomValue) );

        QList<QrealKey*> keysList;
        mAnimator->addKeysInRectToList(frameValueRect, &keysList);
        foreach(QrealKey *key, keysList) {
            addKeyToSelection(key);
        }

        mSelecting = false;
    } else if(mCurrentPoint != NULL) {
        if(mCurrentPoint->isKeyPoint()) {
            if(mFirstMove) {
                if(!isShiftPressed()) {
                    clearKeysSelection();
                    addKeyToSelection(mCurrentPoint->getParentKey());
                }
            }
            mergeKeysIfNeeded();
        } else {
               mCurrentPoint->setSelected(false);
        }
        mCurrentPoint = NULL;

        mAnimator->constrainCtrlsFrameValues();
    }
}

void AnimationWidget::middlePress(QPointF pressPos)
{
    mSavedStartFrame = mStartFrame;
    mSavedEndFrame = mEndFrame;
    mSavedMinShownValue = mMinShownVal;
    mMiddlePressPos = pressPos;
}

void AnimationWidget::middleMove(QPointF movePos)
{
    QPointF diffFrameValue = (movePos - mMiddlePressPos);
    diffFrameValue.setX(diffFrameValue.x()/mPixelsPerFrame);
    diffFrameValue.setY(diffFrameValue.y()/mPixelsPerValUnit);
    if(qAbs(diffFrameValue.x() ) > 1) {
        mStartFrame = mSavedStartFrame - diffFrameValue.x();
        mEndFrame = mSavedEndFrame - diffFrameValue.x();
    }
    setMinShownVal(mSavedMinShownValue + diffFrameValue.y());
}

void AnimationWidget::middleRelease()
{

}

void AnimationWidget::setCtrlsModeForSelected(CtrlsMode mode) {
    if(mSelectedKeys.isEmpty()) return;
    foreach(QrealKey *key, mSelectedKeys) {
        key->setCtrlsMode(mode);
    }
    mAnimator->constrainCtrlsFrameValues();
}

void AnimationWidget::setTwoSideCtrlForSelected() {
    if(mSelectedKeys.isEmpty()) return;
    foreach(QrealKey *key, mSelectedKeys) {
        key->setEndEnabled(true);
        key->setStartEnabled(true);
    }
    mAnimator->constrainCtrlsFrameValues();
    repaintWithBoxesList();
}

void AnimationWidget::setRightSideCtrlForSelected() {
    if(mSelectedKeys.isEmpty()) return;
    foreach(QrealKey *key, mSelectedKeys) {
        key->setEndEnabled(true);
        key->setStartEnabled(false);
    }
    mAnimator->constrainCtrlsFrameValues();
    repaintWithBoxesList();
}

void AnimationWidget::setLeftSideCtrlForSelected() {
    if(mSelectedKeys.isEmpty()) return;
    foreach(QrealKey *key, mSelectedKeys) {
        key->setEndEnabled(false);
        key->setStartEnabled(true);
    }
    mAnimator->constrainCtrlsFrameValues();
    repaintWithBoxesList();
}

void AnimationWidget::setNoSideCtrlForSelected() {
    if(mSelectedKeys.isEmpty()) return;
    foreach(QrealKey *key, mSelectedKeys) {
        key->setEndEnabled(false);
        key->setStartEnabled(false);
    }
    repaintWithBoxesList();
}

void AnimationWidget::deletePressed()
{
    if(mCurrentPoint != NULL) {
            QrealKey *key = mCurrentPoint->getParentKey();
            if(mCurrentPoint->isEndPoint()) {
                key->setEndEnabled(false);
            } else if(mCurrentPoint->isStartPoint()) {
                key->setStartEnabled(false);
            }
    } else {
        foreach(QrealKey *key, mSelectedKeys) {
            mAnimator->removeKey(key);
            key->setSelected(false);
            key->decNumberPointers();
        }
        mSelectedKeys.clear();
        mAnimator->sortKeys();
        mAnimator->updateKeysPath();
    }
}

void AnimationWidget::clearKeysSelection() {
    foreach(QrealKey *key, mSelectedKeys) {
        key->setSelected(false);
        key->decNumberPointers();
    }

    mSelectedKeys.clear();
}

void AnimationWidget::addKeyToSelection(QrealKey *key)
{
    if(key->isSelected()) return;
    key->setSelected(true);
    mSelectedKeys << key;
    key->incNumberPointers();
}

void AnimationWidget::removeKeyFromSelection(QrealKey *key)
{
    if(key->isSelected()) {
        key->setSelected(false);
        if(mSelectedKeys.removeOne(key) ) {
            key->decNumberPointers();
        }
    }
}

void AnimationWidget::mouseMove(QPointF mousePos)
{
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
        getValueAndFrameFromPos(mousePos, &value, &frame);
        if(mCurrentPoint->isKeyPoint()) {
            QPointF currentFrameAndValue = QPointF(frame, value);
            if(mFirstMove) {
                foreach(QrealKey *key, mSelectedKeys) {
                    key->saveCurrentFrameAndValue();
                    key->changeFrameAndValueBy(currentFrameAndValue -
                                               mPressFrameAndValue);
                }
            } else {
                foreach(QrealKey *key, mSelectedKeys) {
                    key->changeFrameAndValueBy(currentFrameAndValue -
                                               mPressFrameAndValue);
                }
            }
            mAnimator->sortKeys();
        } else {
            qreal clampedFrame = clamp(frame, mMinMoveFrame, mMaxMoveFrame);
            mCurrentPoint->moveTo(clampedFrame, mAnimator->clampValue(value));
        }
        mAnimator->updateKeysPath();
    }
    mFirstMove = false;
}

void AnimationWidget::mousePressEvent(QMouseEvent *event)
{
    if(mAnimator == NULL) return;
    if(event->button() == Qt::RightButton) {
        qreal value;
        qreal frame;
        getValueAndFrameFromPos(event->pos(), &value, &frame);
        QrealPoint *point = mAnimator->getPointAt(value, frame,
                                            mPixelsPerFrame, mPixelsPerValUnit);
        if(point == NULL) return;
        QrealPointValueDialog *dialog = new QrealPointValueDialog(point, this);
        dialog->show();
        connect(dialog, SIGNAL(repaintWithBoxesListSignal()),
                this, SLOT(updateDrawPath()) );
        connect(dialog, SIGNAL(finished(int)),
                this, SLOT(mergeKeysIfNeeded()) );
    } else if(event->button() == Qt::MiddleButton) {
        middlePress(event->pos());
    } else {
        mousePress(event->pos());
    }
    repaintWithBoxesList();
}

void AnimationWidget::mouseMoveEvent(QMouseEvent *event) {
    if(mAnimator == NULL) return;
    if(event->buttons() == Qt::MiddleButton) {
        middleMove(event->pos());
        emit changedViewedFrames(mStartFrame,
                                 mEndFrame);
    } else {
        mouseMove(event->pos());
    }
    repaintWithBoxesList();
    mMainWindow->callUpdateSchedulers();
}

void AnimationWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(mAnimator == NULL) return;
    if(event->button() == Qt::MiddleButton) {
        middleRelease();
    } else {
        mouseRelease();
    }
    repaintWithBoxesList();
}

void AnimationWidget::wheelEvent(QWheelEvent *event)
{
    if(mAnimator == NULL) return;
    if(isCtrlPressed()) {
        if(event->delta() > 0) {
            incScale(0.1);
        } else {
            incScale(-0.1);
        }
    } else {
        if(event->delta() > 0) {
            incMinShownVal(0.1);
        } else {
            incMinShownVal(-0.1);
        }
    }
    repaintWithBoxesList();
}

bool AnimationWidget::processFilteredKeyEvent(QKeyEvent *event)
{
    if(!hasFocus() ) return false;
    if(event->key() == Qt::Key_Delete && mAnimator != NULL) {
        deletePressed();
        repaintWithBoxesList();
    } else {
        return false;
    }
    return true;
}

void AnimationWidget::setAnimator(QrealAnimator *animator)
{
    clearKeysSelection();
    mAnimator = animator;
    setScale(1.);
    repaintWithBoxesList();
}

void AnimationWidget::setBoxesList(BoxesList *boxesList)
{
    mBoxesList = boxesList;
}

void AnimationWidget::repaintWithBoxesList()
{
    if(mAnimator != NULL) {
        mAnimator->updateDrawPathIfNeeded(height(), mMargin,
                                          mStartFrame, mMinShownVal,
                                          mPixelsPerFrame, mPixelsPerValUnit);
    }
    repaint();
    if(mBoxesList == NULL) return;
    mBoxesList->repaint();
}

void AnimationWidget::updateDrawPath()
{
    if(mAnimator == NULL) return;
    mAnimator->sortKeys();
    mAnimator->updateKeysPath();
}

void AnimationWidget::mergeKeysIfNeeded() {
    if(mAnimator == NULL) return;
    mAnimator->mergeKeysIfNeeded();
}
