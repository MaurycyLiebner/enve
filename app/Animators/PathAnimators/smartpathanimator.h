#ifndef SMARTPATHANIMATOR_H
#define SMARTPATHANIMATOR_H
#include "Animators/interpolationanimator.h"
#include "smartpathcontainer.h"

#define GetAsSP(key) GetAsPtr(key, SmartPath)

class SmartPathAnimator : public InterpolationAnimator {
    friend class SelfRef;
public:

    void startPathChange() {
        if(mPathChanged) return;
        if(prp_isRecording()) {
            if(prp_isKeyOnCurrentFrame()) return;
            anim_saveCurrentValueAsKey();
        }
        if(prp_isKeyOnCurrentFrame()) {
            GetAsSP(anim_mKeyOnCurrentFrame)->startPathChange();
            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
        } else {
            mCurrentPath->startPathChange();
            prp_updateInfluenceRangeAfterChanged();
        }
        mPathChanged = true;
    }

    void cancelPathChange() {
        if(!mPathChanged) return;
        if(prp_isKeyOnCurrentFrame()) {
            GetAsSP(anim_mKeyOnCurrentFrame)->cancelPathChange();
            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
        } else {
            mCurrentPath->cancelPathChange();
            prp_updateInfluenceRangeAfterChanged();
        }
        mPathChanged = false;
    }

    void finishedPathChange() {
        if(!mPathChanged) return;
        if(prp_isKeyOnCurrentFrame()) {
            GetAsSP(anim_mKeyOnCurrentFrame)->finishedPathChange();
            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
        } else {
            mCurrentPath->finishedPathChange();
            prp_updateInfluenceRangeAfterChanged();
        }
        mPathChanged = false;
        prp_callFinishUpdater();
    }
protected:
    SmartPathAnimator();
private:
    bool mPathChanged = false;
    stdsptr<SmartPath> mCurrentPath;
};

#endif // SMARTPATHANIMATOR_H
