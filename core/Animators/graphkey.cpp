#include "graphkey.h"
#include "qrealpoint.h"
#include "animator.h"

GraphKey::GraphKey(const int &frame,
                   Animator * const parentAnimator) :
    Key(parentAnimator) {
    mRelFrame = frame;
    mEndFrame = mRelFrame + 5;
    mStartFrame = mRelFrame - 5;
    mGraphPoint = SPtrCreate(QrealPoint)(KEY_POINT, this, 6);
    mStartPoint = SPtrCreate(QrealPoint)(START_POINT, this, 4);
    mEndPoint = SPtrCreate(QrealPoint)(END_POINT, this, 4);
}

GraphKey::GraphKey(Animator * const parentAnimator) :
    GraphKey(0, parentAnimator) {}

QrealPoint *GraphKey::mousePress(const qreal &frameT,
                            const qreal &valueT,
                            const qreal &pixelsPerFrame,
                            const qreal &pixelsPerValue) {
    if(isSelected()) {
        if((getStartEnabledForGraph() && hasPrevKey()) ?
            mStartPoint->isNear(frameT, valueT, pixelsPerFrame, pixelsPerValue) :
            false ) {
            return mStartPoint.get();
        }
        if((getEndEnabledForGraph() && hasNextKey()) ?
            mEndPoint->isNear(frameT, valueT, pixelsPerFrame, pixelsPerValue) :
            false ) {
            return mEndPoint.get();
        }
    }
    if(mGraphPoint->isNear(frameT, valueT, pixelsPerFrame, pixelsPerValue)) {
        return mGraphPoint.get();
    }
    return nullptr;
}

void GraphKey::updateCtrlFromCtrl(const QrealPointType &type) {
    if(mCtrlsMode == CTRLS_CORNER) return;
    QPointF fromPt;
    QPointF toPt;
    QrealPoint *targetPt;
    if(type == END_POINT) {
        fromPt = QPointF(getEndFrame(), getEndValue());
        toPt = QPointF(getStartFrame(), getStartValue());
        targetPt = mStartPoint.get();
    } else {
        toPt = QPointF(getEndFrame(), getEndValue());
        fromPt = QPointF(getStartFrame(), getStartValue());
        targetPt = mEndPoint.get();
    }
    QPointF newFrameValue;
    const QPointF graphPt(mRelFrame, getValueForGraph());
    if(mCtrlsMode == CTRLS_SMOOTH) {
        // mFrame and mValue are of different units chence len is wrong
        newFrameValue = symmetricToPosNewLen(fromPt, graphPt,
                                             pointToLen(toPt - graphPt));

    } else if(mCtrlsMode == CTRLS_SYMMETRIC) {
        newFrameValue = symmetricToPos(fromPt, graphPt);
    }
    targetPt->setValue(newFrameValue.y());
    targetPt->setFrame(newFrameValue.x());

    mParentAnimator->anim_updateAfterChangedKey(this);
}

void GraphKey::setCtrlsMode(const CtrlsMode &mode) {
    mCtrlsMode = mode;
    const QPointF pos(mRelFrame, getValueForGraph());
    QPointF startPos(getStartFrame(), getStartValue());
    QPointF endPos(getEndFrame(), getEndValue());
    if(mCtrlsMode == CtrlsMode::CTRLS_SYMMETRIC) {
        gGetCtrlsSymmetricPos(startPos, pos, endPos,
                              startPos, endPos);

    } else if(mCtrlsMode == CtrlsMode::CTRLS_SMOOTH) {
        gGetCtrlsSmoothPos(startPos, pos, endPos,
                           startPos, endPos);
    } else return;
    setStartFrame(startPos.x());
    setStartValueForGraph(startPos.y());
    setEndFrame(endPos.x());
    setEndValueForGraph(endPos.y());
}

const CtrlsMode &GraphKey::getCtrlsMode() const {
    return mCtrlsMode;
}

void GraphKey::drawGraphKey(QPainter *p,
                       const QColor &paintColor) const {
    if(isSelected()) {
        p->save();
        QPen pen(Qt::black, 1.5);
        pen.setCosmetic(true);

        QPen pen2(Qt::white, .75);
        pen2.setCosmetic(true);

        const QPointF thisPos(mRelFrame, getValueForGraph());
        if(getStartEnabledForGraph()) {
            const QPointF startPos(getStartFrame(), getStartValue());
            p->setPen(pen);
            p->drawLine(thisPos, startPos);
            p->setPen(pen2);
            p->drawLine(thisPos, startPos);
        }
        if(getEndEnabledForGraph()) {
            const QPointF endPos(getEndFrame(), getEndValue());
            p->setPen(pen);
            p->drawLine(thisPos, endPos);
            p->setPen(pen2);
            p->drawLine(thisPos, endPos);
        }
        p->restore();
    }
    mGraphPoint->draw(p, paintColor);
    if(isSelected()) {
        if(getStartEnabledForGraph() && hasPrevKey()) {
            mStartPoint->draw(p, paintColor);
        }
        if(getEndEnabledForGraph() && hasNextKey()) {
            mEndPoint->draw(p, paintColor);
        }
    }
}

void GraphKey::constrainEndCtrlValue(const qreal &minVal,
                                     const qreal &maxVal) {
    if(!getEndEnabledForGraph()) return;
    const qreal endValue = getEndValue();
    if(endValue > minVal && endValue < maxVal) return;
    const qreal newValue = clamp(endValue, minVal, maxVal);
    mEndPoint->moveTo(getEndFrame(), newValue);
}

void GraphKey::constrainStartCtrlValue(const qreal &minVal,
                                       const qreal &maxVal) {
    if(!getStartEnabledForGraph()) return;
    const qreal startValue = getStartValue();
    if(startValue > minVal && startValue < maxVal) return;
    const qreal newValue = clamp(startValue, minVal, maxVal);
    mStartPoint->moveTo(getStartFrame(), newValue);
}


void GraphKey::constrainEndCtrlMaxFrame(const int &maxFrame) {
    const qreal endFrame = getEndFrame();
    if(endFrame < maxFrame || !getEndEnabledForGraph()) return;
    const qreal endValue = getEndValue();
    const qreal value = getValueForGraph();
    const qreal newFrame = clamp(endFrame, qreal(mRelFrame), qreal(maxFrame));
    const qreal change = (newFrame - mRelFrame)/(endFrame - mRelFrame);
    mEndPoint->moveTo(newFrame, change*(endValue - value) + value);
}

void GraphKey::constrainStartCtrlMinFrame(const int &minFrame) {
    const qreal startFrame = getStartFrame();
    if(startFrame > minFrame || !getStartEnabledForGraph()) return;
    const qreal startValue = getStartValue();
    const qreal value = getValueForGraph();
    const qreal newFrame = clamp(startFrame, qreal(minFrame), qreal(mRelFrame));
    const qreal change = (mRelFrame - newFrame)/(mRelFrame - startFrame);
    mStartPoint->moveTo(newFrame, change*(startValue - value) + value);
}

void GraphKey::changeFrameAndValueBy(const QPointF &frameValueChange) {
    const int newFrame = qRound(frameValueChange.x() + mSavedRelFrame);
    const bool frameChanged = newFrame != mRelFrame;
    if(!frameChanged) return;
    if(mParentAnimator) {
        mParentAnimator->anim_moveKeyToRelFrame(this, newFrame);
    } else {
        setRelFrame(newFrame);
    }
}

bool GraphKey::isInsideRect(const QRectF &valueFrameRect) const {
    const QPointF keyPoint(getAbsFrame(), getValueForGraph());
    return valueFrameRect.contains(keyPoint);
}

void GraphKey::makeStartAndEndSmooth() {
    const qreal nextKeyVal = getNextKeyValueForGraph();
    const qreal prevKeyVal = getPrevKeyValueForGraph();
    const int nextKeyFrame = getNextKeyRelFrame();
    const int prevKeyFrame = getPrevKeyRelFrame();
    qreal valIncPerFrame;
    if(nextKeyFrame == mRelFrame || prevKeyFrame == mRelFrame) {
        valIncPerFrame = 0;
    } else {
        valIncPerFrame =
                (nextKeyVal - prevKeyVal)/(nextKeyFrame - prevKeyFrame);
    }
    const qreal newStartVal = getValueForGraph() +
            (getStartFrame() - mRelFrame)*valIncPerFrame;
    const qreal newEndVal = getValueForGraph() +
            (getEndFrame() - mRelFrame)*valIncPerFrame;
    setStartValueForGraph(newStartVal);
    setEndValueForGraph(newEndVal);
}


void GraphKey::setStartEnabledForGraph(const bool &bT) {
    mStartEnabled = bT;
}

void GraphKey::setEndEnabledForGraph(const bool &bT) {
    mEndEnabled = bT;
}

qreal GraphKey::getStartFrame() const {
    if(mStartEnabled) return mStartFrame;
    return mRelFrame;
}

qreal GraphKey::getEndFrame() const {
    if(mEndEnabled) return mEndFrame;
    return mRelFrame;
}

bool GraphKey::getEndEnabledForGraph() const {
    return mEndEnabled;
}

bool GraphKey::getStartEnabledForGraph() const {
    return mStartEnabled;
}

void GraphKey::setStartFrameVar(const qreal &startFrame) {
    mStartFrame = startFrame;
}

void GraphKey::setEndFrameVar(const qreal &endFrame) {
    mEndFrame = endFrame;
}

void GraphKey::setEndFrame(const qreal &endFrame) {
    setEndFrameVar(endFrame);
    mParentAnimator->anim_updateAfterChangedKey(this);
}

void GraphKey::setStartFrame(const qreal &startFrame) {
    setStartFrameVar(startFrame);
    mParentAnimator->anim_updateAfterChangedKey(this);
}

void GraphKey::scaleFrameAndUpdateParentAnimator(
        const int &relativeToFrame,
        const qreal &scaleFactor,
        const bool& useSavedFrame) {
    const int thisRelFrame = useSavedFrame ? mSavedRelFrame : mRelFrame;
    const qreal startRelFrame = useSavedFrame ? mSavedStartFrame : mStartFrame;
    const qreal endRelFrame = useSavedFrame ? mSavedEndFrame : mEndFrame;

    setStartFrameVar(thisRelFrame + (startRelFrame - thisRelFrame)*scaleFactor);
    setEndFrameVar(thisRelFrame + (endRelFrame - thisRelFrame)*scaleFactor);

    const int relativeToRelFrame =
            mParentAnimator->prp_absFrameToRelFrame(relativeToFrame);
    const int newFrame = qRound(relativeToRelFrame +
                          (thisRelFrame - relativeToRelFrame)*scaleFactor);
    if(newFrame != mRelFrame) {
        incFrameAndUpdateParentAnimator(newFrame - mRelFrame);
    } else {
        mParentAnimator->anim_updateAfterChangedKey(this);
    }
}

void GraphKey::setRelFrame(const int &frame) {
    if(frame == mRelFrame) return;
    const int dFrame = frame - mRelFrame;
    setEndFrameVar(mEndFrame + dFrame);
    setStartFrameVar(mStartFrame + dFrame);
    mRelFrame = frame;
}

void GraphKey::setStartValueVar(const qreal &value) {
    mStartValue = value;
}

void GraphKey::setEndValueVar(const qreal &value) {
    mEndValue = value;
}

void GraphKey::setStartValueForGraph(const qreal &value) {
    setStartValueVar(value);
    mParentAnimator->anim_updateAfterChangedKey(this);
}

void GraphKey::setEndValueForGraph(const qreal &value) {
    setEndValueVar(value);
    mParentAnimator->anim_updateAfterChangedKey(this);
}

qreal GraphKey::getStartValue() const {
    if(mStartEnabled) return mStartValue;
    return getValueForGraph();
}

qreal GraphKey::getEndValue() const {
    if(mEndEnabled) return mEndValue;
    return getValueForGraph();
}
