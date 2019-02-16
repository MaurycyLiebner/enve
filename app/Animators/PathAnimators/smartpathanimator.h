#ifndef SMARTPATHANIMATOR_H
#define SMARTPATHANIMATOR_H
#include "Animators/interpolationanimator.h"
#include "smartpathcontainer.h"
#include "Animators/animatort.h"

//class SmartPathKey : public KeyT<stdsptr<SmartPath>> {
//    friend class StdSelfRef;
//public:
//    SmartPath * get() {
//        return getValue().get();
//    }
//protected:
//    SmartPathKey() {
//        setValue(SPtrCreate(SmartPath)());
//    }
//};

//#define GetAsSPK(key) GetAsPtr(key, SmartPathKey)

//class SmartPathAnimator : public InterpolationAnimator {
//    friend class SelfRef;
//public:

//    void startPathChange() {
//        if(mPathChanged) return;
//        if(prp_isRecording()) {
//            if(prp_isKeyOnCurrentFrame()) return;
//            anim_saveCurrentValueAsKey();
//        }
//        if(prp_isKeyOnCurrentFrame()) {
//            GetAsSPK(anim_mKeyOnCurrentFrame)->get()->save();
//            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
//        } else {
//            mCurrentPath->save();
//            prp_updateInfluenceRangeAfterChanged();
//        }
//        mPathChanged = true;
//    }

//    void cancelPathChange() {
//        if(!mPathChanged) return;
//        if(prp_isKeyOnCurrentFrame()) {
//            GetAsSPK(anim_mKeyOnCurrentFrame)->get()->restore();
//            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
//        } else {
//            mCurrentPath->restore();
//            prp_updateInfluenceRangeAfterChanged();
//        }
//        mPathChanged = false;
//    }

//    void finishedPathChange() {
//        if(!mPathChanged) return;
//        mPathChanged = false;
//        prp_callFinishUpdater();
//    }
//protected:
//    SmartPathAnimator();
//private:
//    bool mPathChanged = false;
//    stdsptr<SmartPath> mCurrentPath;
//};

#endif // SMARTPATHANIMATOR_H
