#ifndef SMARTPATHANIMATOR_H
#define SMARTPATHANIMATOR_H
#include "smartpathcontainer.h"
#include "Animators/interpolationanimatort.h"
#include "differsinterpolate.h"
#include "basicreadwrite.h"

class SmartPathKey : public InterpolationKeyT<SmartPath> {
    friend class StdSelfRef;
public:
    void save() {
        mValue.save();
    }

    void restore() {
        mValue.restore();
    }

    void updateAfterPrevKeyChanged(Key * const prevKey) {
        mValue.setPrev(&static_cast<SmartPathKey*>(prevKey)->getValue());
    }

    void updateAfterNextKeyChanged(Key * const nextKey) {
        mValue.setNext(&static_cast<SmartPathKey*>(nextKey)->getValue());
    }

    void assignValue(const SmartPath& value) {
        mValue.assign(value);
    }
protected:
    SmartPathKey(const SmartPath& value, const int &relFrame,
                 Animator * const parentAnimator) :
        InterpolationKeyT<SmartPath>(value, relFrame, parentAnimator) {}
    SmartPathKey(Animator * const parentAnimator) :
        InterpolationKeyT<SmartPath>(parentAnimator) {}
};

class SmartPathAnimator : public GraphAnimator {
    friend class SelfRef;
public:
    bool SWT_isSmartPathAnimator() const { return true; }

    void anim_setAbsFrame(const int &frame) {
        const int lastRelFrame = anim_mCurrentRelFrame;
        Animator::anim_setAbsFrame(frame);
        if(this->anim_hasKeys()) {
            const auto prevK1 = anim_getPrevKey(lastRelFrame);
            const auto prevK2 = anim_getPrevKey(anim_mCurrentRelFrame);
            const auto nextK1 = anim_getNextKey(lastRelFrame);
            const auto nextK2 = anim_getNextKey(anim_mCurrentRelFrame);
            if(prevK1 == nullptr && prevK2 == nullptr) return;
            if(nextK1 == nullptr && nextK2 == nullptr) return;
            this->anim_callFrameChangeUpdater();
        }
    }

    SkPath getPathAtAbsFrame(const qreal &frame) const {
        return getPathAtRelFrame(this->prp_absFrameToRelFrameF(frame));
    }

    void graph_getValueConstraints(
            GraphKey *key, const QrealPointType &type,
            qreal &minValue, qreal &maxValue) const;


    SkPath getPathAtRelFrame(const qreal &frame) const {
        if(this->anim_mKeys.isEmpty()) return mBaseValue.getPathAt();
        int prevId;
        int nextId;
        if(this->anim_getNextAndPreviousKeyIdForRelFrameF(prevId, nextId,
                                                          frame)) {
            if(nextId == prevId) {
                return getKeyAtId(nextId)->getValue().getPathAt();
            } else {
                const auto * const prevKey = getKeyAtId(prevId);
                const auto * const nextKey = getKeyAtId(nextId);

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
                const qreal nWeight = (iFrame - prevFrame)/dFrame;
                return prevKey->getValue().interpolateWithNext(nWeight);
            }
        }
        return mBaseValue.getPathAt();
    }

    SmartPath * getCurrentlyEditedPath() const {
        return mPathBeingChanged_d;
    }

    void currentlyEditedPathChanged() {
        if(anim_mKeyOnCurrentFrame) {
            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
        } else {
            prp_updateInfluenceRangeAfterChanged();
        }
    }

    SmartPath* startPathChange() {
        if(mPathBeingChanged_d) return mPathBeingChanged_d;
        if(anim_isRecording() && !anim_mKeyOnCurrentFrame) {
            anim_saveCurrentValueAsKey();
        }
        if(anim_mKeyOnCurrentFrame) {
            const auto spk = GetAsPtr(anim_mKeyOnCurrentFrame,
                                      SmartPathKey);
            mPathBeingChanged_d = &spk->getValue();
        } else {
            mPathBeingChanged_d = &mBaseValue;
        }
        mPathBeingChanged_d->save();
        return mPathBeingChanged_d;
    }

    void cancelPathChange() {
        if(!mPathBeingChanged_d) return;
        mPathBeingChanged_d->restore();
        if(anim_mKeyOnCurrentFrame) {
            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
        } else {
            prp_updateInfluenceRangeAfterChanged();
        }
        mPathBeingChanged_d = nullptr;
    }

    void finishPathChange() {
        if(!mPathBeingChanged_d) return;
        if(anim_isKeyOnCurrentFrame()) {
            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
        } else {
            prp_updateInfluenceRangeAfterChanged();
        }
        mPathBeingChanged_d = nullptr;
    }

    SmartPathKey* getKeyAtId(const int& id) const {
        return GetAsPtr(this->anim_mKeys.at(id), SmartPathKey);
    }

    void anim_saveCurrentValueAsKey() {
        if(!this->anim_mIsRecording) this->anim_setRecording(true);

        if(this->anim_mKeyOnCurrentFrame) {
            const auto spk = GetAsPtr(this->anim_mKeyOnCurrentFrame,
                                      SmartPathKey);
            spk->assignValue(mBaseValue);
        } else {
            const auto newKey = SPtrCreate(SmartPathKey)(
                        mBaseValue, this->anim_mCurrentRelFrame, this);
            this->anim_appendKey(GetAsSPtr(newKey, Key));
            this->anim_mKeyOnCurrentFrame = newKey.get();
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
protected:
    SmartPathAnimator();
private:
    SmartPath mBaseValue;
    SmartPath * mPathBeingChanged_d = &mBaseValue;
};

#endif // SMARTPATHANIMATOR_H
