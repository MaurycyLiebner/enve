#include "qrealpoint.h"
#include "qrealkey.h"

QrealPoint::QrealPoint(QrealPointType type,
                       QrealKey *parentKey,
                       const qreal &radius) {
    mRadius = radius;
    mType = type;
    mParentKey = parentKey;
}

qreal QrealPoint::getFrame() {
    if(mType == KEY_POINT) return mParentKey->getAbsFrame();
    if(mType == START_POINT) return mParentKey->getStartValueFrame();
    /*if(mType == END_POINT)*/ return mParentKey->getEndValueFrame();
}

void QrealPoint::setFrame(const qreal &frame) {
    if(mType == KEY_POINT) {
        return mParentKey->setRelFrameAndUpdateParentAnimator(qRound(frame),
                                                           false);
    }
    if(mType == START_POINT) return mParentKey->setStartFrame(frame);
    if(mType == END_POINT) return mParentKey->setEndFrame(frame);
}

void QrealPoint::startFrameTransform() {
    if(mType == KEY_POINT) return mParentKey->startFrameTransform();
}

void QrealPoint::finishFrameTransform() {
    if(mType == KEY_POINT) return mParentKey->finishFrameTransform();
}

qreal QrealPoint::getValue() {
    if(mType == KEY_POINT) return mParentKey->getValue();
    if(mType == START_POINT) return mParentKey->getStartValue();
    /*if(mType == END_POINT)*/ return mParentKey->getEndValue();
}

void QrealPoint::setValue(const qreal &value) {
    if(mType == KEY_POINT) return mParentKey->setValue(value, false);
    if(mType == START_POINT) return mParentKey->setStartValue(value);
    if(mType == END_POINT) return mParentKey->setEndValue(value);
}

bool QrealPoint::isSelected() {
    if(mType == KEY_POINT) return mParentKey->isSelected();
    return mIsSelected;
}

bool QrealPoint::isNear(const qreal &frameT,
                        const qreal &valueT,
                        const qreal &pixelsPerFrame,
                        const qreal &pixelsPerValue) {
    qreal value = getValue();
    qreal frame = getFrame();
    if(qAbs(frameT - frame)*pixelsPerFrame > mRadius) return false;
    if(qAbs(valueT - value)*pixelsPerValue > mRadius) return false;
    return true;
}

void QrealPoint::moveTo(const qreal &frameT, const qreal &valueT) {
    setFrame(frameT);
    setValue(valueT);
    if(isKeyPoint() ) return;
    mParentKey->updateCtrlFromCtrl(mType);
}

void QrealPoint::draw(QPainter *p, const QColor &paintColor) {
    QPointF center = QPointF(getFrame(), getValue());

    p->setBrush(Qt::black);
    drawCosmeticEllipse(p, center,
                        mRadius,
                        mRadius);

    p->setBrush(paintColor);
    if(isSelected()) {
        drawCosmeticEllipse(p, center,
                            mRadius - 1.,
                            mRadius - 1.);
    } else {
        drawCosmeticEllipse(p, center,
                            mRadius*0.5 - 1.,
                            mRadius*0.5 - 1.);
    }
}

void QrealPoint::setSelected(const bool &bT) {
    if(mType == KEY_POINT) {
        mParentKey->setSelected(bT);
    } else {
        mIsSelected = bT;
    }
}

bool QrealPoint::isKeyPoint() { return mType == KEY_POINT; }

bool QrealPoint::isStartPoint() { return mType == START_POINT; }

bool QrealPoint::isEndPoint() { return mType == END_POINT; }

bool QrealPoint::isEnabled() {
    if(isKeyPoint() ) return true;
    if(isStartPoint() ) return mParentKey->isStartPointEnabled();
    /*if(isEndPoint() )*/ return mParentKey->isEndPointEnabled();
}

QrealKey *QrealPoint::getParentKey()
{
    return mParentKey;
}
