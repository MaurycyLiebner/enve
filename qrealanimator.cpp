#include "qrealanimator.h"
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include "mainwindow.h"

qreal clamp(qreal val, qreal min, qreal max) {
    if(val > max) return max;
    if(val < min) return min;
    return val;
}


QrealAnimator::QrealAnimator()
{

}

qreal QrealAnimator::getValueAtFrame(int frame)
{
    int prevId;
    int nextId;
    if(getNextAndPreviousKeyId(&prevId, &nextId, frame) ) {
        if(nextId == prevId) return mKeys.at(nextId)->getValue();
        QrealKey *prevKey = mKeys.at(prevId);
        QrealKey *nextKey = mKeys.at(nextId);
        qreal prevMultiplier;
        qreal nextMultiplier;
        getKeyMultiplicatorsAtFrame(frame, prevKey, nextKey,
                                    &prevMultiplier, &nextMultiplier);
        return prevKey->getValue()*prevMultiplier +
               nextKey->getValue()*nextMultiplier;
    } else {
        return mCurrentValue;
    }
}

qreal QrealAnimator::getCurrentValue()
{
    return mCurrentValue;
}

void QrealAnimator::setCurrentValue(qreal newValue)
{
    mCurrentValue = newValue;
}

void QrealAnimator::updateValueFromCurrentFrame()
{
    mCurrentValue = getValueAtFrame(mCurrentFrame);
}

QrealKey *QrealAnimator::addNewKeyAtFrame(int frame)
{
    QrealKey *newKey = new QrealKey(frame);
    appendKey(newKey);
    return newKey;
}

void QrealAnimator::saveCurrentValueToKey(QrealKey *key)
{
    saveValueToKey(key, mCurrentValue);
}

void QrealAnimator::saveValueToKey(QrealKey *key, qreal value)
{
    key->setValue(value);
    updateKeysPath();
    updateValueFromCurrentFrame();
}

void QrealAnimator::appendKey(QrealKey *newKey) {
    mKeys.append(newKey);
    sortKeys();
}

void QrealAnimator::removeKey(QrealKey *removeKey) {
    mKeys.removeOne(removeKey);
    sortKeys();
}

void QrealAnimator::setFrame(int frame)
{
    mCurrentFrame = frame;
    updateValueFromCurrentFrame();
}

bool QrealAnimator::getNextAndPreviousKeyId(int *prevIdP, int *nextIdP,
                                            int frame) {
    if(mKeys.isEmpty()) return false;
    int minId = 0;
    int maxId = mKeys.count() - 1;
    if(frame > mKeys.last()->getFrame()) {
        *prevIdP = maxId;
        *nextIdP = maxId;
        return true;
    }
    if(frame < mKeys.first()->getFrame()) {
        *prevIdP = minId;
        *nextIdP = minId;
        return true;
    }
    while(maxId - minId > 1) {
        int guess = (maxId + minId)/2;
        if(mKeys.at(guess)->getFrame() > frame) {
            maxId = guess;
        } else {
            minId = guess;
        }
    }

    if(minId == maxId) {
        QrealKey *key = mKeys.at(minId);
        if(key->getFrame() > frame) {
            if(minId != 0) {
                minId = minId - 1;
            }
        } else if(key->getFrame() < frame) {
            if(minId < mKeys.count() - 1) {
                maxId = minId + 1;
            }
        }
    }
    *prevIdP = minId;
    *nextIdP = maxId;
    return true;
}

QrealKey *QrealAnimator::getKeyAtFrame(int frame)
{
    if(mKeys.isEmpty()) return NULL;
    if(frame > mKeys.last()->getFrame()) return NULL;
    if(frame < mKeys.first()->getFrame()) return NULL;
    int minId = 0;
    int maxId = mKeys.count() - 1;
    while(maxId - minId > 1) {
        int guess = (maxId + minId)/2;
        if(mKeys.at(guess)->getFrame() > frame) {
            maxId = guess;
        } else {
            minId = guess;
        }
    }
    QrealKey *minIdKey = mKeys.at(minId);
    if(minIdKey->getFrame() == frame) return minIdKey;
    QrealKey *maxIdKey = mKeys.at(maxId);
    if(maxIdKey->getFrame() == frame) return maxIdKey;
    return NULL;
}

void QrealAnimator::saveCurrentValueAsKey()
{
    QrealKey *keyAtFrame = getKeyAtFrame(mCurrentFrame);
    if(keyAtFrame == NULL) {
        keyAtFrame = addNewKeyAtFrame(mCurrentFrame);
    }
    saveCurrentValueToKey(keyAtFrame);
}

void QrealAnimator::updateKeysPath()
{
    mKeysPath = QPainterPath();
    QrealKey *lastKey = NULL;
    foreach(QrealKey *key, mKeys) {
        int keyFrame = key->getFrame();
        if(lastKey == NULL) {
            mKeysPath.moveTo(0, -key->getValue());
            mKeysPath.lineTo(keyFrame, -key->getValue());
        } else {
            mKeysPath.cubicTo(
                        QPointF(lastKey->getEndValueFrame(),
                                -lastKey->getEndValue()),
                        QPointF(key->getStartValueFrame(),
                                -key->getStartValue()),
                        QPointF(keyFrame, -key->getValue()));
        }
        lastKey = key;
    }
    if(lastKey != NULL) {
        mKeysPath.lineTo(5000, -lastKey->getValue());
    }
    updateDrawPath();
}

bool keysFrameSort(QrealKey *key1, QrealKey *key2)
{
    return key1->getFrame() < key2->getFrame();
}

void QrealAnimator::sortKeys()
{
    qSort(mKeys.begin(), mKeys.end(), keysFrameSort);
}

qreal calcCubicBezierVal(qreal p0, qreal p1, qreal p2, qreal p3, qreal t) {
    return pow(1 - t, 3)*p0 +
            3*pow(1 - t, 2)*t*p1 +
            3*(1 - t)*t*t*p2 +
            t*t*t*p3;
}

qreal tFromX(qreal p0x, qreal p1x, qreal p2x, qreal p3x, qreal x) {
    qreal minT = 0.;
    qreal maxT = 1.;
    qreal xGuess;
    do {
        qreal guessT = (maxT + minT)*0.5;
        xGuess = calcCubicBezierVal(p0x, p1x, p2x, p3x, guessT);
        if(xGuess > x) {
            maxT = guessT;
        } else {
            minT = guessT;
        }
    } while(qAbs(xGuess - x) > 0.0001);
    return (maxT + minT)*0.5;
}

void QrealAnimator::getKeyMultiplicatorsAtFrame(int frame,
                                                QrealKey *prevKey,
                                                QrealKey *nextKey,
                                                qreal *prevMultiplicator,
                                                qreal *nextMultiplicator)
{
    qreal t = tFromX(prevKey->getFrame(),
                     prevKey->getEndValueFrame(),
                     nextKey->getStartValueFrame(),
                     nextKey->getFrame(), frame);
    qreal p0y = prevKey->getValue();
    qreal p1y = prevKey->getEndValue();
    qreal p2y = nextKey->getStartValue();
    qreal p3y = nextKey->getValue();
    qreal multiplicator = calcCubicBezierVal(p0y, p1y, p2y, p3y, t);
    *nextMultiplicator = multiplicator;
    *prevMultiplicator = 1 - multiplicator;
}

void QrealAnimator::getMinAndMaxValues(qreal *minValP, qreal *maxValP) {
    qreal minVal = 100000.;
    qreal maxVal = -100000.;
    foreach(QrealKey *key, mKeys) {
        qreal keyVal = key->getValue();
        qreal startVal = key->getStartValue();
        qreal endVal = key->getEndValue();
        qreal maxKeyVal = qMax(qMax(startVal, endVal), keyVal);
        qreal minKeyVal = qMin(qMin(startVal, endVal), keyVal);
        if(maxKeyVal > maxVal) maxVal = maxKeyVal;
        if(minKeyVal < minVal) minVal = minKeyVal;
    }

    *minValP = minVal;
    *maxValP = maxVal;
}

void QrealAnimator::getMinAndMaxValuesBetweenFrames(
        int startFrame, int endFrame,
        qreal *minValP, qreal *maxValP) {
    qreal minVal = 100000.;
    qreal maxVal = -100000.;
    foreach(QrealKey *key, mKeys) {
        int keyFrame = key->getFrame();
        if(keyFrame > endFrame || keyFrame < startFrame) continue;
        qreal keyVal = key->getValue();
        qreal startVal = key->getStartValue();
        qreal endVal = key->getEndValue();
        qreal maxKeyVal = qMax(qMax(startVal, endVal), keyVal);
        qreal minKeyVal = qMin(qMin(startVal, endVal), keyVal);
        if(maxKeyVal > maxVal) maxVal = maxKeyVal;
        if(minKeyVal < minVal) minVal = minKeyVal;
    }

    *minValP = minVal;
    *maxValP = maxVal;
}

void QrealAnimator::updateDrawPath() {
    QMatrix transform;
    transform.translate(-mPixelsPerFrame*mStartFrame,
                mDrawRect.height() + mPixelsPerValUnit*mMinVal - mMargin);
    transform.scale(mPixelsPerFrame, mPixelsPerValUnit);
    mKeysDrawPath = mKeysPath*transform;
}

void QrealAnimator::incMargin(qreal inc) {
    qreal newMargin = mMargin + inc;
    setMargin(clamp(newMargin, -mDrawRect.height(),
                    mDrawRect.height()*0.25));
}

void QrealAnimator::setMargin(qreal margin) {
    mMargin = margin;
    updateDimensions();
}

void QrealAnimator::updateDimensions() {
    getMinAndMaxValues(&mMinVal, &mMaxVal);
    mPixelsPerValUnit = (mDrawRect.height() - 2*mMargin)/
                                (mMaxVal - mMinVal);
    int dFrame = mEndFrame - mStartFrame;
    mPixelsPerFrame = mDrawRect.width()/dFrame;
    updateDrawPath();
}

void QrealAnimator::draw(QPainter *p)
{
    p->setBrush(Qt::NoBrush);

    p->setPen(QPen(QColor(0, 0, 0, 75), 1.));
    qreal xL = 0.;
    qreal maxX = mDrawRect.width();
    qreal inc = mPixelsPerFrame;
    while(inc < 10.) inc = mPixelsPerFrame*10;
    while(xL < maxX) {
        p->drawLine(xL, 0, xL, mDrawRect.height());
        xL += inc;
    }

    p->setPen(QPen(Qt::black, 1.));
    p->drawPath(mKeysDrawPath);

    p->save();
    p->translate(0., mDrawRect.height() - mMargin);
    p->setBrush(Qt::black);
    foreach(QrealKey *key, mKeys) {
        key->draw(p,
                  mStartFrame, mMinVal,
                  mPixelsPerFrame, mPixelsPerValUnit);
    }
    p->restore();

    if(mSelecting) {
        p->setBrush(Qt::NoBrush);
        p->setPen(QPen(Qt::blue, 1.f, Qt::DotLine));
        p->drawRect(mSelectionRect);
    }
}

void QrealAnimator::getValueAndFrameFromPos(QPointF pos,
                                            qreal *value, qreal *frame) {
    *value = (mDrawRect.height() - pos.y() - mMargin)/mPixelsPerValUnit
            + mMinVal;
    *frame = mStartFrame + pos.x()/mPixelsPerFrame;
}

void QrealAnimator::updateMinAndMaxMove(QrealKey *key) {
    int keyId = mKeys.indexOf(key);
    if(mCurrentPoint->isEndPoint()) {
        mMinMoveFrame = key->getFrame();
        if(keyId == mKeys.count() - 1) {
            mMaxMoveFrame = 5000.;
        } else {
            mMaxMoveFrame = mKeys.at(keyId + 1)->getFrame() - 1;
        }
    } else if(mCurrentPoint->isStartPoint()) {
        if(keyId == 0) {
            mMinMoveFrame = 0.;
        } else {
            mMinMoveFrame = mKeys.at(keyId - 1)->getFrame() + 1;
        }
        mMaxMoveFrame = key->getFrame();
    }
}

void QrealAnimator::mousePress(QPointF pressPos) {
    mFirstMove = true;
    qreal value;
    qreal frame;
    getValueAndFrameFromPos(pressPos, &value, &frame);
    QrealKey *addedKey = NULL;
    foreach(QrealKey *key, mKeys) {
        mCurrentPoint = key->mousePress(frame, value,
                                        mPixelsPerFrame, mPixelsPerValUnit);
        if(mCurrentPoint != NULL) {
            if(!mCurrentPoint->isKeyPoint()) {
                updateMinAndMaxMove(key);
            }
            addedKey = key;
            break;
        }
    }
    if(mCurrentPoint == NULL) {
        mSelecting = true;
        mSelectionRect.setTopLeft(pressPos);
        mSelectionRect.setBottomRight(pressPos);
    } else if(mCurrentPoint->isKeyPoint()) {
        mPressFrameAndValue = QPointF(frame, value);
        if(isShiftPressed()) {
            if(addedKey->isSelected()) {
                removeKeyFromSelection(addedKey);
            } else {
                addKeyToSelection(addedKey);
            }
        } else {
            if(!addedKey->isSelected()) {
                clearKeysSelection();
                addKeyToSelection(addedKey);
            }
        }
    } else {
        mCurrentPoint->setSelected(true);
        clearKeysSelection();
    }
}

void QrealAnimator::mouseMove(QPointF mousePos)
{
    if(mSelecting) {
        mSelectionRect.setBottomRight(mousePos);
    } else if(mCurrentPoint != NULL) {
        qreal value;
        qreal frame;
        getValueAndFrameFromPos(mousePos, &value, &frame);
        if(mSelectedKeys.isEmpty()) {

            if(mCurrentPoint->isKeyPoint()) {
                mCurrentPoint->setFrame(frame);
                sortKeys();
            } else {
                qreal clampedFrame = clamp(frame, mMinMoveFrame, mMaxMoveFrame);
                mCurrentPoint->setFrame(clampedFrame);
            }
            mCurrentPoint->setValue(value);
        } else {
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
            sortKeys();
        }
        updateKeysPath();
    }
    mFirstMove = false;
}

void QrealAnimator::mergeKeysIfNeeded() {
    QrealKey *lastKey = NULL;
    foreach(QrealKey *key, mKeys) {
        if(lastKey != NULL) {
            if(key->getFrame() == lastKey->getFrame() ) {
                if(key->isSelected()) {
                    mKeys.removeOne(lastKey);
                    delete lastKey;
                } else {
                    mKeys.removeOne(key);
                    delete key;
                    key = NULL;
                }
            }
        }
        lastKey = key;
    }
    updateKeysPath();
}

void QrealAnimator::clearKeysSelection() {
    foreach(QrealKey *key, mKeys) {
        key->setSelected(false);
    }

    mSelectedKeys.clear();
}

void QrealAnimator::addKeyToSelection(QrealKey *key)
{
    if(key->isSelected()) return;
    key->setSelected(true);
    mSelectedKeys << key;
}

void QrealAnimator::removeKeyFromSelection(QrealKey *key)
{
    if(key->isSelected()) {
        key->setSelected(false);
        mSelectedKeys.removeOne(key);
    }
}

void QrealAnimator::setViewedFramesRange(int minF, int maxF)
{
    mStartFrame = minF;
    mEndFrame = maxF;
    updateDimensions();
}

void QrealAnimator::setRect(QRectF rect)
{
    mDrawRect = rect;
    updateDimensions();
}

void QrealAnimator::addKeysInRectToSelection(QRectF rect) {
    qreal topValue;
    qreal leftFrame;
    getValueAndFrameFromPos(rect.topLeft(), &topValue, &leftFrame);
    qreal bottomValue;
    qreal rightFrame;
    getValueAndFrameFromPos(rect.bottomRight(), &bottomValue, &rightFrame);
    QRectF frameValueRect;
    frameValueRect.setTopLeft(QPointF(leftFrame, topValue) );
    frameValueRect.setBottomRight(QPointF(rightFrame, bottomValue) );
    foreach(QrealKey *key, mKeys) {
        if(key->isInsideRect(frameValueRect)) {
            mSelectedKeys << key;
            key->setSelected(true);
        }
    }
}

void QrealAnimator::mouseRelease()
{
    if(mSelecting) {
        if(!isShiftPressed()) {
            clearKeysSelection();
        }
        addKeysInRectToSelection(mSelectionRect);
        mSelecting = false;
    } else if(!mSelectedKeys.isEmpty()) {
        if(mFirstMove && mCurrentPoint != NULL) {
            if(!isShiftPressed()) {
                clearKeysSelection();
                addKeyToSelection(mCurrentPoint->getParentKey());
            }
        }
        mergeKeysIfNeeded();
    } else if(mCurrentPoint != NULL) {
        mergeKeysIfNeeded();
        mCurrentPoint->setSelected(false);
        mCurrentPoint = NULL;
    }
}

QrealKey::QrealKey(int frame) {
    mFrame = frame;
    mKeyPoint = new QrealPoint(KEY_POINT, this);
    mStartPoint = new QrealPoint(START_POINT, this);
    mEndPoint = new QrealPoint(END_POINT, this);
}

QrealPoint *QrealKey::mousePress(qreal frameT, qreal valueT,
                          qreal pixelsPerFrame, qreal pixelsPerValue)
{
    if(mKeyPoint->isNear(frameT, valueT, pixelsPerFrame, pixelsPerValue)) {
        return mKeyPoint;
    }
    if(mStartPoint->isNear(frameT, valueT, pixelsPerFrame, pixelsPerValue)) {
        return mStartPoint;
    }
    if(mEndPoint->isNear(frameT, valueT, pixelsPerFrame, pixelsPerValue)) {
        return mEndPoint;
    }
    return NULL;
}

bool QrealKey::isSelected() { return mKeyPoint->isSelected(); }

qreal QrealKey::getValue() { return mValue; }

void QrealKey::setValue(qreal value) { mValue = value; }

void QrealKey::setStartValue(qreal value)
{
    mStartValue = value;
}

void QrealKey::setEndValue(qreal value)
{
    mEndValue = value;
}

int QrealKey::getFrame() { return mFrame; }

void QrealKey::setFrame(int frame) { mFrame = frame; }

void QrealKey::setStartFrame(qreal startFrame)
{
    mStartFrame = startFrame;
}

void QrealKey::setEndFrame(qreal endFrame)
{
    mEndFrame = endFrame;
}

qreal QrealKey::getStartValue() {
    if(mStartEnabled) return mStartValue;
    return mValue;
}

qreal QrealKey::getEndValue() {
    if(mEndEnabled) return mEndValue;
    return mValue;
}

qreal QrealKey::getStartValueFrame() {
    if(mStartEnabled) return mStartFrame;
    return mFrame;
}

qreal QrealKey::getEndValueFrame() {
    if(mEndEnabled) return mEndFrame;
    return mFrame;
}

void QrealKey::setStartEnabled(bool bT) {
    mStartEnabled = bT;
}

void QrealKey::setEndEnabled(bool bT) {
    mEndEnabled = bT;
}

void QrealKey::setSelected(bool bT)
{
    mKeyPoint->setSelected(bT);
}

bool QrealKey::isInsideRect(QRectF valueFrameRect)
{
    QPointF keyPoint = QPointF(mKeyPoint->getFrame(), mKeyPoint->getValue());
    return valueFrameRect.contains(keyPoint);
}

void QrealKey::draw(QPainter *p,
                    qreal minFrameT, qreal minValueT,
                    qreal pixelsPerFrame, qreal pixelsPerValue)
{
    mKeyPoint->draw(p, minFrameT, minValueT, pixelsPerFrame, pixelsPerValue);
    if(mStartEnabled) {
        mStartPoint->draw(p, minFrameT, minValueT, pixelsPerFrame, pixelsPerValue);
    }
    if(mEndEnabled) {
        mEndPoint->draw(p, minFrameT, minValueT, pixelsPerFrame, pixelsPerValue);
    }
}

void QrealKey::saveCurrentFrameAndValue() {
    mSavedFrame = getFrame();
    mSavedValue = getValue();
}

void QrealKey::changeFrameAndValueBy(QPointF frameValueChange)
{
    setValue(frameValueChange.y() + mSavedValue);
    setFrame(qRound(frameValueChange.x() + mSavedFrame) );
}

QrealPoint::QrealPoint(QrealPointType type, QrealKey *parentKey) {
    mType = type;
    mParentKey = parentKey;
}

qreal QrealPoint::getFrame() {
    if(mType == KEY_POINT) return mParentKey->getFrame();
    if(mType == START_POINT) return mParentKey->getStartValueFrame();
    if(mType == END_POINT) return mParentKey->getEndValueFrame();
}

void QrealPoint::setFrame(qreal frame)
{
    if(mType == KEY_POINT) return mParentKey->setFrame(qRound(frame));
    if(mType == START_POINT) return mParentKey->setStartFrame(frame);
    if(mType == END_POINT) return mParentKey->setEndFrame(frame);
}

qreal QrealPoint::getValue() {
    if(mType == KEY_POINT) return mParentKey->getValue();
    if(mType == START_POINT) return mParentKey->getStartValue();
    if(mType == END_POINT) return mParentKey->getEndValue();
}

void QrealPoint::setValue(qreal value)
{
    if(mType == KEY_POINT) return mParentKey->setValue(value);
    if(mType == START_POINT) return mParentKey->setStartValue(value);
    if(mType == END_POINT) return mParentKey->setEndValue(value);
}

bool QrealPoint::isSelected() { return mIsSelected; }

bool QrealPoint::isNear(qreal frameT, qreal valueT,
                        qreal pixelsPerFrame, qreal pixelsPerValue) {
    qreal value = getValue();
    qreal frame = getFrame();
    if(qAbs(frameT - frame)*pixelsPerFrame > mRadius) return false;
    if(qAbs(valueT - value)*pixelsPerValue > mRadius) return false;
    return true;
}

void QrealPoint::moveTo(qreal frameT, qreal valueT)
{
    setFrame(frameT);
    setValue(valueT);
}

void QrealPoint::draw(QPainter *p,
                      qreal minFrameT, qreal minValueT,
                      qreal pixelsPerFrame, qreal pixelsPerValue) {
    if(mIsSelected) {
        p->setBrush(Qt::red);
    } else {
        p->setBrush(Qt::black);
    }
    p->drawEllipse(QPointF((getFrame() - minFrameT)*pixelsPerFrame,
                           (minValueT - getValue())*pixelsPerValue),
                           mRadius, mRadius);
}

void QrealPoint::setSelected(bool bT) {
    mIsSelected = bT;
}

bool QrealPoint::isKeyPoint() { return mType == KEY_POINT; }

bool QrealPoint::isStartPoint() { return mType == START_POINT; }

bool QrealPoint::isEndPoint() { return mType == END_POINT; }

QrealKey *QrealPoint::getParentKey()
{
    return mParentKey;
}
