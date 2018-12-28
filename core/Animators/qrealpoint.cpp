#include "qrealpoint.h"
#include "Animators/graphkey.h"

QrealPoint::QrealPoint(const QrealPointType& type,
                       GraphKey * const parentKey,
                       const qreal &radius) {
    mRadius = radius;
    mType = type;
    mParentKey = parentKey;
}

qreal QrealPoint::getFrame() {
    if(mType == KEY_POINT) return mParentKey->getAbsFrame();
    if(mType == START_POINT) return mParentKey->getStartFrameForGraph();
    /*if(mType == END_POINT)*/ return mParentKey->getEndFrameForGraph();
}

void QrealPoint::setFrame(const qreal &frame) {
    if(mType == KEY_POINT) {
        return mParentKey->setRelFrameAndUpdateParentAnimator(qRound(frame), false);
    }
    if(mType == START_POINT) return mParentKey->setStartFrameForGraph(frame);
    if(mType == END_POINT) return mParentKey->setEndFrameForGraph(frame);
}

void QrealPoint::startFrameTransform() {
    if(mType == KEY_POINT) return mParentKey->startFrameTransform();
}

void QrealPoint::finishFrameTransform() {
    if(mType == KEY_POINT) return mParentKey->finishFrameTransform();
}

qreal QrealPoint::getValue() {
    if(mType == KEY_POINT) return mParentKey->getValueForGraph();
    if(mType == START_POINT) return mParentKey->getStartValueForGraph();
    /*if(mType == END_POINT)*/ return mParentKey->getEndValueForGraph();
}

void QrealPoint::setValue(const qreal &value) {
    if(mType == KEY_POINT) return mParentKey->setValueForGraph(value);
    if(mType == START_POINT) return mParentKey->setStartValueForGraph(value);
    if(mType == END_POINT) return mParentKey->setEndValueForGraph(value);
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
    if(!isSelected()) {
        p->setBrush(paintColor.lighter());
    }/* else {
        drawCosmeticEllipse(p, center,
                            mRadius*0.5 - 1.,
                            mRadius*0.5 - 1.);
    }*/
    drawCosmeticEllipse(p, center,
                        mRadius - 1.,
                        mRadius - 1.);
}

void QrealPoint::setSelected(const bool &bT) {
    if(mType == KEY_POINT) {
        Q_ASSERT(false); // key selection handled differently
        mParentKey->setSelected(bT);
    } else {
        mIsSelected = bT;
    }
}

bool QrealPoint::isKeyPoint() { return mType == KEY_POINT; }

bool QrealPoint::isStartPoint() { return mType == START_POINT; }

bool QrealPoint::isEndPoint() { return mType == END_POINT; }

bool QrealPoint::isEnabled() {
    if(isKeyPoint()) return true;
    if(isStartPoint()) return mParentKey->getStartEnabledForGraph();
    /*if(isEndPoint())*/ return mParentKey->getEndEnabledForGraph();
}

GraphKey *QrealPoint::getParentKey() {
    return mParentKey;
}
