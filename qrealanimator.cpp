#include "qrealanimator.h"
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include "mainwindow.h"
#include "animationdockwidget.h"
#include "boxeslist.h"
#include <QMenu>

QrealAnimator::QrealAnimator()
{
}

void QrealAnimator::setValueRange(qreal minVal, qreal maxVal)
{
    mMinPossibleVal = minVal;
    mMaxPossibleVal = maxVal;
    setCurrentValue(mCurrentValue);
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

bool QrealAnimator::isKeyOnCurrentFrame() {
    return mKeyOnCurrentFrame;
}

bool QrealAnimator::isBoxesListDetailVisible()
{
    return mBoxesListDetailVisible;
}

void QrealAnimator::setBoxesListDetailVisible(bool bT)
{
    mBoxesListDetailVisible = bT;
}

qreal QrealAnimator::getBoxesListHeight()
{
    return LIST_ITEM_HEIGHT;
}

QString QrealAnimator::getValueText() {
    return QString::number(mCurrentValue, 'f', 2);
}

void QrealAnimator::drawBoxesList(QPainter *p,
                                  qreal drawX, qreal drawY,
                                  qreal pixelsPerFrame,
                                  int startFrame, int endFrame,
                                  bool animationBar)
{
    if(mIsCurrentAnimator) {
        p->fillRect(drawX, drawY,
                    LIST_ITEM_MAX_WIDTH - drawX, LIST_ITEM_HEIGHT,
                    QColor(255, 255, 255, 125));
    }
    if(mIsRecording) {
        p->drawPixmap(drawX, drawY, *BoxesList::ANIMATOR_RECORDING);
    } else {
        p->drawPixmap(drawX, drawY, *BoxesList::ANIMATOR_NOT_RECORDING);
    }
    p->setPen(Qt::black);
    drawX += 2*LIST_ITEM_CHILD_INDENT;
    p->drawText(drawX, drawY,
                LIST_ITEM_MAX_WIDTH - 80. - drawX, LIST_ITEM_HEIGHT,
                Qt::AlignVCenter | Qt::AlignLeft,
                getName() );
    p->setPen(Qt::blue);
    p->drawText(LIST_ITEM_MAX_WIDTH - 80., drawY,
                70., LIST_ITEM_HEIGHT,
                Qt::AlignVCenter | Qt::AlignLeft,
                " " + getValueText() );
    p->setPen(Qt::black);
    if(animationBar) {
        drawKeys(p, pixelsPerFrame, LIST_ITEM_MAX_WIDTH, drawY, LIST_ITEM_HEIGHT,
                 startFrame, endFrame, true);
    }
}

void QrealAnimator::handleListItemMousePress(qreal relX, qreal relY,
                                             QMouseEvent *event)
{
    Q_UNUSED(relY);
    if(event->button() == Qt::RightButton) {
        QMenu menu;
        menu.addAction("Add Key");
        QAction *selected_action = menu.exec(event->globalPos());
        if(selected_action != NULL)
        {
            if(selected_action->text() == "Add Key")
            {
                if(!mIsRecording) {
                    setRecording(true);
                }
                saveCurrentValueAsKey();
            }
        }
        else
        {

        }
    } else {
        if(relX < LIST_ITEM_CHILD_INDENT) {
            setRecording(!mIsRecording);
        } else if(relX < 2*LIST_ITEM_CHILD_INDENT) {
            setBoxesListDetailVisible(!mBoxesListDetailVisible);
        } else {
            mConnectedToMainWindow->getMainWindow()->getBoxesList()->
                    graphSetAnimator(this);
        }
    }
}

void QrealAnimator::removeAllKeys() {
    if(mKeys.isEmpty()) return;
    qreal currentValue = mCurrentValue;
    QList<QrealKey*> keys = mKeys;
    foreach(QrealKey *key, keys) {
        removeKey(key);
    }
    setCurrentValue(currentValue);
}

void QrealAnimator::setRecording(bool rec)
{
    mIsRecording = rec;
    if(rec) {
        saveCurrentValueAsKey();
    } else {
        removeAllKeys();
    }
    if(mParentAnimator != NULL) {
        mParentAnimator->childAnimatorIsRecordingChanged();
    }
}

bool QrealAnimator::isRecording()
{
    return mIsRecording;
}

void QrealAnimator::updateKeyOnCurrrentFrame()
{
    mKeyOnCurrentFrame = getKeyAtFrame(mCurrentFrame) != NULL;
}

QrealKey *QrealAnimator::getKeyAtPos(qreal relX, qreal relY,
                                     int minViewedFrame,
                                     qreal pixelsPerFrame) {
    Q_UNUSED(relY);
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

qreal QrealAnimator::getValueAtFrame(int frame) const
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

qreal QrealAnimator::getCurrentValue() const
{
    return mCurrentValue;
}

void QrealAnimator::setCurrentValue(qreal newValue)
{
    mCurrentValue = clamp(newValue, mMinPossibleVal, mMaxPossibleVal);
    callUpdater();
}

void QrealAnimator::updateValueFromCurrentFrame()
{
    setCurrentValue(getValueAtFrame(mCurrentFrame) );
}

void QrealAnimator::saveCurrentValueToKey(QrealKey *key)
{
    saveValueToKey(key, mCurrentValue);
}

void QrealAnimator::saveValueToKey(QrealKey *key, qreal value)
{
    key->setValue(value);
    updateKeysPath();
    if(mConnectedToMainWindow != NULL) {
        mConnectedToMainWindow->scheduleBoxesListRepaint();
        if(mIsCurrentAnimator) {
            mConnectedToMainWindow->graphUpdateAfterKeysChanged();
        }
    }
}

void QrealAnimator::appendKey(QrealKey *newKey) {
    mKeys.append(newKey);
    newKey->incNumberPointers();
    sortKeys();
    //mergeKeysIfNeeded();
    if(mParentAnimator != NULL) {
        mParentAnimator->addChildQrealKey(newKey);
    }
    if(mConnectedToMainWindow != NULL) {
        mConnectedToMainWindow->scheduleBoxesListRepaint();
        if(mIsCurrentAnimator) {
            mConnectedToMainWindow->graphUpdateAfterKeysChanged();
        }
    }
}

void QrealAnimator::removeKey(QrealKey *removeKey) {
    if(mKeys.removeOne(removeKey) ) {
        if(mParentAnimator != NULL) {
            mParentAnimator->removeChildQrealKey(removeKey);
        }
        removeKey->decNumberPointers();
        sortKeys();
    }
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
    updateKeyOnCurrrentFrame();
}

bool QrealAnimator::getNextAndPreviousKeyId(int *prevIdP, int *nextIdP,
                                            int frame) const {
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
        keyAtFrame = new QrealKey(mCurrentFrame, this, mCurrentValue);
        appendKey(keyAtFrame);
    } else {
        saveCurrentValueToKey(keyAtFrame);
    }
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
    if(lastKey == NULL) {
        mKeysPath.moveTo(0, -mCurrentValue);
        mKeysPath.lineTo(5000, -mCurrentValue);
    } else {
        mKeysPath.lineTo(5000, -lastKey->getValue());
    }
    updateValueFromCurrentFrame();
    mDrawPathUpdateNeeded = true;
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
                                    QrealKey *nextKey) const
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
    if(mKeys.isEmpty()) {
        *minValP = mCurrentValue;
        *maxValP = mCurrentValue;
    } else {
        bool first = true;
        foreach(QrealKey *key, mKeys) {
            int keyFrame = key->getFrame();
            if(keyFrame > endFrame || keyFrame < startFrame) continue;
            if(first) first = false;
            qreal keyVal = key->getValue();
            qreal startVal = key->getStartValue();
            qreal endVal = key->getEndValue();
            qreal maxKeyVal = qMax(qMax(startVal, endVal), keyVal);
            qreal minKeyVal = qMin(qMin(startVal, endVal), keyVal);
            if(maxKeyVal > maxVal) maxVal = maxKeyVal;
            if(minKeyVal < minVal) minVal = minKeyVal;
        }
        if(first) {
            int midFrame = (startFrame + endFrame)/2;
            qreal value = getValueAtFrame(midFrame);
            *minValP = value;
            *maxValP = value;
        } else {
            *minValP = minVal;
            *maxValP = maxVal;
        }
    }
}

void QrealAnimator::updateDrawPathIfNeeded(qreal height, qreal margin,
                                   qreal startFrame, qreal minShownVal,
                                   qreal pixelsPerFrame, qreal pixelsPerValUnit)
{
    if(mDrawPathUpdateNeeded ) {
        mDrawPathUpdateNeeded = false;
        QMatrix transform;
        transform.translate(-pixelsPerFrame*(startFrame - 0.5),
                    height + pixelsPerValUnit*minShownVal - margin);
        transform.scale(pixelsPerFrame, pixelsPerValUnit);
        mKeysDrawPath = mKeysPath*transform;
    }
}

void QrealAnimator::drawKeysPath(QPainter *p,
                                 qreal height, qreal margin,
                                 qreal startFrame, qreal minShownVal,
                                 qreal pixelsPerFrame, qreal pixelsPerValUnit)
{
    p->setPen(QPen(Qt::black, 4.));
    p->drawPath(mKeysDrawPath);
    p->setPen(QPen(Qt::red, 2.));
    p->drawPath(mKeysDrawPath);
    p->setPen(QPen(Qt::black, 2.));

    p->save();
    p->translate(0., height - margin);
    p->setBrush(Qt::black);
    foreach(QrealKey *key, mKeys) {
        key->drawGraphKey(p,
                  startFrame, minShownVal,
                  pixelsPerFrame, pixelsPerValUnit);
    }
    p->restore();
}

void QrealAnimator::setDrawPathUpdateNeeded()
{
    mDrawPathUpdateNeeded = true;
}

void QrealAnimator::getMinAndMaxMoveFrame(
                                     QrealKey *key, QrealPoint *currentPoint,
                                     qreal *minMoveFrame, qreal *maxMoveFrame) {
    qreal minMoveFrameT;
    qreal maxMoveFrameT;
    int keyId = mKeys.indexOf(key);
    if(currentPoint->isEndPoint()) {
        minMoveFrameT = key->getFrame();
        if(keyId == mKeys.count() - 1) {
            maxMoveFrameT = minMoveFrameT + 5000.;
        } else {
            maxMoveFrameT = mKeys.at(keyId + 1)->getFrame();
        }
    } else if(currentPoint->isStartPoint()) {
        if(keyId == 0) {
            minMoveFrameT = minMoveFrameT - 5000.;
        } else {
            QrealKey *prevKey = mKeys.at(keyId - 1);
            minMoveFrameT = prevKey->getFrame();
        }
        maxMoveFrameT = key->getFrame();
    } else {
        return;
    }
    *minMoveFrame = minMoveFrameT;
    *maxMoveFrame = maxMoveFrameT;

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

void QrealAnimator::finishTransform()
{
    if(mTransformed) {
        mConnectedToMainWindow->addUndoRedo(
                    new ChangeQrealAnimatorValue(mSavedCurrentValue,
                                                 mCurrentValue,
                                                 this) );
        if(mIsRecording) {
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
    Q_UNUSED(detailedView);
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

QrealPoint *QrealAnimator::getPointAt(qreal value, qreal frame,
                                qreal pixelsPerFrame, qreal pixelsPerValUnit) {
    QrealPoint *point = NULL;
    foreach(QrealKey *key, mKeys) {
        point = key->mousePress(frame, value,
                                pixelsPerFrame, pixelsPerValUnit);
        if(point != NULL) {
            break;
        }
    }
    return point;
}

void QrealAnimator::mergeKeysIfNeeded() {
    QrealKey *lastKey = NULL;
    QList<QrealKeyPair> keyPairsToMerge;
    foreach(QrealKey *key, mKeys) {
        if(lastKey != NULL) {
            if(key->getFrame() == lastKey->getFrame() ) {
                if(key->isDescendantSelected()) {
                    keyPairsToMerge << QrealKeyPair(key, lastKey);
                } else {
                    keyPairsToMerge << QrealKeyPair(lastKey, key);
                    key = NULL;
                }
            }
        }
        lastKey = key;
    }
    foreach(QrealKeyPair keyPair, keyPairsToMerge) {
        keyPair.merge();
    }

    updateKeysPath();
}

void QrealAnimator::addKeysInRectToList(QRectF frameValueRect,
                                        QList<QrealKey*> *keys) {
    foreach(QrealKey *key, mKeys) {
        if(key->isInsideRect(frameValueRect)) {
            keys->append(key);
        }
    }
}

void QrealAnimator::setIsCurrentAnimator(bool bT)
{
    mIsCurrentAnimator = bT;
}
