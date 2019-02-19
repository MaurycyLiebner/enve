#ifndef SMARTPATHANIMATOR_H
#define SMARTPATHANIMATOR_H
#include "Animators/interpolationanimator.h"
#include "smartpathcontainer.h"
#include "Animators/graphanimatort.h"
#include "differsinterpolate.h"
#include "basicreadwrite.h"
class PathAnimator;
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

    void updateAfterPrevKeyChanged(Key * const prevKey) {
        mValue.setPrev(&static_cast<SmartPathKey*>(prevKey)->getValue());
    }

    void updateAfterNextKeyChanged(Key * const nextKey) {
        mValue.setNext(&static_cast<SmartPathKey*>(nextKey)->getValue());
    }
protected:
    SmartPathKey(const SmartPath& value, const int &relFrame,
                 Animator * const parentAnimator) :
        GraphKeyT<SmartPath>(value, relFrame, parentAnimator) {}
    SmartPathKey(Animator * const parentAnimator) :
        GraphKeyT<SmartPath>(parentAnimator) {}
};

typedef BasedAnimatorT<GraphAnimator,
                       SmartPathKey,
                       SmartPath> SmartPathAnimatorBase;

class SmartPathAnimator : public SmartPathAnimatorBase {
    friend class SelfRef;
public:
    bool SWT_isSmartPathAnimator() const { return true; }

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

    void startPathChange() {
        if(mPathBeingChanged_d) return;
        if(anim_isRecording() && !anim_mKeyOnCurrentFrame) {
            anim_saveCurrentValueAsKey();
        }
        if(anim_mKeyOnCurrentFrame) {
            const auto spk = GetAsPtr(anim_mKeyOnCurrentFrame,
                                      SmartPathKey);
            spk->save();
            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
            mPathBeingChanged_d = &spk->getValue();
        } else {
            mCurrentValue.save();
            prp_updateInfluenceRangeAfterChanged();
            mPathBeingChanged_d = &mCurrentValue;
        }
    }

    void cancelPathChange() {
        if(!mPathBeingChanged_d) return;
        if(anim_mKeyOnCurrentFrame) {
            const auto spk = GetAsPtr(anim_mKeyOnCurrentFrame,
                                      SmartPathKey);
            spk->restore();
            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
        } else {
            mCurrentValue.restore();
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
protected:
    SmartPathAnimator(PathAnimator * const pathAnimator);
private:
    SmartPath * mPathBeingChanged_d = nullptr;
};

#endif // SMARTPATHANIMATOR_H
