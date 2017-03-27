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
#include "qrealpoint.h"

QrealAnimator::QrealAnimator() : Animator() {

}

QrealAnimator::~QrealAnimator() {
}

#include <QSqlError>
#include <QSqlQuery>
int QrealAnimator::prp_saveToSql(QSqlQuery *query, const int &parentId) {
    Q_UNUSED(parentId);
    if(!query->exec(
        QString("INSERT INTO qrealanimator (currentvalue) "
                "VALUES (%1)").
                arg(mCurrentValue, 0, 'f') ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    int thisSqlId = query->lastInsertId().toInt();

    QrealKey *key; foreachQK(key, anim_mKeys)
        key->saveToSql(thisSqlId);
    }

    return thisSqlId;
}

void QrealAnimator::prp_loadFromSql(const int &qrealAnimatorId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM qrealanimator WHERE id = " +
            QString::number(qrealAnimatorId);
    if(query.exec(queryStr)) {
        query.next();
        int idQrealAnimatorId = query.record().indexOf("id");
        int currentValue = query.record().indexOf("currentvalue");

        anim_loadKeysFromSql(query.value(idQrealAnimatorId).toInt() );

        if(anim_mKeys.isEmpty()) {
            qra_setCurrentValue(query.value(currentValue).toReal());
        } else {
            anim_setRecordingWithoutChangingKeys(true, false);
        }
    } else {
        qDebug() << "Could not load qpointfanimator with id " << qrealAnimatorId;
    }
}

void QrealAnimator::anim_loadKeysFromSql(int qrealAnimatorId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM qrealkey WHERE qrealanimatorid = " +
            QString::number(qrealAnimatorId);
    if(query.exec(queryStr)) {
        int idId = query.record().indexOf("id");
        while(query.next() ) {
            QrealKey *key = new QrealKey(this);
            key->loadFromSql(query.value(idId).toInt());
            anim_appendKey(key);
        }
    } else {
        qDebug() << "Could not load qpointfanimator with id " << qrealAnimatorId;
    }
}

void QrealAnimator::qra_setValueRange(qreal minVal, qreal maxVal) {
    mMinPossibleVal = minVal;
    mMaxPossibleVal = maxVal;
    qra_setCurrentValue(mCurrentValue);
}

void QrealAnimator::qra_incAllValues(qreal valInc)
{
    QrealKey *key; foreachQK(key, anim_mKeys)
        key->incValue(valInc);
    }
    qra_incCurrentValue(valInc);
}

QString QrealAnimator::prp_getValueText() {
    return QString::number(mCurrentValue, 'f', 2);
}

void QrealAnimator::prp_openContextMenu(QPoint pos) {
    QMenu menu;
    menu.addAction("Add Key");
    QAction *selected_action = menu.exec(pos);
    if(selected_action != NULL)
    {
        if(selected_action->text() == "Add Key")
        {
            if(anim_mIsRecording) {
                anim_saveCurrentValueAsKey();
            } else {
                prp_setRecording(true);
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

qreal QrealAnimator::getPrefferedValueStep() {
    return mPrefferedValueStep;
}

void QrealAnimator::setPrefferedValueStep(const qreal &valueStep) {
    mPrefferedValueStep = valueStep;
}

void QrealAnimator::prp_setRecording(bool rec) {
    anim_setRecordingWithoutChangingKeys(rec);
    if(anim_mIsRecording) {
        anim_saveCurrentValueAsKey();
    } else {
        anim_removeAllKeys();
        qra_updateKeysPath();
    }
}

void QrealAnimator::removeThisFromGraphAnimator() {
    //mMainWindow->getKeysView()->graphRemoveViewedAnimator(this);
}

void QrealAnimator::setAnimatorColor(QColor color) {
    anim_mAnimatorColor = color;
}

void QrealAnimator::prp_clearFromGraphView() {
    removeThisFromGraphAnimator();
}

void QrealAnimator::freezeMinMaxValues() {
    mMinMaxValuesFrozen = true;
}

qreal QrealAnimator::getCurrentValueAtAbsFrame(const int &frame) const {
    if(frame == anim_mCurrentAbsFrame) return mCurrentValue;
    return qra_getValueAtAbsFrame(frame);
}

qreal QrealAnimator::qra_getValueAtAbsFrame(int frame) const {
    return qra_getValueAtRelFrame(anim_absFrameToRelFrame(frame));
}

QrealKey *QrealAnimator::getQrealKeyAtId(const int &id) const {
    return (QrealKey*)anim_mKeys.at(id);
}

qreal QrealAnimator::qra_getValueAtRelFrame(int frame) const {
    int prevId;
    int nextId;
    qreal returnVal;
    if(anim_getNextAndPreviousKeyIdForRelFrame(&prevId, &nextId, frame) ) {
        if(nextId == prevId) {
            returnVal = getQrealKeyAtId(nextId)->getValue();
        } else {
            QrealKey *prevKey = getQrealKeyAtId(prevId);
            QrealKey *nextKey = getQrealKeyAtId(nextId);
            returnVal = qra_getValueAtRelFrame(frame, prevKey, nextKey);
        }
    } else {
        returnVal = mCurrentValue;
    }
    return returnVal;
}

qreal QrealAnimator::qra_getValueAtRelFrame(int frame,
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

qreal QrealAnimator::qra_getCurrentValue() const {
    return mCurrentValue;
}

void QrealAnimator::qra_setCurrentValue(qreal newValue, bool finish) {
    newValue = clamp(newValue, mMinPossibleVal, mMaxPossibleVal);

    if(finish) {
        prp_startTransform();
        mCurrentValue = newValue;
        prp_finishTransform();

        emit valueChangedSignal(mCurrentValue);
        prp_callUpdater();

        return;
    }
    if(newValue == mCurrentValue) return;
    mCurrentValue = newValue;

    emit valueChangedSignal(mCurrentValue);

    prp_callUpdater();
}

void QrealAnimator::qra_updateValueFromCurrentFrame() {
    qra_setCurrentValue(qra_getValueAtAbsFrame(anim_mCurrentAbsFrame));
}

void QrealAnimator::qra_saveCurrentValueToKey(QrealKey *key) {
    qra_saveValueToKey(key, mCurrentValue);
}

void QrealAnimator::qra_saveValueToKey(int frame,
                                       qreal value) {
    QrealKey *keyAtFrame = (QrealKey*)anim_getKeyAtAbsFrame(frame);
    if(keyAtFrame == NULL) {
        keyAtFrame = new QrealKey(this);
        keyAtFrame->setRelFrame(frame);
        keyAtFrame->setValue(value);
        anim_appendKey(keyAtFrame);
        qra_updateKeysPath();
    } else {
        qra_saveValueToKey(keyAtFrame, value);
    }
}

void QrealAnimator::qra_saveValueToKey(QrealKey *key,
                                       qreal value,
                                       bool saveUndoRedo) {
    key->setValue(value, saveUndoRedo);
    qra_updateKeysPath();
    qra_updateValueFromCurrentFrame();

    if(anim_mIsCurrentAnimator) {
        graphScheduleUpdateAfterKeysChanged();
    }
}

void QrealAnimator::prp_setAbsFrame(int frame) {
    anim_mCurrentAbsFrame = frame;
    anim_updateRelFrame();
    //updateValueFromCurrentFrame();

    anim_updateKeyOnCurrrentFrame();
    qreal newValue = qra_getValueAtAbsFrame(anim_mCurrentAbsFrame);
    if(newValue == mCurrentValue) return;
    mCurrentValue = newValue;

    emit valueChangedSignal(mCurrentValue);

    anim_callFrameChangeUpdater();
}

void QrealAnimator::saveValueAtAbsFrameAsKey(int frame) {
    QrealKey *keyAtFrame = (QrealKey*)anim_getKeyAtAbsFrame(frame);
    if(keyAtFrame == NULL) {
        qreal value = qra_getValueAtAbsFrame(frame);
        keyAtFrame = new QrealKey(this);
        keyAtFrame->setRelFrame(frame);
        keyAtFrame->setValue(value);
        anim_appendKey(keyAtFrame);
        qra_updateKeysPath();
    } else {
        qra_saveCurrentValueToKey(keyAtFrame);
    }
}

void QrealAnimator::anim_saveCurrentValueAsKey() {
    if(!anim_mIsRecording) prp_setRecording(true);
    QrealKey *keyAtFrame = (QrealKey*)anim_getKeyAtAbsFrame(
                                            anim_mCurrentAbsFrame);
    if(keyAtFrame == NULL) {
        keyAtFrame = new QrealKey(this);
        keyAtFrame->setRelFrame(anim_mCurrentRelFrame);
        keyAtFrame->setValue(mCurrentValue);
        anim_appendKey(keyAtFrame);
        qra_updateKeysPath();
    } else {
        qra_saveCurrentValueToKey(keyAtFrame);
    }
}

void QrealAnimator::anim_removeAllKeys() {
    if(anim_mKeys.isEmpty()) return;
    qreal currentValue = mCurrentValue;
    QList<Key*> keys = anim_mKeys;
    foreach(Key *key, keys) {
        anim_removeKey(key);
    }
    qra_setCurrentValue(currentValue);
}

void QrealAnimator::anim_mergeKeysIfNeeded() {
    Animator::anim_mergeKeysIfNeeded();
    qra_updateKeysPath();
}

void QrealAnimator::anim_appendKey(Key *newKey,
                                   bool saveUndoRedo) {
    Animator::anim_appendKey(newKey, saveUndoRedo);
    qra_updateValueFromCurrentFrame();
}

void QrealAnimator::anim_removeKey(Key *keyToRemove,
                                   bool saveUndoRedo) {
    Animator::anim_removeKey(keyToRemove, saveUndoRedo);
    qra_updateValueFromCurrentFrame();
}

void QrealAnimator::anim_moveKeyToFrame(Key *key,
                                        int newFrame) {
    Animator::anim_moveKeyToFrame(key, newFrame);

    qra_updateKeysPath();
}

void QrealAnimator::qra_updateKeysPath() {
    mKeysPath = QPainterPath();
    QrealKey *lastKey = NULL;
    QrealKey *key; foreachQK(key, anim_mKeys)
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
    qra_updateValueFromCurrentFrame();
    setDrawPathUpdateNeeded();
}

void QrealAnimator::qra_getMinAndMaxValues(qreal *minValP, qreal *maxValP) {
    if(mMinMaxValuesFrozen) {
        *minValP = mMinPossibleVal;
        *maxValP = mMaxPossibleVal;
        return;
    }
    if(anim_mIsComplexAnimator) {
//        *minValP = mMainWindow->getKeysView()->getMinViewedFrame();
//        *maxValP = mMainWindow->getKeysView()->getMaxViewedFrame();
        return;
    }
    qreal minVal = 100000.;
    qreal maxVal = -100000.;
    if(anim_mKeys.isEmpty()) {
        *minValP = mCurrentValue - mPrefferedValueStep;
        *maxValP = mCurrentValue + mPrefferedValueStep;
    } else {
        QrealKey *key; foreachQK(key, anim_mKeys)
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

void QrealAnimator::qra_getMinAndMaxValuesBetweenFrames(
        int startFrame, int endFrame,
        qreal *minValP, qreal *maxValP) {
    qreal minVal = 100000.;
    qreal maxVal = -100000.;
    if(anim_mKeys.isEmpty()) {
        *minValP = mCurrentValue;
        *maxValP = mCurrentValue;
    } else {
        bool first = true;
        QrealKey *key; foreachQK(key, anim_mKeys)
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
            qreal value = qra_getValueAtAbsFrame(midFrame);
            *minValP = value;
            *maxValP = value;
        } else {
            *minValP = minVal;
            *maxValP = maxVal;
        }
    }
}

void QrealAnimator::qra_updateDrawPathIfNeeded(qreal height, qreal margin,
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
    p->setPen(QPen(anim_mAnimatorColor, 2.));
    p->drawPath(mKeysDrawPath);
    p->setPen(QPen(Qt::black, 2.));

    p->save();
    p->translate(0., height - margin);
    p->setBrush(Qt::black);
    QrealKey *key; foreachQK(key, anim_mKeys)
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
    int keyId = anim_mKeys.indexOf(key);

    if(keyId == anim_mKeys.count() - 1) {
        endMaxMoveFrame = keyFrame + 5000.;
    } else {
        endMaxMoveFrame = anim_mKeys.at(keyId + 1)->getAbsFrame();
    }

    if(keyId == 0) {
        startMinMoveFrame = keyFrame - 5000.;
    } else {
        QrealKey *prevKey = getQrealKeyAtId(keyId - 1);
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

void QrealAnimator::qra_constrainCtrlsFrameValues() {
    QrealKey *lastKey = NULL;
    QrealKey *key; foreachQK(key, anim_mKeys)
        if(lastKey != NULL) {
            lastKey->constrainEndCtrlMaxFrame(key->getAbsFrame());
            key->constrainStartCtrlMinFrame(lastKey->getAbsFrame());
        }
        lastKey = key;
    }
    qra_updateKeysPath();
}

qreal QrealAnimator::qra_clampValue(qreal value) {
    return value;
}

qreal QrealAnimator::qra_getPrevKeyValue(QrealKey *key) {
    int keyId = anim_mKeys.indexOf(key);
    if(keyId == 0) return key->getValue();
    return getQrealKeyAtId(keyId - 1)->getValue();
}

qreal QrealAnimator::qra_getNextKeyValue(QrealKey *key) {
    int keyId = anim_mKeys.indexOf(key);
    if(keyId == anim_mKeys.count() - 1) return key->getValue();
    return getQrealKeyAtId(keyId + 1)->getValue();
}

void QrealAnimator::prp_retrieveSavedValue() {
    qra_setCurrentValue(mSavedCurrentValue);
}

void QrealAnimator::incSavedValueToCurrentValue(qreal incBy) {
    qra_setCurrentValue(mSavedCurrentValue + incBy);
}

void QrealAnimator::multSavedValueToCurrentValue(qreal multBy) {
    qra_setCurrentValue(mSavedCurrentValue * multBy);
}

void QrealAnimator::qra_incCurrentValue(qreal incBy) {
    qra_setCurrentValue(mCurrentValue + incBy);
}

void QrealAnimator::prp_startTransform() {
    if(mTransformed) return;
    mSavedCurrentValue = mCurrentValue;
    mTransformed = true;
}

void QrealAnimator::prp_finishTransform() {
    if(mTransformed) {
//        if(mSavedCurrentValue == mCurrentValue) {
//            mTransformed = false;
//        } else {
        addUndoRedo(new ChangeQrealAnimatorValue(mSavedCurrentValue,
                                                 mCurrentValue,
                                                 this) );
        if(anim_mIsRecording) {
            anim_saveCurrentValueAsKey();
        }
        mTransformed = false;

        if(anim_mIsCurrentAnimator) {
            graphScheduleUpdateAfterKeysChanged();
        }
//        }
        prp_callFinishUpdater();
    }
}

void QrealAnimator::prp_cancelTransform() {
    if(mTransformed) {
        mTransformed = false;
        prp_retrieveSavedValue();
        prp_callFinishUpdater();
    }
}

void QrealAnimator::prp_makeDuplicate(Property *target) {
    QrealAnimator *qa_target = (QrealAnimator*)target;
    qa_target->prp_setName(prp_mName);
    qa_target->prp_setRecording(false);
    qa_target->qra_setCurrentValue(mCurrentValue, false);
    if(anim_mIsRecording) {
        qa_target->anim_setRecordingWithoutChangingKeys(anim_mIsRecording);
    }
    QrealKey *key; foreachQK(key, anim_mKeys)
        QrealKey *duplicate = key->makeQrealKeyDuplicate(qa_target);
        qa_target->anim_appendKey(duplicate);
    }
}

Property *QrealAnimator::prp_makeDuplicate() {
    QrealAnimator *target = new QrealAnimator();
    prp_makeDuplicate(target);
    return target;
}

void QrealAnimator::qra_multCurrentValue(qreal mult) {
    qra_setCurrentValue(mCurrentValue*mult);
}

qreal QrealAnimator::qra_getSavedValue() {
    return mSavedCurrentValue;
}

QrealPoint *QrealAnimator::qra_getPointAt(
        qreal value, qreal frame,
        qreal pixelsPerFrame, qreal pixelsPerValUnit) {
    QrealPoint *point = NULL;
    QrealKey *key; foreachQK(key, anim_mKeys)
//        point = key->mousePress(frame, value,
//                                pixelsPerFrame, pixelsPerValUnit);
        if(point != NULL) {
            break;
        }
    }
    return point;
}

void QrealAnimator::addKeysInRectToList(QRectF frameValueRect,
                                        QList<QrealKey*> *keys) {
    QrealKey *key; foreachQK(key, anim_mKeys)
        if(key->isInsideRect(frameValueRect)) {
            keys->append(key);
        }
    }
}

void QrealAnimator::setIsCurrentAnimator(bool bT)
{
    anim_mIsCurrentAnimator = bT;
    if(bT) {
        qra_updateKeysPath();
    }
}
