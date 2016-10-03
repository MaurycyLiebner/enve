#include "qrealkey.h"
#include "qrealanimator.h"
#include "complexanimator.h"

QrealKey::QrealKey(int frame, QrealAnimator *parentAnimator, qreal value) :
    QrealPoint(KEY_POINT, this) {
    mParentAnimator = parentAnimator;
    mFrame = frame;
    mEndFrame = frame + 5;
    mStartFrame = frame - 5;
    mValue = value;
    mStartValue = value;
    mEndValue = value;

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
    mParentAnimator->removeKey(this);
}

void QrealKey::setParentKey(ComplexKey *parentKey)
{
    mParentKey = parentKey;
}

bool QrealKey::isAncestorSelected()
{
    if(mParentKey == NULL) return isSelected();
    return isSelected() || mParentKey->isAncestorSelected();
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
    return mParentAnimator->getPrevKeyValue(this);
}

qreal QrealKey::getNextKeyValue()
{
    return mParentAnimator->getNextKeyValue(this);
}

bool QrealKey::hasPrevKey()
{
    return mParentAnimator->hasPrevKey(this);
}

bool QrealKey::hasNextKey()
{
    return mParentAnimator->hasNextKey(this);
}

void QrealKey::constrainStartCtrlMinFrame(int minFrame) {
    if(mStartFrame > minFrame || !mStartEnabled) return;
    qreal newFrame = clamp(mStartFrame, minFrame, mFrame);
    qreal change = (mFrame - newFrame)/(mFrame - mStartFrame);
    mStartPoint->moveTo(newFrame, change*(mStartValue - mValue) + mValue);
}

QrealPoint *QrealKey::mousePress(qreal frameT, qreal valueT,
                          qreal pixelsPerFrame, qreal pixelsPerValue)
{
    if(isSelected() ) {
        if( (mStartEnabled) ?
            mStartPoint->isNear(frameT, valueT, pixelsPerFrame, pixelsPerValue) :
            false ) {
            return mStartPoint;
        }
        if((mEndEnabled) ?
            mEndPoint->isNear(frameT, valueT, pixelsPerFrame, pixelsPerValue) :
            false ) {
            return mEndPoint;
        }
    }
    if(isNear(frameT, valueT, pixelsPerFrame, pixelsPerValue)) {
        return this;
    }
    return NULL;
}

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

void QrealKey::setValue(qreal value) {
    value = clamp(value,
                  mParentAnimator->getMinPossibleValue(),
                  mParentAnimator->getMaxPossibleValue());
    qreal dVal = value - mValue;
    setEndValue(mEndValue + dVal);
    setStartValue(mStartValue + dVal);
    mValue = value;
}

void QrealKey::incFrameAndUpdateParentAnimator(int inc) {
    if((mParentKey == NULL) ? false : mParentKey->isAncestorSelected() ) return;
    mParentAnimator->moveKeyToFrame(this, mFrame + inc);
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

int QrealKey::getFrame() { return mFrame; }

void QrealKey::setFrame(int frame) {
    if(frame == mFrame) return;
    int dFrame = frame - mFrame;
    setEndFrame(mEndFrame + dFrame);
    setStartFrame(mStartFrame + dFrame);
    mFrame = frame;
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
    QrealPoint::draw(p, minFrameT, minValueT, pixelsPerFrame, pixelsPerValue);
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
