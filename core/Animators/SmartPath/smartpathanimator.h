#ifndef SMARTPATHANIMATOR_H
#define SMARTPATHANIMATOR_H
#include "Animators/interpolationanimatort.h"
#include "differsinterpolate.h"
#include "basicreadwrite.h"
#include "smartpathkey.h"

class SmartPathAnimator : public GraphAnimator {
    friend class SelfRef;
    Q_OBJECT
public:
    bool SWT_isSmartPathAnimator() const { return true; }

    void anim_setAbsFrame(const int &frame) {
        if(frame == anim_mCurrentAbsFrame) return;
        const int lastRelFrame = anim_mCurrentRelFrame;
        Animator::anim_setAbsFrame(frame);
        if(this->anim_hasKeys()) {
            const auto prevK1 = anim_getPrevKey<SmartPathKey>(lastRelFrame);
            const auto prevK2 = anim_getPrevKey<SmartPathKey>(anim_mCurrentRelFrame);
            const auto nextK1 = anim_getNextKey<SmartPathKey>(lastRelFrame);
            const auto nextK2 = anim_getNextKey<SmartPathKey>(anim_mCurrentRelFrame);
            const auto keyAtFrame1 = anim_getKeyAtRelFrame(lastRelFrame);
            if(!prevK1 && !prevK2 && !keyAtFrame1) return;
            if(!nextK1 && !nextK2 && !keyAtFrame1) return;
            const auto keyAtFrame2 = anim_getKeyOnCurrentFrame<SmartPathKey>();
            if(!prevK2) {
                mBaseValue.assign(nextK2->getValue());
            } else if(!nextK2) {
                mBaseValue.assign(prevK2->getValue());
            } else if(keyAtFrame2) {
                mBaseValue.assign(keyAtFrame2->getValue());
            } else {
                const qreal nWeight =
                        prevKeyWeight(prevK2, nextK2, anim_mCurrentRelFrame);
                gInterpolate(prevK2->getValue(), nextK2->getValue(),
                             nWeight, mBaseValue);
            }
            this->anim_callFrameChangeUpdater();
        }
    }

    SkPath getPathAtAbsFrame(const qreal &frame) const {
        return getPathAtRelFrame(this->prp_absFrameToRelFrameF(frame));
    }

    void graph_getValueConstraints(
            GraphKey *key, const QrealPointType &type,
            qreal &minValue, qreal &maxValue) const;

    qreal prevKeyWeight(const SmartPathKey * const prevKey,
                        const SmartPathKey * const nextKey,
                        const qreal & frame) const {
        const qreal prevFrame = prevKey->getRelFrame();
        const qreal nextFrame = nextKey->getRelFrame();

        const qCubicSegment1D seg{qreal(prevKey->getRelFrame()),
                                  prevKey->getEndFrame(),
                                  nextKey->getStartFrame(),
                                  qreal(nextKey->getRelFrame())};
        const qreal t = gTFromX(seg, frame);
        const qreal p0y = prevKey->getValueForGraph();
        const qreal p1y = prevKey->getEndValue();
        const qreal p2y = nextKey->getStartValue();
        const qreal p3y = nextKey->getValueForGraph();
        const qreal iFrame = gCubicValueAtT({p0y, p1y, p2y, p3y}, t);
        const qreal dFrame = nextFrame - prevFrame;
        const qreal pWeight = (iFrame - prevFrame)/dFrame;
        return pWeight;
    }

    SkPath getPathAtRelFrame(const qreal &frame) const {
        if(this->anim_mKeys.isEmpty()) return mBaseValue.getPathAt();
        int prevId;
        int nextId;
        if(this->anim_getNextAndPreviousKeyIdForRelFrameF(prevId, nextId,
                                                          frame)) {
            const auto nextKey = anim_getKeyAtIndex<SmartPathKey>(nextId);
            if(nextId == prevId) {
                return nextKey->getValue().getPathAt();
            } else {
                const auto prevKey = anim_getKeyAtIndex<SmartPathKey>(prevId);
                const qreal pWeight = prevKeyWeight(prevKey, nextKey, frame);
                return nextKey->getValue().interpolateWithPrev(pWeight);
            }
        }
        return mBaseValue.getPathAt();
    }

    SmartPath * getCurrentlyEditedPath() const {
        return mPathBeingChanged_d;
    }

    void anim_afterKeyOnCurrentFrameChanged(Key* const key) {
        const auto spk = static_cast<SmartPathKey*>(key);
        if(spk) {
            mPathBeingChanged_d = &spk->getValue();
        } else {
            mPathBeingChanged_d = &mBaseValue;
        }
    }

    void beforeBinaryPathChange() {
        if(anim_isRecording() && !anim_getKeyOnCurrentFrame()) {
            anim_saveCurrentValueAsKey();
        }
    }

    void startPathChange() {
        if(anim_isRecording() && !anim_getKeyOnCurrentFrame()) {
            anim_saveCurrentValueAsKey();
        }
        mPathBeingChanged_d->save();
    }

    void pathChanged() {
        const auto spk = anim_getKeyOnCurrentFrame<SmartPathKey>();
        if(spk) {
            anim_updateAfterChangedKey(spk);
        } else {
            prp_updateInfluenceRangeAfterChanged();
        }
    }

    void cancelPathChange() {
        mPathBeingChanged_d->restore();
        const auto spk = anim_getKeyOnCurrentFrame<SmartPathKey>();
        if(spk) {
            anim_updateAfterChangedKey(spk);
        } else {
            prp_updateInfluenceRangeAfterChanged();
        }
    }

    void finishPathChange() {
        const auto spk = anim_getKeyOnCurrentFrame<SmartPathKey>();
        if(spk) {
            anim_updateAfterChangedKey(spk);
        } else {
            prp_updateInfluenceRangeAfterChanged();
        }
    }

    void anim_saveCurrentValueAsKey() {
        if(!this->anim_mIsRecording) this->anim_setRecording(true);

        const auto spk = this->anim_getKeyOnCurrentFrame<SmartPathKey>();
        if(spk) {
            spk->assignValue(mBaseValue);
        } else {
            const auto newKey = SPtrCreate(SmartPathKey)(
                        mBaseValue, this->anim_mCurrentRelFrame, this);
            this->anim_appendKey(newKey);
        }
    }

    void writeProperty(QIODevice * const target) const {
        int nKeys = this->anim_mKeys.count();
        target->write(rcConstChar(&nKeys), sizeof(int));
        for(const auto &key : this->anim_mKeys) {
            key->writeKey(target);
        }
        gWrite(target, mBaseValue);
    }

    void readProperty(QIODevice *target) {
        int nKeys;
        target->read(rcChar(&nKeys), sizeof(int));
        for(int i = 0; i < nKeys; i++) {
            auto newKey = SPtrCreate(SmartPathKey)(this);
            newKey->readKey(target);
            this->anim_appendKey(newKey);
        }
        gRead(target, mBaseValue);
    }
signals:
    void pathChangedAfterFrameChange();
protected:
    SmartPathAnimator();
private:
    SmartPath mBaseValue;
    SmartPath * mPathBeingChanged_d = &mBaseValue;
};

#endif // SMARTPATHANIMATOR_H
