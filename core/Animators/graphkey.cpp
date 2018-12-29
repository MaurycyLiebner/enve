#include "graphkey.h"
#include "qrealpoint.h"

GraphKey::GraphKey(const int &frame,
                   Animator * const parentAnimator) :
    Key(parentAnimator) {
    mRelFrame = frame;
    mEndFrame = mRelFrame + 5;
    mStartFrame = mRelFrame - 5;
    mGraphPoint = SPtrCreate(QrealPoint)(KEY_POINT, this, 6.);
    mStartPoint = SPtrCreate(QrealPoint)(START_POINT, this, 4.);
    mEndPoint = SPtrCreate(QrealPoint)(END_POINT, this, 4.);
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
        fromPt = QPointF(getEndFrame(),
                         getEndValue());
        toPt = QPointF(getStartFrame(),
                       getStartValue());
        targetPt = mStartPoint.get();
    } else {
        toPt = QPointF(getEndFrame(),
                       getEndValue());
        fromPt = QPointF(getStartFrame(),
                         getStartValue());
        targetPt = mEndPoint.get();
    }
    QPointF newFrameValue;
    if(mCtrlsMode == CTRLS_SMOOTH) {
        QPointF graphPt(mRelFrame, getValueForGraph());
        // mFrame and mValue are of different units chence len is wrong
        newFrameValue = symmetricToPosNewLen(
            fromPt,
            graphPt,
            pointToLen(toPt - graphPt));

    } else if(mCtrlsMode == CTRLS_SYMMETRIC) {
        newFrameValue = symmetricToPos(
            fromPt,
            QPointF(mRelFrame, getValueForGraph()));
    }
    targetPt->setValue(newFrameValue.y() );
    targetPt->setFrame(newFrameValue.x() );

    mParentAnimator->anim_updateAfterChangedKey(this);
}

void GraphKey::setCtrlsMode(const CtrlsMode &mode) {
    mCtrlsMode = mode;
    QPointF pos(mRelFrame, getValueForGraph());
    QPointF startPos(getStartFrame(),
                     getStartValue());
    QPointF endPos(getEndFrame(),
                   getEndValue());
    if(mCtrlsMode == CtrlsMode::CTRLS_SYMMETRIC) {
        getCtrlsSymmetricPos(endPos, startPos, pos,
                             &endPos, &startPos);

    } else if(mCtrlsMode == CtrlsMode::CTRLS_SMOOTH) {
        getCtrlsSmoothPos(endPos, startPos, pos,
                          &endPos, &startPos);
    } else {
        return;
    }
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
        QPen pen = QPen(Qt::black, 1.5);
        pen.setCosmetic(true);

        QPen pen2 = QPen(Qt::white, .75);
        pen2.setCosmetic(true);

        QPointF thisPos = QPointF(mRelFrame, getValueForGraph());
        if(getStartEnabledForGraph()) {
            QPointF startPos = QPointF(getStartFrame(),
                                       getStartValue());
            p->setPen(pen);
            p->drawLine(thisPos, startPos);
            p->setPen(pen2);
            p->drawLine(thisPos, startPos);
        }
        if(getEndEnabledForGraph()) {
            QPointF endPos = QPointF(getEndFrame(),
                                     getEndValue());
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

void GraphKey::constrainEndCtrlMaxFrame(const int &maxFrame) {
    qreal endFrame = getEndFrame();
    if(endFrame < maxFrame ||
            !getEndEnabledForGraph()) return;
    qreal endValue = getEndValue();
    qreal value = getValueForGraph();
    qreal newFrame = clamp(endFrame, mRelFrame, maxFrame);
    qreal change = (newFrame - mRelFrame)/(endFrame - mRelFrame);
    mEndPoint->moveTo(newFrame, change*(endValue - value) + value);
}

void GraphKey::constrainStartCtrlMinFrame(const int &minFrame) {
    qreal startFrame = getStartFrame();
    if(getStartFrame() > minFrame ||
            !getStartEnabledForGraph()) return;
    qreal startValue = getStartValue();
    qreal value = getValueForGraph();
    qreal newFrame = clamp(startFrame, minFrame, mRelFrame);
    qreal change = (mRelFrame - newFrame)/(mRelFrame - startFrame);
    mStartPoint->moveTo(newFrame, change*(startValue - value) + value);
}

void GraphKey::changeFrameAndValueBy(const QPointF &frameValueChange) {
    int newFrame = qRound(frameValueChange.x() + mSavedRelFrame);
    bool frameChanged = newFrame != mRelFrame;
    if(!frameChanged) return;
    if(mParentAnimator != nullptr) {
        mParentAnimator->anim_moveKeyToRelFrame(this, newFrame, false);
    } else {
        setRelFrame(newFrame);
    }
}

bool GraphKey::isInsideRect(const QRectF &valueFrameRect) const {
    QPointF keyPoint = QPointF(getAbsFrame(), getValueForGraph());
    return valueFrameRect.contains(keyPoint);
}

void GraphKey::makeStartAndEndSmooth() {
    qreal nextKeyVal = getNextKeyValueForGraph();
    qreal prevKeyVal = getPrevKeyValueForGraph();
    int nextKeyFrame = getNextKeyRelFrame();
    int prevKeyFrame = getPrevKeyRelFrame();
    qreal valIncPerFrame;
    if(nextKeyFrame == mRelFrame || prevKeyFrame == mRelFrame) {
        valIncPerFrame = 0.;
    } else {
        valIncPerFrame =
                (nextKeyVal - prevKeyVal)/(nextKeyFrame - prevKeyFrame);
    }
    qreal newStartVal = getValueForGraph() +
            (getStartFrame() - mRelFrame)*valIncPerFrame;
    qreal newEndVal = getValueForGraph() +
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
    int thisRelFrame = useSavedFrame ? mSavedRelFrame : mRelFrame;
    qreal startRelFrame = useSavedFrame ? mSavedStartFrame : mStartFrame;
    qreal endRelFrame = useSavedFrame ? mSavedEndFrame : mEndFrame;

    setStartFrameVar(thisRelFrame + (startRelFrame - thisRelFrame)*scaleFactor);
    setEndFrameVar(thisRelFrame + (endRelFrame - thisRelFrame)*scaleFactor);

    int relativeToRelFrame =
            mParentAnimator->prp_absFrameToRelFrame(relativeToFrame);
    int newFrame = qRound(relativeToRelFrame +
                          (thisRelFrame - relativeToRelFrame)*scaleFactor);
    if(newFrame != mRelFrame) {
        incFrameAndUpdateParentAnimator(newFrame - mRelFrame);
    } else {
        mParentAnimator->anim_updateAfterChangedKey(this);
    }
}

void GraphKey::setRelFrame(const int &frame) {
    if(frame == mRelFrame) return;
    int dFrame = frame - mRelFrame;
    setEndFrameVar(mEndFrame + dFrame);
    setStartFrameVar(mStartFrame + dFrame);
    mRelFrame = frame;
    if(mParentAnimator == nullptr) return;
    mParentAnimator->anim_updateKeyOnCurrrentFrame();
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
