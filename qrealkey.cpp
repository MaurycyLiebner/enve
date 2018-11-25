#include "qrealkey.h"
#include "Animators/qrealanimator.h"
#include "Animators/complexanimator.h"
#include "clipboardcontainer.h"
#include "qrealpoint.h"

QrealKey::QrealKey(const int &frame,
                   const qreal &val,
                   QrealAnimator* parentAnimator) :
    Key(parentAnimator) {
    mValue = val;
    mRelFrame = frame;
    mEndFrame = mRelFrame + 5;
    mStartFrame = mRelFrame - 5;
    mStartValue = mValue;
    mEndValue = mValue;

    mGraphPoint = SPtrCreate(QrealPoint)(KEY_POINT, this, 6.);
    mStartPoint = SPtrCreate(QrealPoint)(START_POINT, this, 4.);
    mEndPoint = SPtrCreate(QrealPoint)(END_POINT, this, 4.);
}

QrealKey::QrealKey(QrealAnimator *parentAnimator) :
    QrealKey(0, 0., parentAnimator) { }

QrealKeySPtr QrealKey::makeQrealKeyDuplicate(
        QrealAnimator* targetParent) {
    QrealKeySPtr target = SPtrCreate(QrealKey)(targetParent);
    target->setValue(mValue);
    target->setRelFrame(mRelFrame);
    target->setCtrlsMode(mCtrlsMode);
    target->setStartEnabled(mStartEnabled);
    target->setStartFrameVar(mStartFrame);
    target->setStartValueVar(mStartValue);
    target->setEndEnabled(mEndEnabled);
    target->setEndFrameVar(mEndFrame);
    target->setEndValueVar(mEndValue);
    //targetParent->appendKey(target);
    return target;
}

void QrealKey::constrainEndCtrlMaxFrame(const int &maxFrame) {
    if(mEndFrame < maxFrame || !mEndEnabled) return;
    qreal newFrame = clamp(mEndFrame, mRelFrame, maxFrame);
    qreal change = (newFrame - mRelFrame)/(mEndFrame - mRelFrame);
    mEndPoint->moveTo(newFrame, change*(mEndValue - mValue) + mValue);
}

void QrealKey::incValue(const qreal &incBy,
                        const bool &saveUndoRedo,
                        const bool &finish,
                        const bool &callUpdater) {
    setValue(mValue + incBy, saveUndoRedo,
             finish, callUpdater);
}

CtrlsMode QrealKey::getCtrlsMode() {
    return mCtrlsMode;
}

QrealPoint *QrealKey::getStartPoint() {
    return mStartPoint.get();
}

QrealPoint *QrealKey::getEndPoint() {
    return mEndPoint.get();
}

QrealPoint *QrealKey::getGraphPoint() {
    return mGraphPoint.get();
}

bool QrealKey::isEndPointEnabled() {
    return mEndEnabled;
}

bool QrealKey::isStartPointEnabled() {
    return mStartEnabled;
}

QrealAnimator *QrealKey::getParentQrealAnimator() {
    return static_cast<QrealAnimator*>(mParentAnimator.data());
}

qreal QrealKey::getPrevKeyValue() {
    if(mParentAnimator == nullptr) return mValue;
    return getParentQrealAnimator()->qra_getPrevKeyValue(this);
}

qreal QrealKey::getNextKeyValue() {
    if(mParentAnimator == nullptr) return mValue;
    return getParentQrealAnimator()->qra_getNextKeyValue(this);
}

void QrealKey::constrainStartCtrlMinFrame(const int &minFrame) {
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

QrealPoint *QrealKey::mousePress(const qreal &frameT,
                                 const qreal &valueT,
                                 const qreal &pixelsPerFrame,
                                 const qreal &pixelsPerValue) {
    if(isSelected() ) {
        if( (mStartEnabled && hasPrevKey()) ?
            mStartPoint->isNear(frameT, valueT, pixelsPerFrame, pixelsPerValue) :
            false ) {
            return mStartPoint.get();
        }
        if((mEndEnabled && hasNextKey()) ?
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

void QrealKey::setCtrlsMode(const CtrlsMode &mode) {
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

void QrealKey::updateCtrlFromCtrl(const QrealPointType &type) {
    if(mCtrlsMode == CTRLS_CORNER) return;
    QPointF fromPt;
    QPointF toPt;
    QrealPoint *targetPt;
    if(type == END_POINT) {
        fromPt = QPointF(mEndFrame, mEndValue);
        toPt = QPointF(mStartFrame, mStartValue);
        targetPt = mStartPoint.get();
    } else {
        toPt = QPointF(mEndFrame, mEndValue);
        fromPt = QPointF(mStartFrame, mStartValue);
        targetPt = mEndPoint.get();
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

    mParentAnimator->anim_updateAfterChangedKey(this);
}

qreal QrealKey::getValue() { return mValue; }

#include "undoredo.h"
void QrealKey::setValue(qreal value,
                        const bool &saveUndoRedo,
                        const bool &finish,
                        const bool &callUpdater) {
    if(mParentAnimator != nullptr) {
        value = clamp(value,
                      getParentQrealAnimator()->getMinPossibleValue(),
                      getParentQrealAnimator()->getMaxPossibleValue());
    }
    qreal dVal = value - mValue;
    setEndValueVar(mEndValue + dVal);
    setStartValueVar(mStartValue + dVal);
    if(saveUndoRedo) {
        if(mParentAnimator != nullptr) {
//            mParentAnimator->addUndoRedo(
//                    new ChangeQrealKeyValueUndoRedo(mValue, value, this) );
        }
    }
    mValue = value;
    if(finish && callUpdater) {
        mParentAnimator->anim_updateAfterChangedKey(this);
    }
}

void QrealKey::finishValueTransform() {
    if(mParentAnimator != nullptr) {
//        mParentAnimator->addUndoRedo(
//                    new ChangeQrealKeyValueUndoRedo(mSavedValue,
//                                                    mValue, this) );
        mParentAnimator->anim_updateAfterChangedKey(this);
    }
}

void QrealKey::startValueTransform() {
    mSavedValue = mValue;
}

void QrealKey::setStartValueVar(const qreal &value) {
    mStartValue = value;
}

void QrealKey::setEndValueVar(const qreal &value) {
    mEndValue = value;
}

void QrealKey::setStartFrameVar(const qreal &startFrame) {
    mStartFrame = startFrame;
}

void QrealKey::setEndFrameVar(const qreal &endFrame) {
    mEndFrame = endFrame;
}

void QrealKey::setEndFrame(const qreal &endFrame) {
    setEndFrameVar(endFrame);
    mParentAnimator->anim_updateAfterChangedKey(this);
}

void QrealKey::setStartFrame(const qreal &startFrame) {
    setStartFrameVar(startFrame);
    mParentAnimator->anim_updateAfterChangedKey(this);
}

void QrealKey::setStartValue(const qreal &value) {
    setStartValueVar(value);
    mParentAnimator->anim_updateAfterChangedKey(this);
}

void QrealKey::setEndValue(const qreal &value) {
    setEndValueVar(value);
    mParentAnimator->anim_updateAfterChangedKey(this);
}

void QrealKey::scaleFrameAndUpdateParentAnimator(
        const int &relativeToFrame, const qreal &scaleFactor,
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

void QrealKey::setStartEnabled(const bool &bT) {
    mStartEnabled = bT;
}

void QrealKey::setEndEnabled(const bool &bT) {
    mEndEnabled = bT;
}

bool QrealKey::isInsideRect(const QRectF &valueFrameRect) {
    QPointF keyPoint = QPointF(getAbsFrame(), getValue());
    return valueFrameRect.contains(keyPoint);
}

void QrealKey::drawGraphKey(QPainter *p,
                            const QColor &paintColor) {
    if(isSelected()) {
        p->save();
        QPen pen = QPen(Qt::black, 1.5);
        pen.setCosmetic(true);

        QPen pen2 = QPen(Qt::white, .75);
        pen2.setCosmetic(true);
        p->setPen(pen);
        QPointF thisPos = QPointF(mRelFrame, mValue);
        if(mStartEnabled) {
            p->drawLine(thisPos,
                        QPointF(mStartFrame, mStartValue));
        }
        if(mEndEnabled) {
            p->drawLine(thisPos,
                        QPointF(mEndFrame, mEndValue));
        }
        p->setPen(pen2);
        if(mStartEnabled) {
            p->drawLine(thisPos,
                        QPointF(mStartFrame, mStartValue));
        }
        if(mEndEnabled) {
            p->drawLine(thisPos,
                        QPointF(mEndFrame, mEndValue));
        }
        p->restore();
    }
    mGraphPoint->draw(p, paintColor);
    if(isSelected() ) {
        if(mStartEnabled && hasPrevKey()) {
            mStartPoint->draw(p, paintColor);
        }
        if(mEndEnabled && hasNextKey()) {
            mEndPoint->draw(p, paintColor);
        }
    }
}

void QrealKey::saveCurrentFrameAndValue() {
    mSavedRelFrame = getRelFrame();
    mSavedValue = getValue();
}

void QrealKey::setRelFrame(const int &frame) {
    if(frame == mRelFrame) return;
    int dFrame = frame - mRelFrame;
    setEndFrameVar(mEndFrame + dFrame);
    setStartFrameVar(mStartFrame + dFrame);
    mRelFrame = frame;
    if(mParentAnimator == nullptr) return;
    mParentAnimator->anim_updateKeyOnCurrrentFrame();
}

bool QrealKey::differsFromKey(Key *key) {
    if(key == this) return false;
    QrealKeySPtr qa_key = key->ref<QrealKey>();
    if(isZero(qa_key->getValue() - mValue)) {
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

void QrealKey::changeFrameAndValueBy(const QPointF &frameValueChange) {
    setValue(frameValueChange.y() + mSavedValue);
    int newFrame = qRound(frameValueChange.x() + mSavedRelFrame);
    if(mParentAnimator != nullptr) {
        mParentAnimator->anim_moveKeyToRelFrame(this, newFrame, false);
    } else {
        setRelFrame(newFrame);
    }
}
