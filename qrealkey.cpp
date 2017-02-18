#include "qrealkey.h"
#include "Animators/qrealanimator.h"
#include "Animators/complexanimator.h"
#include "clipboardcontainer.h"

QrealKey::QrealKey(QrealAnimator *parentAnimator) :
    SmartPointerTarget() {
    mParentAnimator = parentAnimator;
    mFrame = 0;
    mEndFrame = mFrame + 5;
    mStartFrame = mFrame - 5;
    mValue = 0.;
    mStartValue = mValue;
    mEndValue = mValue;

    mStartPoint = new QrealPoint(START_POINT, this, 7.5);
    mStartPoint->incNumberPointers();
    mEndPoint = new QrealPoint(END_POINT, this, 7.5);
    mEndPoint->incNumberPointers();
}

QrealKey::~QrealKey()
{
    mStartPoint->decNumberPointers();
    mEndPoint->decNumberPointers();
}

QrealKey *QrealKey::makeQrealKeyDuplicate(QrealAnimator *targetParent) {
    QrealKey *target = new QrealKey(targetParent);
    target->setValue(mValue);
    target->setFrame(mFrame);
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
                arg(mFrame).
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
        mFrame = query.value(idFrame).toInt();
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

bool QrealKey::isSelected() { return mIsSelected; }

void QrealKey::copyToContainer(KeysClipboardContainer *container) {
    container->copyKeyToContainer(this);
}

void QrealKey::constrainEndCtrlMaxFrame(int maxFrame) {
    if(mEndFrame < maxFrame || !mEndEnabled) return;
    qreal newFrame = clamp(mEndFrame, mFrame, maxFrame);
    qreal change = (newFrame - mFrame)/(mEndFrame - mFrame);
    mEndPoint->moveTo(newFrame, change*(mEndValue - mValue) + mValue);
}

void QrealKey::incValue(qreal incBy) {
    setValue(mValue + incBy);
}

void QrealKey::removeFromAnimator()
{
    if(mParentAnimator == NULL) return;
    mParentAnimator->removeKey(this);
}

void QrealKey::setParentKey(ComplexKey *parentKey)
{
    mParentKey = parentKey;
}

bool QrealKey::isAncestorSelected()
{
    if(mParentKey == NULL) return isSelected();
    if(isSelected()) return true;
    return mParentKey->isAncestorSelected();
}

CtrlsMode QrealKey::getCtrlsMode()
{
    return mCtrlsMode;
}

QrealPoint *QrealKey::getStartPoint()
{
    return mStartPoint;
}

QrealPoint *QrealKey::getEndPoint()
{
    return mEndPoint;
}

bool QrealKey::isEndPointEnabled()
{
    return mEndEnabled;
}

bool QrealKey::isStartPointEnabled()
{
    return mStartEnabled;
}

qreal QrealKey::getPrevKeyValue()
{
    if(mParentAnimator == NULL) return mValue;
    return mParentAnimator->getPrevKeyValue(this);
}

qreal QrealKey::getNextKeyValue()
{
    if(mParentAnimator == NULL) return mValue;
    return mParentAnimator->getNextKeyValue(this);
}

bool QrealKey::hasPrevKey()
{
    if(mParentAnimator == NULL) return false;
    return mParentAnimator->hasPrevKey(this);
}

bool QrealKey::hasNextKey()
{
    if(mParentAnimator == NULL) return false;
    return mParentAnimator->hasNextKey(this);
}

void QrealKey::constrainStartCtrlMinFrame(int minFrame) {
    if(mStartFrame > minFrame || !mStartEnabled) return;
    qreal newFrame = clamp(mStartFrame, minFrame, mFrame);
    qreal change = (mFrame - newFrame)/(mFrame - mStartFrame);
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

void QrealKey::setCtrlsMode(CtrlsMode mode)
{
    mCtrlsMode = mode;
    if(mCtrlsMode == CtrlsMode::CTRLS_SYMMETRIC) {
        QPointF newStartPos;
        QPointF newEndPos;
        getCtrlsSymmetricPos(QPointF(mEndFrame, mEndValue),
                             QPointF(mStartFrame, mStartValue),
                             QPointF(mFrame, mValue),
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
                          QPointF(mFrame, mValue),
                          &newEndPos,
                          &newStartPos);
        mStartFrame = newStartPos.x();
        mStartValue = newStartPos.y();
        mEndFrame = newEndPos.x();
        mEndValue = newEndPos.y();
    }
}

void QrealKey::updateCtrlFromCtrl(QrealPointType type)
{
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
            QPointF(mFrame, mValue),
            pointToLen(toPt -
                       QPointF(mFrame, mValue)) );

    } else if(mCtrlsMode == CTRLS_SYMMETRIC) {
        newFrameValue = symmetricToPos(
            fromPt,
            QPointF(mFrame, mValue));
    }
    targetPt->setValue(newFrameValue.y() );
    targetPt->setFrame(newFrameValue.x() );
}

qreal QrealKey::getValue() { return mValue; }

#include "undoredo.h"
void QrealKey::setValue(qreal value, bool saveUndoRedo) {
    if(mParentAnimator != NULL) {
        value = clamp(value,
                      mParentAnimator->getMinPossibleValue(),
                      mParentAnimator->getMaxPossibleValue());
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
}

void QrealKey::incFrameAndUpdateParentAnimator(int inc) {
    if(mParentAnimator == NULL) return;
    if((mParentKey == NULL) ? false : mParentKey->isAncestorSelected() ) return;
    mParentAnimator->moveKeyToFrame(this, mFrame + inc);
}

void QrealKey::addToSelection(QList<QrealKey*> *selectedKeys) {
    if(isSelected()) return;
    setSelected(true);
    selectedKeys->append(this);
    incNumberPointers();
}

void QrealKey::removeFromSelection(QList<QrealKey *> *selectedKeys) {
    if(isSelected()) {
        setSelected(false);
        if(selectedKeys->removeOne(this) ) {
            decNumberPointers();
        }
    }
}

QrealAnimator *QrealKey::getParentAnimator()
{
    return mParentAnimator;
}

void QrealKey::setStartValue(qreal value)
{
    mStartValue = value;
}

void QrealKey::setEndValue(qreal value)
{
    mEndValue = value;
}

void QrealKey::startFrameTransform() {
    mSavedFrame = getFrame();
}

void QrealKey::cancelFrameTransform() {
    mParentAnimator->moveKeyToFrame(this, mSavedFrame);
}

void QrealKey::scaleFrameAndUpdateParentAnimator(
        const int &relativeToFrame,
        const qreal &scaleFactor) {
    int newFrame = qRound(mSavedFrame +
                          (mSavedFrame - relativeToFrame)*scaleFactor);
    if(newFrame == mFrame) return;
    incFrameAndUpdateParentAnimator(newFrame - mFrame);
}

void QrealKey::setSelected(bool bT) {
    mIsSelected = bT;
}

void QrealKey::finishFrameTransform()
{
    if(mParentAnimator == NULL) return;
    mParentAnimator->addUndoRedo(
                new ChangeQrealKeyFrameUndoRedo(mSavedFrame, mFrame, this));
}

int QrealKey::getFrame() { return mFrame; }

void QrealKey::setFrame(int frame) {
    if(frame == mFrame) return;
    int dFrame = frame - mFrame;
    setEndFrame(mEndFrame + dFrame);
    setStartFrame(mStartFrame + dFrame);
    mFrame = frame;
    if(mParentAnimator == NULL) return;
    mParentAnimator->updateKeyOnCurrrentFrame();
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

bool QrealKey::isInsideRect(QRectF valueFrameRect)
{
    QPointF keyPoint = QPointF(getFrame(), getValue());
    return valueFrameRect.contains(keyPoint);
}

void QrealKey::drawGraphKey(QPainter *p,
                    qreal minFrameT, qreal minValueT,
                    qreal pixelsPerFrame, qreal pixelsPerValue)
{
    if(isSelected()) {
        p->save();
        p->setPen(QPen(Qt::black, 2., Qt::DotLine));
        QPointF thisPos = QPointF((mFrame - minFrameT + 0.5)*pixelsPerFrame,
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
    mSavedFrame = getFrame();
    mSavedValue = getValue();
}

void QrealKey::changeFrameAndValueBy(QPointF frameValueChange)
{
    setValue(frameValueChange.y() + mSavedValue);
    int newFrame = qRound(frameValueChange.x() + mSavedFrame);
    if(mParentAnimator != NULL) {
        mParentAnimator->moveKeyToFrame(this, newFrame);
    } else {
        setFrame(newFrame);
    }
}
