#include "qrealkey.h"
#include "Animators/qrealanimator.h"
#include "Animators/complexanimator.h"
#include "clipboardcontainer.h"
#include "qrealpoint.h"

QrealKey::QrealKey(QrealAnimator *parentAnimator) :
    Key(parentAnimator) {
    mEndFrame = mRelFrame + 5;
    mStartFrame = mRelFrame - 5;
    mValue = 0.;
    mStartValue = mValue;
    mEndValue = mValue;

    mStartPoint = new QrealPoint(START_POINT, this, 7.5);
    mEndPoint = new QrealPoint(END_POINT, this, 7.5);
}

QrealKey::QrealKey(const int &frame,
                   const qreal &val,
                   QrealAnimator *parentAnimator) :
    Key(parentAnimator) {
    mValue = val;
    mRelFrame = frame;
    mEndFrame = mRelFrame + 5;
    mStartFrame = mRelFrame - 5;
    mStartValue = mValue;
    mEndValue = mValue;

    mStartPoint = new QrealPoint(START_POINT, this, 7.5);
    mEndPoint = new QrealPoint(END_POINT, this, 7.5);
}

QrealKey::~QrealKey() {
}

Key *QrealKey::makeKeyDuplicate(Animator *animator) {
    return makeQrealKeyDuplicate((QrealAnimator*)animator);
}

QrealKey *QrealKey::makeQrealKeyDuplicate(QrealAnimator *targetParent) {
    QrealKey *target = new QrealKey(targetParent);
    target->setValue(mValue);
    target->setRelFrame(mRelFrame);
    target->setCtrlsMode(mCtrlsMode);
    target->setStartEnabled(mStartEnabled);
    target->setStartFrame(mStartFrame);
    target->setStartValue(mStartValue);
    target->setEndEnabled(mEndEnabled);
    target->setEndFrame(mEndFrame);
    target->setEndValue(mEndValue);
    //targetParent->appendKey(target);
    return target;
}

#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
int QrealKey::saveToSql(int parentAnimatorSqlId) {
    QSqlQuery query;
    if(!query.exec(
        QString("INSERT INTO qrealkey (value, frame, endenabled, "
                "startenabled, ctrlsmode, endvalue, endframe, startvalue, "
                "startframe, qrealanimatorid) "
                "VALUES (%1, %2, %3, %4, %5, %6, %7, %8, %9, %10)").
                arg(mValue, 0, 'f').
                arg(mRelFrame).
                arg(boolToSql(mEndEnabled)).
                arg(boolToSql(mStartEnabled)).
                arg(mCtrlsMode).
                arg(mEndValue, 0, 'f').
                arg(mEndFrame).
                arg(mStartValue, 0, 'f').
                arg(mStartFrame).
                arg(parentAnimatorSqlId) ) ) {
        qDebug() << query.lastError() << endl << query.lastQuery();
    }

    return query.lastInsertId().toInt();
}

#include <QSqlRecord>
void QrealKey::loadFromSql(int keyId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM qrealkey WHERE id = " +
            QString::number(keyId);
    if(query.exec(queryStr)) {
        query.next();
        int idValue = query.record().indexOf("value");
        int idFrame = query.record().indexOf("frame");
        int idEndEnabled = query.record().indexOf("endenabled");
        int idStartEnabled = query.record().indexOf("startenabled");
        int idCtrlsMode = query.record().indexOf("ctrlsmode");
        int idEndValue = query.record().indexOf("endvalue");
        int idEndFrame = query.record().indexOf("endframe");
        int idStartValue = query.record().indexOf("startvalue");
        int idStartFrame = query.record().indexOf("startframe");

        mValue = query.value(idValue).toReal();
        mRelFrame = query.value(idFrame).toInt();
        mEndEnabled = query.value(idEndEnabled).toBool();
        mStartEnabled = query.value(idStartEnabled).toBool();
        mCtrlsMode = static_cast<CtrlsMode>(query.value(idCtrlsMode).toInt());
        mEndValue = query.value(idEndValue).toReal();
        mEndFrame = query.value(idEndFrame).toInt();
        mStartValue = query.value(idStartValue).toReal();
        mStartFrame = query.value(idStartFrame).toInt();
    } else {
        qDebug() << "Could not load qrealkey with id " << keyId;
    }
}

void QrealKey::copyToContainer(KeysClipboardContainer *container) {
    container->copyKeyToContainer(this);
}

void QrealKey::constrainEndCtrlMaxFrame(int maxFrame) {
    if(mEndFrame < maxFrame || !mEndEnabled) return;
    qreal newFrame = clamp(mEndFrame, mRelFrame, maxFrame);
    qreal change = (newFrame - mRelFrame)/(mEndFrame - mRelFrame);
    mEndPoint->moveTo(newFrame, change*(mEndValue - mValue) + mValue);
}

void QrealKey::incValue(qreal incBy) {
    setValue(mValue + incBy);
}

CtrlsMode QrealKey::getCtrlsMode() {
    return mCtrlsMode;
}

QrealPoint *QrealKey::getStartPoint() {
    return mStartPoint;
}

QrealPoint *QrealKey::getEndPoint() {
    return mEndPoint;
}

bool QrealKey::isEndPointEnabled() {
    return mEndEnabled;
}

bool QrealKey::isStartPointEnabled() {
    return mStartEnabled;
}

QrealAnimator *QrealKey::getParentQrealAnimator() {
    return ((QrealAnimator*)mParentAnimator);
}

qreal QrealKey::getPrevKeyValue() {
    if(mParentAnimator == NULL) return mValue;
    return getParentQrealAnimator()->qra_getPrevKeyValue(this);
}

qreal QrealKey::getNextKeyValue() {
    if(mParentAnimator == NULL) return mValue;
    return getParentQrealAnimator()->qra_getNextKeyValue(this);
}

void QrealKey::constrainStartCtrlMinFrame(int minFrame) {
    if(mStartFrame > minFrame || !mStartEnabled) return;
    qreal newFrame = clamp(mStartFrame, minFrame, mRelFrame);
    qreal change = (mRelFrame - newFrame)/(mRelFrame - mStartFrame);
    mStartPoint->moveTo(newFrame, change*(mStartValue - mValue) + mValue);
}

//bool QrealKey::isNear(qreal frameT, qreal valueT,
//                        qreal pixelsPerFrame, qreal pixelsPerValue) {
//    qreal value = getValue();
//    qreal frame = getFrame();
//    if(qAbs(frameT - frame)*pixelsPerFrame > mRadius) return false;
//    if(qAbs(valueT - value)*pixelsPerValue > mRadius) return false;
//    return true;
//}

//QrealPoint *QrealKey::mousePress(qreal frameT, qreal valueT,
//                          qreal pixelsPerFrame, qreal pixelsPerValue)
//{
//    if(isSelected() ) {
//        if( (mStartEnabled) ?
//            mStartPoint->isNear(frameT, valueT, pixelsPerFrame, pixelsPerValue) :
//            false ) {
//            return mStartPoint;
//        }
//        if((mEndEnabled) ?
//            mEndPoint->isNear(frameT, valueT, pixelsPerFrame, pixelsPerValue) :
//            false ) {
//            return mEndPoint;
//        }
//    }
//    if(isNear(frameT, valueT, pixelsPerFrame, pixelsPerValue)) {
//        return this;
//    }
//    return NULL;
//}

void QrealKey::setCtrlsMode(CtrlsMode mode) {
    mCtrlsMode = mode;
    if(mCtrlsMode == CtrlsMode::CTRLS_SYMMETRIC) {
        QPointF newStartPos;
        QPointF newEndPos;
        getCtrlsSymmetricPos(QPointF(mEndFrame, mEndValue),
                             QPointF(mStartFrame, mStartValue),
                             QPointF(mRelFrame, mValue),
                             &newEndPos,
                             &newStartPos);
        mStartFrame = newStartPos.x();
        mStartValue = newStartPos.y();
        mEndFrame = newEndPos.x();
        mEndValue = newEndPos.y();

    } else if(mCtrlsMode == CtrlsMode::CTRLS_SMOOTH) {
        QPointF newStartPos;
        QPointF newEndPos;
        getCtrlsSmoothPos(QPointF(mEndFrame, mEndValue),
                          QPointF(mStartFrame, mStartValue),
                          QPointF(mRelFrame, mValue),
                          &newEndPos,
                          &newStartPos);
        mStartFrame = newStartPos.x();
        mStartValue = newStartPos.y();
        mEndFrame = newEndPos.x();
        mEndValue = newEndPos.y();
    }
}

void QrealKey::updateCtrlFromCtrl(QrealPointType type) {
    if(mCtrlsMode == CTRLS_CORNER) return;
    QPointF fromPt;
    QPointF toPt;
    QrealPoint *targetPt;
    if(type == END_POINT) {
        fromPt = QPointF(mEndFrame, mEndValue);
        toPt = QPointF(mStartFrame, mStartValue);
        targetPt = mStartPoint;
    } else {
        toPt = QPointF(mEndFrame, mEndValue);
        fromPt = QPointF(mStartFrame, mStartValue);
        targetPt = mEndPoint;
    }
    QPointF newFrameValue;
    if(mCtrlsMode == CTRLS_SMOOTH) {
        // mFrame and mValue are of different units chence len is wrong
        newFrameValue = symmetricToPosNewLen(
            fromPt,
            QPointF(mRelFrame, mValue),
            pointToLen(toPt -
                       QPointF(mRelFrame, mValue)) );

    } else if(mCtrlsMode == CTRLS_SYMMETRIC) {
        newFrameValue = symmetricToPos(
            fromPt,
            QPointF(mRelFrame, mValue));
    }
    targetPt->setValue(newFrameValue.y() );
    targetPt->setFrame(newFrameValue.x() );
}

qreal QrealKey::getValue() { return mValue; }

#include "undoredo.h"
void QrealKey::setValue(qreal value, bool saveUndoRedo) {
    if(mParentAnimator != NULL) {
        value = clamp(value,
                      getParentQrealAnimator()->getMinPossibleValue(),
                      getParentQrealAnimator()->getMaxPossibleValue());
    }
    qreal dVal = value - mValue;
    setEndValue(mEndValue + dVal);
    setStartValue(mStartValue + dVal);
    if(saveUndoRedo) {
        if(mParentAnimator != NULL) {
            mParentAnimator->addUndoRedo(
                        new ChangeQrealKeyValueUndoRedo(mValue, value, this) );
        }
    }
    mValue = value;
    mParentAnimator->anim_updateAfterChangedKey(this);
}

void QrealKey::incFrameAndUpdateParentAnimator(int inc) {
    if(mParentAnimator == NULL) return;
    mParentAnimator->anim_moveKeyToFrame(this, mRelFrame + inc);
}

void QrealKey::setStartValue(qreal value) {
    mStartValue = value;
}

void QrealKey::setEndValue(qreal value)
{
    mEndValue = value;
}


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
    return mRelFrame;
}

qreal QrealKey::getEndValueFrame() {
    if(mEndEnabled) return mEndFrame;
    return mRelFrame;
}

void QrealKey::setStartEnabled(bool bT) {
    mStartEnabled = bT;
}

void QrealKey::setEndEnabled(bool bT) {
    mEndEnabled = bT;
}

bool QrealKey::isInsideRect(QRectF valueFrameRect)
{
    QPointF keyPoint = QPointF(getAbsFrame(), getValue());
    return valueFrameRect.contains(keyPoint);
}

void QrealKey::drawGraphKey(QPainter *p,
                    qreal minFrameT, qreal minValueT,
                    qreal pixelsPerFrame, qreal pixelsPerValue)
{
    if(isSelected()) {
        p->save();
        p->setPen(QPen(Qt::black, 2., Qt::DotLine));
        QPointF thisPos = QPointF((mRelFrame - minFrameT + 0.5)*pixelsPerFrame,
                                  (minValueT - mValue)*pixelsPerValue);
        if(mStartEnabled) {
            p->drawLine(thisPos,
                        QPointF((mStartFrame - minFrameT + 0.5)*pixelsPerFrame,
                                (minValueT - mStartValue)*pixelsPerValue));
        }
        if(mEndEnabled) {
            p->drawLine(thisPos,
                        QPointF((mEndFrame - minFrameT + 0.5)*pixelsPerFrame,
                                (minValueT - mEndValue)*pixelsPerValue));
        }
        p->restore();
    }
//    QrealPoint::draw(p, minFrameT, minValueT, pixelsPerFrame, pixelsPerValue);
    if(isSelected() ) {
        if(mStartEnabled) {
            mStartPoint->draw(p, minFrameT, minValueT, pixelsPerFrame, pixelsPerValue);
        }
        if(mEndEnabled) {
            mEndPoint->draw(p, minFrameT, minValueT, pixelsPerFrame, pixelsPerValue);
        }
    }
}

void QrealKey::saveCurrentFrameAndValue() {
    mSavedRelFrame = getRelFrame();
    mSavedValue = getValue();
}

void QrealKey::setRelFrame(int frame) {
    if(frame == mRelFrame) return;
    int dFrame = frame - mRelFrame;
    setEndFrame(mEndFrame + dFrame);
    setStartFrame(mStartFrame + dFrame);
    mRelFrame = frame;
    if(mParentAnimator == NULL) return;
    mParentAnimator->anim_updateKeyOnCurrrentFrame();
}

bool QrealKey::differsFromKey(Key *key) {
    QrealKey *qa_key = (QrealKey*)key;
    if(qa_key->getValue() == mValue) {
        if(key->getRelFrame() > mRelFrame) {
            if(qa_key->isStartPointEnabled() ||
               isEndPointEnabled()) return true;
        } else {
            if(qa_key->isEndPointEnabled() ||
               isStartPointEnabled()) return true;
        }
        return false;
    }
    return true;
}

void QrealKey::changeFrameAndValueBy(QPointF frameValueChange) {
    setValue(frameValueChange.y() + mSavedValue);
    int newFrame = qRound(frameValueChange.x() + mSavedRelFrame);
    if(mParentAnimator != NULL) {
        mParentAnimator->anim_moveKeyToFrame(this, newFrame);
    } else {
        setRelFrame(newFrame);
    }
}

KeyCloner *QrealKey::createNewKeyCloner() {
    return new QrealKeyCloner(this);
}

QrealKeyCloner::QrealKeyCloner(QrealKey *key) : KeyCloner(key) {
    mValue = key->getValue();
}

Key *QrealKeyCloner::createKeyForAnimator(Animator *parentAnimator) {
    QrealKey *key = new QrealKey(mRelFrame,
                                 mValue,
                                 (QrealAnimator*)parentAnimator);
    parentAnimator->anim_appendKey(key);
    return key;
}
