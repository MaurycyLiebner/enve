#include "animatedsurface.h"

AnimatedSurface::AnimatedSurface() : Animator("canvas"),
    mBaseValue(SPtrCreate(DrawableAutoTiledSurface)()),
    mCurrent_d(mBaseValue.get()) {}

ASKey::ASKey(AnimatedSurface * const parent) :
    Key(parent),
    mValue(SPtrCreate(DrawableAutoTiledSurface)()) {}

ASKey::ASKey(const int frame, AnimatedSurface * const parent) :
    Key(frame, parent),
    mValue(SPtrCreate(DrawableAutoTiledSurface)()) {}

ASKey::ASKey(const DrawableAutoTiledSurface &value,
             const int frame, AnimatedSurface * const parent) :
    ASKey(frame, parent) {
    mValue->deepCopy(value);
}
