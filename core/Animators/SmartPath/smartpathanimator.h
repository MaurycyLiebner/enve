#ifndef SMARTPATHANIMATOR_H
#define SMARTPATHANIMATOR_H
#include "Animators/interpolationanimator.h"
#include "smartpathcontainer.h"
#include "Animators/graphanimatort.h"
#include "differsinterpolate.h"
#include "basicreadwrite.h"

class SmartPathKey : public GraphKeyT<SmartPath> {
    friend class StdSelfRef;
public:
    void save() {
        mValue.save();
    }

    void restore() {
        mValue.restore();
    }

    qreal getValueForGraph() const {
        return mRelFrame;
    }

    void setValueForGraph(const qreal& value) {
        Q_UNUSED(value);
    }

    void setRelFrame(const int &frame) {
        if(frame == mRelFrame) return;
        const int dFrame = frame - mRelFrame;
        GraphKey::setRelFrame(frame);
        mEndValue += dFrame;
        mStartValue += dFrame;
    }

    void updateAfterNeighbouringKeysChanged(Key * const prevKey,
                                            Key * const nextKey) {
        mValue.setPrev(&static_cast<SmartPathKey*>(prevKey)->getValue());
        mValue.setNext(&static_cast<SmartPathKey*>(nextKey)->getValue());
    }
protected:
    SmartPathKey(const SmartPath& value, const int &relFrame,
                 Animator * const parentAnimator) :
        GraphKeyT<SmartPath>(value, relFrame, parentAnimator) {}
};

typedef BasedAnimatorT<GraphAnimator,
                       SmartPathKey,
                       SmartPath> SmartPathAnimatorBase;

class SmartPathAnimator : public SmartPathAnimatorBase {
    friend class SelfRef;
public:

    void startPathChange() {
        if(mPathChanged) return;
        if(prp_isRecording()) {
            if(prp_isKeyOnCurrentFrame()) return;
            anim_saveCurrentValueAsKey();
        }
        if(prp_isKeyOnCurrentFrame()) {
            const auto spk = GetAsPtr(anim_mKeyOnCurrentFrame, SmartPathKey);
            spk->save();
            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
        } else {
            mCurrentValue.save();
            prp_updateInfluenceRangeAfterChanged();
        }
        mPathChanged = true;
    }

    void cancelPathChange() {
        if(!mPathChanged) return;
        if(prp_isKeyOnCurrentFrame()) {
            const auto spk = GetAsPtr(anim_mKeyOnCurrentFrame, SmartPathKey);
            spk->restore();
            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
        } else {
            mCurrentValue.restore();
            prp_updateInfluenceRangeAfterChanged();
        }
        mPathChanged = false;
    }

    void finishedPathChange() {
        if(!mPathChanged) return;
        mPathChanged = false;
        prp_callFinishUpdater();
    }
protected:
    SmartPathAnimator();
private:
    bool mPathChanged = false;
};

#endif // SMARTPATHANIMATOR_H
