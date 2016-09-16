#include "qrealpoint.h"
#include "qrealkey.h"

QrealPoint::QrealPoint(QrealPointType type, QrealKey *parentKey, qreal radius) :
SmartPointerTarget() {
    mRadius = radius;
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
    if(isKeyPoint() ) return;
    mParentKey->updateCtrlFromCtrl(mType);
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

bool QrealPoint::isEnabled()
{
    if(isKeyPoint() ) return true;
    if(isStartPoint() ) return mParentKey->isStartPointEnabled();
    if(isEndPoint() ) return mParentKey->isEndPointEnabled();
}

QrealKey *QrealPoint::getParentKey()
{
    return mParentKey;
}
