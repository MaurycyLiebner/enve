#include "qrealanimator.h"
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include "mainwindow.h"
#include "animationdockwidget.h"


QrealAnimator::QrealAnimator()
{
}

QrealAnimator::~QrealAnimator()
{
    if(mUpdater != NULL) {
        delete mUpdater;
        setUpdater(NULL);
    }
}

void QrealAnimator::getKeysInRect(QRectF selectionRect,
                                  int minViewedFrame,
                                  qreal pixelsPerFrame,
                                  QList<QrealKey*> *keysList) {
    int keyRectFramesSpan = KEY_RECT_SIZE/pixelsPerFrame;
    int selLeftFrame = selectionRect.left()/pixelsPerFrame + minViewedFrame -
            keyRectFramesSpan;
    int selRightFrame = selectionRect.right()/pixelsPerFrame + minViewedFrame -
            keyRectFramesSpan;
    for(int i = selRightFrame; i >= selLeftFrame; i--) {
        QrealKey *keyAtPos = getKeyAtFrame(i);
        if(keyAtPos != NULL) {
            keysList->append(keyAtPos);
        }
    }
}

void QrealAnimator::addAllKeysToComplexAnimator()
{
    if(mParentAnimator == NULL) return;
    foreach(QrealKey *key, mKeys) {
        mParentAnimator->addChildQrealKey(key);
    }
}

void QrealAnimator::removeAllKeysFromComplexAnimator()
{
    if(mParentAnimator == NULL) return;
    foreach(QrealKey *key, mKeys) {
        mParentAnimator->removeChildQrealKey(key);
    }
}

bool QrealAnimator::hasKeys()
{
    return !mKeys.isEmpty();
}

void QrealAnimator::incAllValues(qreal valInc)
{
    foreach(QrealKey *key, mKeys) {
        key->incValue(valInc);
    }
    incCurrentValue(valInc);
}

QString QrealAnimator::getName()
{
    return mName;
}

void QrealAnimator::setName(QString newName)
{
    mName = newName;
}

QrealKey *QrealAnimator::getKeyAtPos(qreal relX,
                                     int minViewedFrame,
                                     qreal pixelsPerFrame) {
    qreal relFrame = relX/pixelsPerFrame;
    qreal pressFrame = relFrame + minViewedFrame;
    if(pixelsPerFrame > KEY_RECT_SIZE) {
        int relFrameInt = relFrame;
        if( qAbs((relFrameInt + 0.5)*pixelsPerFrame - relX) > KEY_RECT_SIZE*0.5) {
            return NULL;
        }
    }
    qreal keyRectFramesSpan = 0.5*KEY_RECT_SIZE/pixelsPerFrame;
    int minPossibleKey = pressFrame - keyRectFramesSpan;
    int maxPossibleKey = pressFrame + keyRectFramesSpan;
    QrealKey *keyAtPos = NULL;
    for(int i = maxPossibleKey; i >= minPossibleKey; i--) {
        keyAtPos = getKeyAtFrame(i);
        if(keyAtPos != NULL) {
            return keyAtPos;
        }
    }
    return NULL;
}

void QrealAnimator::setParentAnimator(ComplexAnimator *parentAnimator)
{
    mParentAnimator = parentAnimator;
}

qreal QrealAnimator::getValueAtFrame(int frame)
{
    int prevId;
    int nextId;
    if(getNextAndPreviousKeyId(&prevId, &nextId, frame) ) {
        if(nextId == prevId) return mKeys.at(nextId)->getValue();
        QrealKey *prevKey = mKeys.at(prevId);
        QrealKey *nextKey = mKeys.at(nextId);
        return getValueAtFrame(frame, prevKey, nextKey);
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
    callUpdater();
}

void QrealAnimator::updateValueFromCurrentFrame()
{
    setCurrentValue(getValueAtFrame(mCurrentFrame) );
}

QrealKey *QrealAnimator::addNewKeyAtFrame(int frame)
{
    QrealKey *newKey = new QrealKey(frame, this);
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
}

void QrealAnimator::appendKey(QrealKey *newKey) {
    mKeys.append(newKey);
    if(mConnectedToMainWindow != NULL) {
        mConnectedToMainWindow->scheduleBoxesListRepaint();
    }
    if(mParentAnimator != NULL) {
        mParentAnimator->addChildQrealKey(newKey);
    }
    sortKeys();
}

void QrealAnimator::removeKey(QrealKey *removeKey) {
    mKeys.removeOne(removeKey);
    if(mParentAnimator != NULL) {
        mParentAnimator->removeChildQrealKey(removeKey);
    }
    sortKeys();
}

void QrealAnimator::moveKeyToFrame(QrealKey *key, int newFrame)
{
    if(mParentAnimator != NULL) {
        mParentAnimator->removeChildQrealKey(key);
    }
    key->setFrame(newFrame);
    if(mParentAnimator != NULL) {
        mParentAnimator->addChildQrealKey(key);
    }
    sortKeys();
    updateKeysPath();
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
    if(frame >= mKeys.last()->getFrame()) {
        *prevIdP = maxId;
        *nextIdP = maxId;
        return true;
    }
    if(frame <= mKeys.first()->getFrame()) {
        *prevIdP = minId;
        *nextIdP = minId;
        return true;
    }
    while(maxId - minId > 1) {
        int guess = (maxId + minId)/2;
        int keyFrame = mKeys.at(guess)->getFrame();
        if(keyFrame > frame) {
            maxId = guess;
        } else if(keyFrame < frame) {
            minId = guess;
        } else {
            *nextIdP = guess;
            *prevIdP = guess;
            return true;
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
            mKeysPath.moveTo(-5000, -key->getValue());
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
    } else {
        mKeysPath.moveTo(0, -mCurrentValue);
        mKeysPath.lineTo(5000, -mCurrentValue);
    }
    updateValueFromCurrentFrame();
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
    qreal guessT;
    do {
        guessT = (maxT + minT)*0.5;
        xGuess = calcCubicBezierVal(p0x, p1x, p2x, p3x, guessT);
        if(xGuess > x) {
            maxT = guessT;
        } else {
            minT = guessT;
        }
    } while(qAbs(xGuess - x) > 0.0001);
    return guessT;
}

qreal QrealAnimator::getValueAtFrame(int frame,
                                    QrealKey *prevKey,
                                    QrealKey *nextKey)
{
    qreal t = tFromX(prevKey->getFrame(),
                     prevKey->getEndValueFrame(),
                     nextKey->getStartValueFrame(),
                     nextKey->getFrame(), frame);
    qreal p0y = prevKey->getValue();
    qreal p1y = prevKey->getEndValue();
    qreal p2y = nextKey->getStartValue();
    qreal p3y = nextKey->getValue();
    return calcCubicBezierVal(p0y, p1y, p2y, p3y, t);
}

void QrealAnimator::middlePress(QPointF pressPos)
{
    mSavedStartFrame = mStartFrame;
    mSavedEndFrame = mEndFrame;
    mSavedMinShownValue = mMinShownVal;
    mMiddlePressPos = pressPos;
}

void QrealAnimator::middleMove(QPointF movePos)
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

void QrealAnimator::middleRelease()
{

}

int QrealAnimator::getStartFrame()
{
    return mStartFrame;
}

int QrealAnimator::getEndFrame()
{
    return mEndFrame;
}

void QrealAnimator::getMinAndMaxValues(qreal *minValP, qreal *maxValP) {
    qreal minVal = 100000.;
    qreal maxVal = -100000.;
    if(mKeys.isEmpty()) {
        *minValP = mCurrentValue;
        *maxValP = mCurrentValue;
    } else {
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
                mDrawRect.height() + mPixelsPerValUnit*mMinShownVal - mMargin);
    transform.scale(mPixelsPerFrame, mPixelsPerValUnit);
    mKeysDrawPath = mKeysPath*transform;
}

void QrealAnimator::incScale(qreal inc) {
    qreal newScale = mValueScale + inc;
    setScale(clamp(newScale, 0.1, 10.));
}

void QrealAnimator::setScale(qreal scale) {
    mValueScale = scale;
    updateDimensions();
}

void QrealAnimator::updateDimensions() {
    getMinAndMaxValues(&mMinVal, &mMaxVal);
    if(qAbs(mMinVal - mMaxVal) < 0.1 ) {
        mMinVal -= 0.05;
        mMaxVal += 0.05;
    }
    mPixelsPerValUnit = mValueScale*(mDrawRect.height() - 2*mMargin)/
                                (mMaxVal - mMinVal);
    incMinShownVal(0.);
    int dFrame = mEndFrame - mStartFrame;
    mPixelsPerFrame = mDrawRect.width()/dFrame;
    updateDrawPath();
}

void QrealAnimator::draw(QPainter *p)
{
    p->fillRect(mDrawRect, QColor(150, 150, 150));
    p->setBrush(Qt::NoBrush);

    p->fillRect(0, 0, mDrawRect.width(), 20, Qt::white);
    p->fillRect(0, 0, 40, mDrawRect.height(), Qt::white);

    qreal maxX = mDrawRect.width();
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
            p->drawLine(xL, 20, xL, mDrawRect.height());
            xL += inc;
            currFrame += incFrame;
        }
        currAlpha *= 1.5;
        lineWidth *= 1.5;
    }

    p->setPen(QPen(Qt::green, 2.f));
    qreal xL = (mCurrentFrame - mStartFrame)*mPixelsPerFrame;
    /*p->drawText(QRectF(xL - 20, 0, 40, 20),
                Qt::AlignCenter, QString::number(mCurrentFrame));*/
    p->drawLine(xL, 20, xL, mDrawRect.height());


    lineWidth = 2.;
    qreal incValue = 1000.;
    bool by2 = true;
    currAlpha = 255;
    while(mPixelsPerValUnit*incValue > 50.) {
        qreal yL = mDrawRect.height() +
                fmod(mMinShownVal, incValue)*mPixelsPerValUnit - mMargin;
        qreal incY = incValue*mPixelsPerValUnit;
        if( yL > 20 && (yL < mDrawRect.height() || yL - incY > 20) ) {
            p->setPen(QPen(QColor(0, 0, 0, currAlpha), lineWidth));
            qreal currValue = mMinShownVal - fmod(mMinShownVal, incValue);
            while(yL > 20) {
                p->drawText(QRectF(0, yL - incY, 40, 2*incY),
                            Qt::AlignCenter, QString::number(currValue));
                p->drawLine(40, yL, mDrawRect.width(), yL);
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

    p->setClipRect(40, 20, mDrawRect.width() - 20, mDrawRect.height() - 20);

    p->setPen(QPen(Qt::black, 4.));
    p->drawPath(mKeysDrawPath);
    p->setPen(QPen(Qt::red, 2.));
    p->drawPath(mKeysDrawPath);
    p->setPen(QPen(Qt::black, 2.));

    p->save();
    p->translate(0., mDrawRect.height() - mMargin);
    p->setBrush(Qt::black);
    foreach(QrealKey *key, mKeys) {
        key->draw(p,
                  mStartFrame, mMinShownVal,
                  mPixelsPerFrame, mPixelsPerValUnit);
    }
    p->restore();

    if(mSelecting) {
        p->setBrush(Qt::NoBrush);
        p->setPen(QPen(Qt::blue, 2.f, Qt::DotLine));
        p->drawRect(mSelectionRect);
    }
}

void QrealAnimator::incMinShownVal(qreal inc) {
    setMinShownVal(inc*(mMaxVal - mMinVal) + mMinShownVal);
}

void QrealAnimator::setMinShownVal(qreal newMinShownVal) {
    qreal halfHeightVal = (mDrawRect.height() - 2*mMargin)*0.5/mPixelsPerValUnit;
    mMinShownVal = clamp(newMinShownVal,
                         mMinVal - halfHeightVal,
                         mMaxVal - halfHeightVal);
    updateDrawPath();
}

void QrealAnimator::getValueAndFrameFromPos(QPointF pos,
                                            qreal *value, qreal *frame) {
    *value = (mDrawRect.height() - pos.y() - mMargin)/mPixelsPerValUnit
            + mMinShownVal;
    *frame = mStartFrame + pos.x()/mPixelsPerFrame;
}

void QrealAnimator::updateMinAndMaxMove(QrealKey *key) {
    int keyId = mKeys.indexOf(key);
    if(mCurrentPoint->isEndPoint()) {
        mMinMoveFrame = key->getFrame();
        if(keyId == mKeys.count() - 1) {
            mMaxMoveFrame = 5000.;
        } else {
            mMaxMoveFrame = mKeys.at(keyId + 1)->getFrame();
        }
    } else if(mCurrentPoint->isStartPoint()) {
        if(keyId == 0) {
            mMinMoveFrame = 0.;
        } else {
            QrealKey *prevKey = mKeys.at(keyId - 1);
            mMinMoveFrame = prevKey->getFrame();
        }
        mMaxMoveFrame = key->getFrame();
    }
}

QrealPoint *QrealAnimator::getPointAt(QPointF pos) {
    qreal value;
    qreal frame;
    getValueAndFrameFromPos(pos, &value, &frame);
    return getPointAt(value, frame);
}

void QrealAnimator::constrainCtrlsFrameValues() {
    QrealKey *lastKey = NULL;
    foreach(QrealKey *key, mKeys) {
        if(lastKey != NULL) {
            lastKey->constrainEndCtrlMaxFrame(key->getFrame());
            key->constrainStartCtrlMinFrame(lastKey->getFrame());
        }
        lastKey = key;
    }
    updateKeysPath();
}

qreal QrealAnimator::clampValue(qreal value)
{
    return value;
}

qreal QrealAnimator::getPrevKeyValue(QrealKey *key)
{
    int keyId = mKeys.indexOf(key);
    if(keyId == 0) return key->getValue();
    return mKeys.at(keyId - 1)->getValue();
}

qreal QrealAnimator::getNextKeyValue(QrealKey *key)
{
    int keyId = mKeys.indexOf(key);
    if(keyId == mKeys.count() - 1) return key->getValue();
    return mKeys.at(keyId + 1)->getValue();
}

bool QrealAnimator::hasPrevKey(QrealKey *key)
{
    int keyId = mKeys.indexOf(key);
    if(keyId > 0) return true;
    return false;
}

bool QrealAnimator::hasNextKey(QrealKey *key)
{
    int keyId = mKeys.indexOf(key);
    if(keyId < mKeys.count() - 1) return true;
    return false;
}

void QrealAnimator::retrieveSavedValue()
{
    setCurrentValue(mSavedCurrentValue);
}

void QrealAnimator::incCurrentValue(qreal incBy)
{
    setCurrentValue(mCurrentValue + incBy);
}

void QrealAnimator::startTransform()
{
    mSavedCurrentValue = mCurrentValue;
    mTransformed = true;
}

void QrealAnimator::finishTransform(bool record)
{
    if(mTransformed) {
        mConnectedToMainWindow->addUndoRedo(
                    new ChangeQrealAnimatorValue(mSavedCurrentValue,
                                                 mCurrentValue,
                                                 this) );
        if(record) {
            saveCurrentValueAsKey();
        }
        mTransformed = false;
    }
}

void QrealAnimator::cancelTransform() {
    if(mTransformed) {
        mTransformed = false;
        retrieveSavedValue();
    }
}

void QrealAnimator::setUpdater(AnimatorUpdater *updater)
{
    mUpdater = updater;
}

void QrealAnimator::callUpdater()
{
    if(mUpdater == NULL) return;
    mUpdater->update();
}

void QrealAnimator::drawKeys(QPainter *p, qreal pixelsPerFrame,
                             qreal startX, qreal startY, qreal height,
                             int startFrame, int endFrame, bool detailedView)
{
    p->setPen(QPen(Qt::black, 1.));
    foreach(QrealKey *key, mKeys) {
        if(key->getFrame() >= startFrame && key->getFrame() <= endFrame) {
            if(key->isSelected() ) {
                p->setBrush(Qt::yellow);
            } else {
                p->setBrush(Qt::red);
            }
            p->drawRect(
                QRectF(
                    QPointF((key->getFrame() - startFrame + 0.5)*pixelsPerFrame +
                            startX - KEY_RECT_SIZE*0.5,
                            startY + (height - KEY_RECT_SIZE)*0.5 ),
                    QSize(KEY_RECT_SIZE, KEY_RECT_SIZE) ) );
        }
    }
}

void QrealAnimator::multCurrentValue(qreal mult)
{
    setCurrentValue(mCurrentValue*mult);
}

qreal QrealAnimator::getSavedValue()
{
    return mSavedCurrentValue;
}

void QrealAnimator::setConnectedToMainWindow(ConnectedToMainWindow *connected)
{
    mConnectedToMainWindow = connected;
}

void QrealAnimator::setCtrlsModeForSelected(CtrlsMode mode) {
    foreach(QrealKey *key, mSelectedKeys) {
        key->setCtrlsMode(mode);
    }
    constrainCtrlsFrameValues();
}

void QrealAnimator::setTwoSideCtrlForSelected() {
    foreach(QrealKey *key, mSelectedKeys) {
        key->setEndEnabled(true);
        key->setStartEnabled(true);
    }
    constrainCtrlsFrameValues();
}

void QrealAnimator::setRightSideCtrlForSelected() {
    foreach(QrealKey *key, mSelectedKeys) {
        key->setEndEnabled(true);
        key->setStartEnabled(false);
    }
    constrainCtrlsFrameValues();
}

void QrealAnimator::setLeftSideCtrlForSelected() {
    foreach(QrealKey *key, mSelectedKeys) {
        key->setEndEnabled(false);
        key->setStartEnabled(true);
    }
    constrainCtrlsFrameValues();
}

void QrealAnimator::setNoSideCtrlForSelected() {
    foreach(QrealKey *key, mSelectedKeys) {
        key->setEndEnabled(false);
        key->setStartEnabled(false);
    }
}

void QrealAnimator::deletePressed()
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
            mKeys.removeOne(key);
            delete key;
        }
        mSelectedKeys.clear();
        sortKeys();
        updateKeysPath();
    }
}

QrealPoint *QrealAnimator::getPointAt(qreal value, qreal frame) {
    QrealPoint *point = NULL;
    foreach(QrealKey *key, mKeys) {
        point = key->mousePress(frame, value,
                                mPixelsPerFrame, mPixelsPerValUnit);
        if(point != NULL) {
            break;
        }
    }
    return point;
}

void QrealAnimator::mousePress(QPointF pressPos) {
    mFirstMove = true;
    qreal value;
    qreal frame;
    getValueAndFrameFromPos(pressPos, &value, &frame);

    mCurrentPoint = getPointAt(value, frame);
    QrealKey *parentKey = (mCurrentPoint == NULL) ?
                NULL :
                mCurrentPoint->getParentKey();
    if(mCurrentPoint == NULL) {
        if(isCtrlPressed() ) {
            clearKeysSelection();
            QrealKey *newKey = new QrealKey(qRound(frame), this, value);
            appendKey(newKey);
            addKeyToSelection(newKey);
            mergeKeysIfNeeded();
            mCurrentPoint = newKey->getEndPoint();
            updateMinAndMaxMove(newKey);
            mCurrentPoint->setSelected(true);
            updateKeysPath();
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
        updateMinAndMaxMove(parentKey);
        mCurrentPoint->setSelected(true);
        //clearKeysSelection();
    }
}

void QrealAnimator::mouseMove(QPointF mousePos)
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
            sortKeys();
        } else {
            qreal clampedFrame = clamp(frame, mMinMoveFrame, mMaxMoveFrame);
            mCurrentPoint->moveTo(clampedFrame, clampValue(value));
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
                    key->mergeWith(lastKey);
                    mKeys.removeOne(lastKey);
                    delete lastKey;
                } else {
                    lastKey->mergeWith(key);
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

        constrainCtrlsFrameValues();
    }
}
