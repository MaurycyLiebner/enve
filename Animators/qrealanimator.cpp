#include "Animators/qrealanimator.h"
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include "mainwindow.h"
#include "animationdockwidget.h"
#include <QMenu>
#include "qrealanimatorvalueslider.h"
#include <QWidgetAction>
#include "BoxesList/boxsinglewidget.h"

QrealAnimator::QrealAnimator() : Animator() {

}

QrealAnimator::~QrealAnimator() {
    emit beingDeleted();
    foreach(QrealKey *key, mKeys) {
        key->decNumberPointers();
    }
    if(mUpdater != NULL) {
        mUpdater->decNumberPointers();
    }
}

#include <QSqlError>
#include <QSqlQuery>
int QrealAnimator::saveToSql(QSqlQuery *query) {
    if(!query->exec(
        QString("INSERT INTO qrealanimator (currentvalue) "
                "VALUES (%1)").
                arg(mCurrentValue, 0, 'f') ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    int thisSqlId = query->lastInsertId().toInt();

    foreach(QrealKey *key, mKeys) {
        key->saveToSql(thisSqlId);
    }

    return thisSqlId;
}

void QrealAnimator::loadFromSql(int qrealAnimatorId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM qrealanimator WHERE id = " +
            QString::number(qrealAnimatorId);
    if(query.exec(queryStr)) {
        query.next();
        int idQrealAnimatorId = query.record().indexOf("id");
        int currentValue = query.record().indexOf("currentvalue");

        loadKeysFromSql(query.value(idQrealAnimatorId).toInt() );

        if(mKeys.isEmpty()) {
            setCurrentValue(query.value(currentValue).toReal());
        } else {
            setRecordingWithoutChangingKeys(true, false);
        }
    } else {
        qDebug() << "Could not load qpointfanimator with id " << qrealAnimatorId;
    }
}

void QrealAnimator::loadKeysFromSql(int qrealAnimatorId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM qrealkey WHERE qrealanimatorid = " +
            QString::number(qrealAnimatorId);
    if(query.exec(queryStr)) {
        int idId = query.record().indexOf("id");
        while(query.next() ) {
            QrealKey *key = new QrealKey(this);
            key->loadFromSql(query.value(idId).toInt());
            appendKey(key);
        }
    } else {
        qDebug() << "Could not load qpointfanimator with id " << qrealAnimatorId;
    }
}

void QrealAnimator::setValueRange(qreal minVal, qreal maxVal) {
    mMinPossibleVal = minVal;
    mMaxPossibleVal = maxVal;
    setCurrentValue(mCurrentValue);
}

void QrealAnimator::getKeysInRect(QRectF selectionRect,
                                  qreal pixelsPerFrame,
                                  QList<QrealKey*> *keysList) {
    selectionRect.translate(-getFrameShift(), 0.);
    int selLeftFrame = selectionRect.left();
    if(0.5*pixelsPerFrame + KEY_RECT_SIZE*0.5 <
       selectionRect.left() - selLeftFrame*pixelsPerFrame) {
        selLeftFrame++;
    }
    int selRightFrame = selectionRect.right();
    if(0.5*pixelsPerFrame - KEY_RECT_SIZE*0.5 >
       selectionRect.right() - selRightFrame*pixelsPerFrame) {
        selRightFrame--;
    }
    for(int i = selRightFrame; i >= selLeftFrame; i--) {
        QrealKey *keyAtPos = getKeyAtAbsFrame(i);
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

bool QrealAnimator::isKeyOnCurrentFrame() {
    return mKeyOnCurrentFrame;
}


QString QrealAnimator::getValueText() {
    return QString::number(mCurrentValue, 'f', 2);
}

void QrealAnimator::openContextMenu(QPoint pos) {
    QMenu menu;
    menu.addAction("Add Key");
    QAction *selected_action = menu.exec(pos);
    if(selected_action != NULL)
    {
        if(selected_action->text() == "Add Key")
        {
            if(mIsRecording) {
                saveCurrentValueAsKey();
            } else {
                setRecording(true);
            }
        }
    } else {

    }
}

qreal QrealAnimator::getMinPossibleValue() {
    return mMinPossibleVal;
}

qreal QrealAnimator::getMaxPossibleValue() {
    return mMaxPossibleVal;
}

qreal QrealAnimator::getPrefferedValueStep()
{
    return mPrefferedValueStep;
}

void QrealAnimator::setPrefferedValueStep(qreal valueStep)
{
    mPrefferedValueStep = valueStep;
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

void QrealAnimator::setRecordingWithoutChangingKeys(bool rec, bool saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new QrealAnimatorRecordingSetUndoRedo(mIsRecording,
                                                          rec,
                                                          this));
    }
    mIsRecording = rec;
    setTraceKeyOnCurrentFrame(rec); // !!!
    if(mParentAnimator != NULL) {
        mParentAnimator->childAnimatorIsRecordingChanged();
    }
}

void QrealAnimator::setRecording(bool rec)
{
    setRecordingWithoutChangingKeys(rec);
    if(mIsRecording) {
        saveCurrentValueAsKey();
    } else {
        removeAllKeys();
        updateKeysPath();
    }
}

bool QrealAnimator::isRecording()
{
    return mIsRecording;
}

void QrealAnimator::updateKeyOnCurrrentFrame()
{
    if(mTraceKeyOnCurrentFrame) {
        mKeyOnCurrentFrame = getKeyAtAbsFrame(mCurrentAbsFrame) != NULL;
    }
}

QrealKey *QrealAnimator::getKeyAtPos(qreal relX,
                                     int minViewedFrame,
                                     qreal pixelsPerFrame) {
    qreal relFrame = relX/pixelsPerFrame - getFrameShift();
    qreal pressFrame = relFrame + minViewedFrame;
    if(pixelsPerFrame > KEY_RECT_SIZE) {
        int relFrameInt = relFrame;
        if( qAbs((relFrameInt + 0.5)*pixelsPerFrame - relX) > KEY_RECT_SIZE*0.5) {
            return NULL;
        }
    }
    if(pressFrame < 0) pressFrame -= 1.;
    qreal keyRectFramesSpan = 0.5*KEY_RECT_SIZE/pixelsPerFrame;
    int minPossibleKey = pressFrame - keyRectFramesSpan;
    int maxPossibleKey = pressFrame + keyRectFramesSpan;
    QrealKey *keyAtPos = NULL;
    for(int i = maxPossibleKey; i >= minPossibleKey; i--) {
        keyAtPos = getKeyAtRelFrame(i);
        if(keyAtPos != NULL) {
            return keyAtPos;
        }
    }
    return NULL;
}

void QrealAnimator::removeThisFromGraphAnimator() {
    //mMainWindow->getKeysView()->graphRemoveViewedAnimator(this);
}

void QrealAnimator::setAnimatorColor(QColor color)
{
    mAnimatorColor = color;
}

void QrealAnimator::clearFromGraphView() {
    removeThisFromGraphAnimator();
}

void QrealAnimator::freezeMinMaxValues()
{
    mMinMaxValuesFrozen = true;
}

qreal QrealAnimator::getCurrentValueAtAbsFrame(const int &frame) const {
    if(frame == mCurrentAbsFrame) return mCurrentValue;
    return getValueAtAbsFrame(frame);
}

qreal QrealAnimator::getValueAtAbsFrame(int frame) const {
    return getValueAtRelFrame(absFrameToRelFrame(frame));
}

qreal QrealAnimator::getValueAtRelFrame(int frame) const {
    int prevId;
    int nextId;
    qreal returnVal;
    if(getNextAndPreviousKeyIdForRelFrame(&prevId, &nextId, frame) ) {
        if(nextId == prevId) {
            returnVal = mKeys.at(nextId)->getValue();
        } else {
            QrealKey *prevKey = mKeys.at(prevId);
            QrealKey *nextKey = mKeys.at(nextId);
            returnVal = getValueAtRelFrame(frame, prevKey, nextKey);
        }
    } else {
        returnVal = mCurrentValue;
    }
    return returnVal;
}

qreal QrealAnimator::getValueAtRelFrame(int frame,
                                    QrealKey *prevKey,
                                    QrealKey *nextKey) const
{
    qreal t = tFromX(prevKey->getRelFrame(),
                     prevKey->getEndValueFrame(),
                     nextKey->getStartValueFrame(),
                     nextKey->getRelFrame(), frame);
    qreal p0y = prevKey->getValue();
    qreal p1y = prevKey->getEndValue();
    qreal p2y = nextKey->getStartValue();
    qreal p3y = nextKey->getValue();
    return calcCubicBezierVal(p0y, p1y, p2y, p3y, t);
}

qreal QrealAnimator::getCurrentValue() const
{
    return mCurrentValue;
}

void QrealAnimator::setCurrentValue(qreal newValue, bool finish)
{
    newValue = clamp(newValue, mMinPossibleVal, mMaxPossibleVal);

    if(finish) {
        startTransform();
        mCurrentValue = newValue;
        finishTransform();

        emit valueChangedSignal(mCurrentValue);
        callUpdater();

        return;
    }
    if(newValue == mCurrentValue) return;
    mCurrentValue = newValue;

    emit valueChangedSignal(mCurrentValue);

    callUpdater();
}

void QrealAnimator::updateValueFromCurrentFrame()
{
    setCurrentValue(getValueAtAbsFrame(mCurrentAbsFrame));
}

void QrealAnimator::saveCurrentValueToKey(QrealKey *key)
{
    saveValueToKey(key, mCurrentValue);
}

void QrealAnimator::saveValueToKey(int frame, qreal value)
{
    QrealKey *keyAtFrame = getKeyAtAbsFrame(frame);
    if(keyAtFrame == NULL) {
        keyAtFrame = new QrealKey(this);
        keyAtFrame->setRelFrame(frame);
        keyAtFrame->setValue(value);
        appendKey(keyAtFrame);
        updateKeysPath();
    } else {
        saveValueToKey(keyAtFrame, value);
    }
}

void QrealAnimator::saveValueToKey(QrealKey *key, qreal value, bool saveUndoRedo)
{
    key->setValue(value, saveUndoRedo);
    updateKeysPath();
    updateValueFromCurrentFrame();

    if(mIsCurrentAnimator) {
        graphScheduleUpdateAfterKeysChanged();
    }
}

void QrealAnimator::appendKey(QrealKey *newKey, bool saveUndoRedo) {
    if(saveUndoRedo && !isComplexAnimator()) {
        addUndoRedo(new AddQrealKeyToAnimatorUndoRedo(newKey, this));
    }
    mKeys.append(newKey);
    newKey->incNumberPointers();
    sortKeys();
    //mergeKeysIfNeeded();
    if(mParentAnimator != NULL && !newKey->hasParentKey()) {
        mParentAnimator->addChildQrealKey(newKey);
    }

    if(mIsCurrentAnimator) {
        graphScheduleUpdateAfterKeysChanged();
    }

    updateKeyOnCurrrentFrame();
    updateValueFromCurrentFrame();
}

void QrealAnimator::removeKey(QrealKey *removeKey, bool saveUndoRedo) {
    if(mKeys.removeOne(removeKey) ) {

        if(saveUndoRedo && !isComplexAnimator()) {
            addUndoRedo(new RemoveQrealKeyFromAnimatorUndoRedo(removeKey, this));
        }

        if(mParentAnimator != NULL) {
            mParentAnimator->removeChildQrealKey(removeKey);
        }
        removeKey->decNumberPointers();
        sortKeys();

        if(mIsCurrentAnimator) {
            graphScheduleUpdateAfterKeysChanged();
        }

        updateKeyOnCurrrentFrame();

        updateValueFromCurrentFrame();
    }
}

void QrealAnimator::moveKeyToFrame(QrealKey *key, int newFrame)
{
    if(mParentAnimator != NULL) {
        mParentAnimator->removeChildQrealKey(key);
    }
    key->setRelFrame(newFrame);
    if(mParentAnimator != NULL) {
        mParentAnimator->addChildQrealKey(key);
    }
    sortKeys();
    updateKeysPath();

    updateKeyOnCurrrentFrame();
}

void QrealAnimator::setAbsFrame(int frame) {
    Animator::setAbsFrame(frame);
    //updateValueFromCurrentFrame();

    updateKeyOnCurrrentFrame();
    qreal newValue = getValueAtAbsFrame(mCurrentAbsFrame);
    if(newValue == mCurrentValue) return;
    mCurrentValue = newValue;

    emit valueChangedSignal(mCurrentValue);

    callFrameChangeUpdater();
}

bool QrealAnimator::getNextAndPreviousKeyIdForRelFrame(
                                int *prevIdP, int *nextIdP,
                                int frame) const {
    if(mKeys.isEmpty()) return false;
    int minId = 0;
    int maxId = mKeys.count() - 1;
    if(frame >= mKeys.last()->getRelFrame()) {
        *prevIdP = maxId;
        *nextIdP = maxId;
        return true;
    }
    if(frame <= mKeys.first()->getRelFrame()) {
        *prevIdP = minId;
        *nextIdP = minId;
        return true;
    }
    while(maxId - minId > 1) {
        int guess = (maxId + minId)/2;
        int keyFrame = mKeys.at(guess)->getRelFrame();
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
        if(key->getRelFrame() > frame) {
            if(minId != 0) {
                minId = minId - 1;
            }
        } else if(key->getRelFrame() < frame) {
            if(minId < mKeys.count() - 1) {
                maxId = minId + 1;
            }
        }
    }
    *prevIdP = minId;
    *nextIdP = maxId;
    return true;
}

QrealKey *QrealAnimator::getKeyAtAbsFrame(const int &frame) {
    return getKeyAtRelFrame(absFrameToRelFrame(frame));
}

QrealKey *QrealAnimator::getKeyAtRelFrame(const int &frame) {
    if(mKeys.isEmpty()) return NULL;
    if(frame > mKeys.last()->getRelFrame()) return NULL;
    if(frame < mKeys.first()->getRelFrame()) return NULL;
    int minId = 0;
    int maxId = mKeys.count() - 1;
    while(maxId - minId > 1) {
        int guess = (maxId + minId)/2;
        if(mKeys.at(guess)->getRelFrame() > frame) {
            maxId = guess;
        } else {
            minId = guess;
        }
    }
    QrealKey *minIdKey = mKeys.at(minId);
    if(minIdKey->getRelFrame() == frame) return minIdKey;
    QrealKey *maxIdKey = mKeys.at(maxId);
    if(maxIdKey->getRelFrame() == frame) return maxIdKey;
    return NULL;
}

void QrealAnimator::saveValueAtAbsFrameAsKey(int frame) {
    QrealKey *keyAtFrame = getKeyAtAbsFrame(frame);
    if(keyAtFrame == NULL) {
        qreal value = getValueAtAbsFrame(frame);
        keyAtFrame = new QrealKey(this);
        keyAtFrame->setRelFrame(frame);
        keyAtFrame->setValue(value);
        appendKey(keyAtFrame);
        updateKeysPath();
    } else {
        saveCurrentValueToKey(keyAtFrame);
    }
}

void QrealAnimator::setTraceKeyOnCurrentFrame(bool bT)
{
    mTraceKeyOnCurrentFrame = bT;
}

void QrealAnimator::saveCurrentValueAsKey()
{
    if(!mIsRecording) setRecording(true);
    QrealKey *keyAtFrame = getKeyAtAbsFrame(mCurrentAbsFrame);
    if(keyAtFrame == NULL) {
        keyAtFrame = new QrealKey(this);
        keyAtFrame->setRelFrame(mCurrentAbsFrame);
        keyAtFrame->setValue(mCurrentValue);
        appendKey(keyAtFrame);
        updateKeysPath();
    } else {
        saveCurrentValueToKey(keyAtFrame);
    }
}

void QrealAnimator::updateKeysPath()
{
    mKeysPath = QPainterPath();
    QrealKey *lastKey = NULL;
    foreach(QrealKey *key, mKeys) {
        int keyFrame = key->getAbsFrame();
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
        mKeysPath.moveTo(-5000, -mCurrentValue);
        mKeysPath.lineTo(5000, -mCurrentValue);
    } else {
        mKeysPath.lineTo(5000, -lastKey->getValue());
    }
    updateValueFromCurrentFrame();
    setDrawPathUpdateNeeded();
}

bool keysFrameSort(QrealKey *key1, QrealKey *key2)
{
    return key1->getAbsFrame() < key2->getAbsFrame();
}

void QrealAnimator::sortKeys()
{
    qSort(mKeys.begin(), mKeys.end(), keysFrameSort);
}

void QrealAnimator::getMinAndMaxValues(qreal *minValP, qreal *maxValP) {
    if(mMinMaxValuesFrozen) {
        *minValP = mMinPossibleVal;
        *maxValP = mMaxPossibleVal;
        return;
    }
    if(mIsComplexAnimator) {
//        *minValP = mMainWindow->getKeysView()->getMinViewedFrame();
//        *maxValP = mMainWindow->getKeysView()->getMaxViewedFrame();
        return;
    }
    qreal minVal = 100000.;
    qreal maxVal = -100000.;
    if(mKeys.isEmpty()) {
        *minValP = mCurrentValue - mPrefferedValueStep;
        *maxValP = mCurrentValue + mPrefferedValueStep;
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

        *minValP = minVal - mPrefferedValueStep;
        *maxValP = maxVal + mPrefferedValueStep;
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
            int keyFrame = key->getAbsFrame();
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
            qreal value = getValueAtAbsFrame(midFrame);
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
    p->setPen(QPen(mAnimatorColor, 2.));
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
    if(currentPoint->isKeyPoint()) return;
    qreal keyFrame = key->getAbsFrame();

    qreal startMinMoveFrame;
    qreal endMaxMoveFrame;
    int keyId = mKeys.indexOf(key);

    if(keyId == mKeys.count() - 1) {
        endMaxMoveFrame = keyFrame + 5000.;
    } else {
        endMaxMoveFrame = mKeys.at(keyId + 1)->getAbsFrame();
    }

    if(keyId == 0) {
        startMinMoveFrame = keyFrame - 5000.;
    } else {
        QrealKey *prevKey = mKeys.at(keyId - 1);
        startMinMoveFrame = prevKey->getAbsFrame();
    }

    if(key->getCtrlsMode() == CtrlsMode::CTRLS_SYMMETRIC) {
        if(currentPoint->isEndPoint()) {
            *minMoveFrame = keyFrame;
            *maxMoveFrame = qMin(endMaxMoveFrame, 2*keyFrame - startMinMoveFrame);
        } else {
            *minMoveFrame = qMax(startMinMoveFrame, 2*keyFrame - endMaxMoveFrame);
            *maxMoveFrame = keyFrame;
        }
    } else {
        if(currentPoint->isEndPoint()) {
            *minMoveFrame = keyFrame;
            *maxMoveFrame = endMaxMoveFrame;
        } else {
            *minMoveFrame = startMinMoveFrame;
            *maxMoveFrame = keyFrame;
        }
    }
}

void QrealAnimator::constrainCtrlsFrameValues() {
    QrealKey *lastKey = NULL;
    foreach(QrealKey *key, mKeys) {
        if(lastKey != NULL) {
            lastKey->constrainEndCtrlMaxFrame(key->getAbsFrame());
            key->constrainStartCtrlMinFrame(lastKey->getAbsFrame());
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

void QrealAnimator::incSavedValueToCurrentValue(qreal incBy) {
    setCurrentValue(mSavedCurrentValue + incBy);
}

void QrealAnimator::multSavedValueToCurrentValue(qreal multBy) {
    setCurrentValue(mSavedCurrentValue * multBy);
}

void QrealAnimator::incCurrentValue(qreal incBy)
{
    setCurrentValue(mCurrentValue + incBy);
}

void QrealAnimator::startTransform()
{
    if(mTransformed) return;
    mSavedCurrentValue = mCurrentValue;
    mTransformed = true;
}

void QrealAnimator::finishTransform()
{
    if(mTransformed) {
//        if(mSavedCurrentValue == mCurrentValue) {
//            mTransformed = false;
//        } else {
            addUndoRedo(new ChangeQrealAnimatorValue(mSavedCurrentValue,
                                                     mCurrentValue,
                                                     this) );
            if(mIsRecording) {
                saveCurrentValueAsKey();
            }
            mTransformed = false;

            if(mIsCurrentAnimator) {
                graphScheduleUpdateAfterKeysChanged();
            }
//        }
    }
}

void QrealAnimator::cancelTransform() {
    if(mTransformed) {
        mTransformed = false;
        retrieveSavedValue();
    }
}

void QrealAnimator::setTransformed(bool bT) {
    mTransformed = bT;
}

void QrealAnimator::makeDuplicate(QrealAnimator *target) {
    target->setName(mName);
    target->setRecording(false);
    target->setCurrentValue(mCurrentValue, false);
    if(mIsRecording) {
        target->setRecordingWithoutChangingKeys(mIsRecording);
    }
    foreach(QrealKey *key, mKeys) {
        QrealKey *duplicate = key->makeQrealKeyDuplicate(target);
        target->appendKey(duplicate);
    }
}

QrealAnimator *QrealAnimator::makeDuplicate() {
    QrealAnimator *target = new QrealAnimator();
    makeDuplicate(target);
    return target;
}

void QrealAnimator::setUpdater(AnimatorUpdater *updater)
{
    if(mUpdaterBlocked) return;
    if(mUpdater != NULL) mUpdater->decNumberPointers();
    mUpdater = updater;
    if(mUpdater != NULL) mUpdater->incNumberPointers();
}

void QrealAnimator::blockUpdater() {
    mUpdaterBlocked = true;
}

void QrealAnimator::callUpdater()
{
    if(mUpdater == NULL) {
        return;
    } else {
        mUpdater->update();
    }
}

void QrealAnimator::callFrameChangeUpdater() {
    if(mUpdater == NULL) {
        return;
    } else {
        mUpdater->frameChangeUpdate();
    }
}

void QrealAnimator::drawKey(QPainter *p,
                            QrealKey *key,
                            const qreal &pixelsPerFrame,
                            const qreal &drawY,
                            const int &startFrame) {
    if(key->isSelected() ||
       key->isAncestorSelected()) {
        p->setBrush(Qt::yellow);
    } else {
        p->setBrush(Qt::red);
    }
    if(key->isHovered()) {
        p->save();
        p->setPen(QPen(Qt::black, 1.5));
        p->drawEllipse(
            QRectF(
                QPointF((key->getAbsFrame() - startFrame + 0.5)*
                        pixelsPerFrame - KEY_RECT_SIZE*0.5,
                        drawY + (BOX_HEIGHT -
                                  KEY_RECT_SIZE)*0.5 ),
                QSize(KEY_RECT_SIZE, KEY_RECT_SIZE) ) );
        p->restore();
    } else {
        p->drawEllipse(
            QRectF(
                QPointF((key->getAbsFrame() - startFrame + 0.5)*
                        pixelsPerFrame - KEY_RECT_SIZE*0.5,
                        drawY + (BOX_HEIGHT -
                                  KEY_RECT_SIZE)*0.5 ),
                QSize(KEY_RECT_SIZE, KEY_RECT_SIZE) ) );
    }
}

void QrealAnimator::drawKeys(QPainter *p, qreal pixelsPerFrame,
                             qreal drawY,
                             int startFrame, int endFrame)
{
    p->save();
    //p->translate(getFrameShift()*pixelsPerFrame, 0.);
    foreach(QrealKey *key, mKeys) {
        if(key->getAbsFrame() >= startFrame &&
           key->getAbsFrame() <= endFrame) {
            drawKey(p, key, pixelsPerFrame, drawY, startFrame);
        }
    }
    p->restore();
}

void QrealAnimator::multCurrentValue(qreal mult) {
    setCurrentValue(mCurrentValue*mult);
}

qreal QrealAnimator::getSavedValue() {
    return mSavedCurrentValue;
}

QrealPoint *QrealAnimator::getPointAt(qreal value, qreal frame,
                                qreal pixelsPerFrame, qreal pixelsPerValUnit) {
    QrealPoint *point = NULL;
    foreach(QrealKey *key, mKeys) {
//        point = key->mousePress(frame, value,
//                                pixelsPerFrame, pixelsPerValUnit);
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
            if(key->getAbsFrame() == lastKey->getAbsFrame() ) {
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
    if(bT) {
        updateKeysPath();
    }
}
