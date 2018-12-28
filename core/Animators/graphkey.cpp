#include "graphkey.h"
#include "qrealpoint.h"

GraphKey::GraphKey(Animator * const parentAnimator) : Key(parentAnimator) {
    mGraphPoint = SPtrCreate(QrealPoint)(KEY_POINT, this, 6.);
    mStartPoint = SPtrCreate(QrealPoint)(START_POINT, this, 4.);
    mEndPoint = SPtrCreate(QrealPoint)(END_POINT, this, 4.);
}

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
        fromPt = QPointF(getEndFrameForGraph(),
                         getEndValueForGraph());
        toPt = QPointF(getStartFrameForGraph(),
                       getStartValueForGraph());
        targetPt = mStartPoint.get();
    } else {
        toPt = QPointF(getEndFrameForGraph(),
                       getEndValueForGraph());
        fromPt = QPointF(getStartFrameForGraph(),
                         getStartValueForGraph());
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
    QPointF startPos(getStartFrameForGraph(),
                     getStartValueForGraph());
    QPointF endPos(getEndFrameForGraph(),
                   getEndValueForGraph());
    if(mCtrlsMode == CtrlsMode::CTRLS_SYMMETRIC) {
        getCtrlsSymmetricPos(endPos, startPos, pos,
                             &endPos, &startPos);

    } else if(mCtrlsMode == CtrlsMode::CTRLS_SMOOTH) {
        getCtrlsSmoothPos(endPos, startPos, pos,
                          &endPos, &startPos);
    } else {
        return;
    }
    setStartFrameForGraph(startPos.x());
    setStartValueForGraph(startPos.y());
    setEndFrameForGraph(endPos.x());
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
            QPointF startPos = QPointF(getStartFrameForGraph(),
                                       getStartValueForGraph());
            p->setPen(pen);
            p->drawLine(thisPos, startPos);
            p->setPen(pen2);
            p->drawLine(thisPos, startPos);
        }
        if(getEndEnabledForGraph()) {
            QPointF endPos = QPointF(getEndFrameForGraph(),
                                     getEndValueForGraph());
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
    qreal endFrame = getEndFrameForGraph();
    if(endFrame < maxFrame ||
            !getEndEnabledForGraph()) return;
    qreal endValue = getEndValueForGraph();
    qreal value = getValueForGraph();
    qreal newFrame = clamp(endFrame, mRelFrame, maxFrame);
    qreal change = (newFrame - mRelFrame)/(endFrame - mRelFrame);
    mEndPoint->moveTo(newFrame, change*(endValue - value) + value);
}

void GraphKey::constrainStartCtrlMinFrame(const int &minFrame) {
    qreal startFrame = getStartFrameForGraph();
    if(getStartFrameForGraph() > minFrame ||
            !getStartEnabledForGraph()) return;
    qreal startValue = getStartValueForGraph();
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
            (getStartFrameForGraph() - mRelFrame)*valIncPerFrame;
    qreal newEndVal = getValueForGraph() +
            (getEndFrameForGraph() - mRelFrame)*valIncPerFrame;
    setStartValueForGraph(newStartVal);
    setEndValueForGraph(newEndVal);
}
