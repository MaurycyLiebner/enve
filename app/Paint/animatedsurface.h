#ifndef ANIMATEDSURFACE_H
#define ANIMATEDSURFACE_H
#include "Animators/graphanimator.h"
#include "Animators/interpolationkeyt.h"
#include "autotiledsurface.h"
#include "Animators/qrealpoint.h"

class AnimatedSurface : public GraphAnimator {
    friend class SelfRef;
    typedef InterpolationKeyT<AutoTiledSurface> BKey;
protected:
    AnimatedSurface();
public:
    void anim_saveCurrentValueAsKey() {
        if(anim_getKeyOnCurrentFrame()) return;
        const auto newKey = SPtrCreate(BKey)(
                    mBaseValue, anim_getCurrentRelFrame(), this);
        anim_appendKey(newKey);
    }

    void anim_addKeyAtRelFrame(const int& relFrame) {
        if(anim_getKeyAtRelFrame(relFrame)) return;
        const auto newKey = SPtrCreate(BKey)(this);
        newKey->setRelFrame(relFrame);
        anim_appendKey(newKey);
    }

    void anim_afterKeyOnCurrentFrameChanged(Key* const key) {
        const auto spk = static_cast<BKey*>(key);
        if(spk) mCurrent_d = &spk->getValue();
        else mCurrent_d = &mBaseValue;
    }


    void graph_getValueConstraints(
            GraphKey *key, const QrealPointType &type,
            qreal &minValue, qreal &maxValue) const {
        if(type == QrealPointType::KEY_POINT) {
            minValue = key->getRelFrame();
            maxValue = minValue;
            //getFrameConstraints(key, type, minValue, maxValue);
        } else {
            minValue = -DBL_MAX;
            maxValue = DBL_MAX;
        }
    }


    SkBitmap toBitmap() {
        return mCurrent_d->toBitmap();
    }

    AutoTiledSurface * getCurrentSurface() {
        return mCurrent_d;
    }
private:
    AutoTiledSurface mBaseValue;
    AutoTiledSurface * mCurrent_d = &mBaseValue;
};

#endif // ANIMATEDSURFACE_H
