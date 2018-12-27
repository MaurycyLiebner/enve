#include "key.h"
#include "qrealpoint.h"
#include "Animators/complexanimator.h"

Key::Key(Animator * const parentAnimator) {
    mParentAnimator = parentAnimator;
    mRelFrame = 0;
    mGraphPoint = SPtrCreate(QrealPoint)(KEY_POINT, this, 6.);
    mStartPoint = SPtrCreate(QrealPoint)(START_POINT, this, 4.);
    mEndPoint = SPtrCreate(QrealPoint)(END_POINT, this, 4.);
}

bool Key::isSelected() const { return mIsSelected; }

void Key::removeFromAnimator() {
    if(mParentAnimator == nullptr) return;
    mParentAnimator->anim_removeKey(ref<Key>());
}

Key* Key::getNextKey() const {
    return mParentAnimator->anim_getNextKey(this);
}

Key* Key::getPrevKey() const {
    return mParentAnimator->anim_getPrevKey(this);
}

bool Key::hasPrevKey() const {
    if(mParentAnimator == nullptr) return false;
    return mParentAnimator->anim_hasPrevKey(this);
}

bool Key::hasNextKey() const {
    if(mParentAnimator == nullptr) return false;
    return mParentAnimator->anim_hasNextKey(this);
}

void Key::incFrameAndUpdateParentAnimator(const int &inc,
                                          const bool &finish) {
    setRelFrameAndUpdateParentAnimator(mRelFrame + inc, finish);
}

void Key::setRelFrameAndUpdateParentAnimator(const int &relFrame,
                                          const bool &finish) {
    if(mParentAnimator == nullptr) return;
    mParentAnimator->anim_moveKeyToRelFrame(this, relFrame, finish);
}

void Key::addToSelection(QList<qptr<Animator>> &selectedAnimators) {
    if(isSelected()) return;
    if(!mParentAnimator->hasSelectedKeys()) {
        selectedAnimators.append(mParentAnimator);
    }
    mParentAnimator->addKeyToSelected(this);
}

void Key::removeFromSelection(QList<qptr<Animator>> &selectedAnimators) {
    if(isSelected()) {
        mParentAnimator->removeKeyFromSelected(this);
        if(!mParentAnimator->hasSelectedKeys()) {
            selectedAnimators.removeOne(mParentAnimator);
        }
    }
}

Animator* Key::getParentAnimator() const {
    return mParentAnimator;
}

void Key::startFrameTransform() {
    mSavedRelFrame = mRelFrame;
}

void Key::cancelFrameTransform() {
    mParentAnimator->anim_moveKeyToRelFrame(this,
                                            mSavedRelFrame,
                                            false);
}

void Key::scaleFrameAndUpdateParentAnimator(
        const int &relativeToFrame,
        const qreal &scaleFactor,
        const bool &useSavedFrame) {
    int thisRelFrame = useSavedFrame ? mSavedRelFrame : mRelFrame;
    int relativeToRelFrame =
            mParentAnimator->prp_absFrameToRelFrame(relativeToFrame);
    int newFrame = qRound(relativeToRelFrame +
                          (thisRelFrame - relativeToRelFrame)*scaleFactor);
    if(newFrame == mRelFrame) return;
    setRelFrameAndUpdateParentAnimator(newFrame);
}

void Key::setSelected(const bool &bT) {
    mIsSelected = bT;
}

void Key::finishFrameTransform() {
    if(mParentAnimator == nullptr) return;
//    mParentAnimator->addUndoRedo(
//                new ChangeKeyFrameUndoRedo(mSavedRelFrame,
//                                           mRelFrame, this));
}

int Key::getAbsFrame() const {
    return mParentAnimator->prp_relFrameToAbsFrame(mRelFrame);
}

int Key::getRelFrame() const {
    return mRelFrame;
}

void Key::setRelFrame(const int &frame) {
    if(frame == mRelFrame) return;
    mRelFrame = frame;
    if(mParentAnimator == nullptr) return;
    mParentAnimator->anim_updateKeyOnCurrrentFrame();
}

void Key::setAbsFrame(const int &frame) {
    setRelFrame(mParentAnimator->prp_absFrameToRelFrame(frame));
}

QrealPoint *Key::mousePress(const qreal &frameT,
                            const qreal &valueT,
                            const qreal &pixelsPerFrame,
                            const qreal &pixelsPerValue) {
    if(isSelected() ) {
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

void Key::updateCtrlFromCtrl(const QrealPointType &type) {
    if(mCtrlsMode == CTRLS_CORNER) return;
    QPointF fromPt;
    QPointF toPt;
    QrealPoint *targetPt;
    if(type == END_POINT) {
        fromPt = QPointF(getEndValueFrameForGraph(),
                         getEndValueForGraph());
        toPt = QPointF(getStartValueFrameForGraph(),
                       getStartValueForGraph());
        targetPt = mStartPoint.get();
    } else {
        toPt = QPointF(getEndValueFrameForGraph(),
                       getEndValueForGraph());
        fromPt = QPointF(getStartValueFrameForGraph(),
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

void Key::afterKeyChanged() {
    mParentAnimator->anim_updateAfterChangedKey(this);
}

void Key::setCtrlsMode(const CtrlsMode &mode) {
    mCtrlsMode = mode;
    QPointF pos(mRelFrame, getValueForGraph());
    QPointF startPos(getStartValueFrameForGraph(),
                     getStartValueForGraph());
    QPointF endPos(getEndValueFrameForGraph(),
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
    setStartValueFrameForGraph(startPos.x());
    setStartValueForGraph(startPos.y());
    setEndValueFrameForGraph(endPos.x());
    setEndValueForGraph(endPos.y());
}

const CtrlsMode &Key::getCtrlsMode() const {
    return mCtrlsMode;
}

void Key::drawGraphKey(QPainter *p,
                       const QColor &paintColor) const {
    if(isSelected()) {
        p->save();
        QPen pen = QPen(Qt::black, 1.5);
        pen.setCosmetic(true);

        QPen pen2 = QPen(Qt::white, .75);
        pen2.setCosmetic(true);

        QPointF thisPos = QPointF(mRelFrame, getValueForGraph());
        if(getStartEnabledForGraph()) {
            QPointF startPos = QPointF(getStartValueFrameForGraph(),
                                       getStartValueForGraph());
            p->setPen(pen);
            p->drawLine(thisPos, startPos);
            p->setPen(pen2);
            p->drawLine(thisPos, startPos);
        }
        if(getEndEnabledForGraph()) {
            QPointF endPos = QPointF(getEndValueFrameForGraph(),
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

void Key::constrainEndCtrlMaxFrame(const int &maxFrame) {
    qreal endFrame = getEndValueFrameForGraph();
    if(endFrame < maxFrame ||
            !getEndEnabledForGraph()) return;
    qreal endValue = getEndValueForGraph();
    qreal value = getValueForGraph();
    qreal newFrame = clamp(endFrame, mRelFrame, maxFrame);
    qreal change = (newFrame - mRelFrame)/(endFrame - mRelFrame);
    mEndPoint->moveTo(newFrame, change*(endValue - value) + value);
}

void Key::constrainStartCtrlMinFrame(const int &minFrame) {
    qreal startFrame = getStartValueFrameForGraph();
    if(getStartValueFrameForGraph() > minFrame ||
            !getStartEnabledForGraph()) return;
    qreal startValue = getStartValueForGraph();
    qreal value = getValueForGraph();
    qreal newFrame = clamp(startFrame, minFrame, mRelFrame);
    qreal change = (mRelFrame - newFrame)/(mRelFrame - startFrame);
    mStartPoint->moveTo(newFrame, change*(startValue - value) + value);
}

void Key::changeFrameAndValueBy(const QPointF &frameValueChange) {
    int newFrame = qRound(frameValueChange.x() + mSavedRelFrame);
    bool frameChanged = newFrame != mRelFrame;
    if(!frameChanged) return;
    if(mParentAnimator != nullptr) {
        mParentAnimator->anim_moveKeyToRelFrame(this, newFrame, false);
    } else {
        setRelFrame(newFrame);
    }
}

bool Key::isInsideRect(const QRectF &valueFrameRect) const {
    QPointF keyPoint = QPointF(getAbsFrame(), getValueForGraph());
    return valueFrameRect.contains(keyPoint);
}

void Key::makeStartAndEndSmooth() {
    qreal nextKeyVal = getNextKeyValue();
    qreal prevKeyVal = getPrevKeyValue();
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
            (getStartValueFrameForGraph() - mRelFrame)*valIncPerFrame;
    qreal newEndVal = getValueForGraph() +
            (getEndValueFrameForGraph() - mRelFrame)*valIncPerFrame;
    setStartValueForGraph(newStartVal);
    setEndValueForGraph(newEndVal);
}
