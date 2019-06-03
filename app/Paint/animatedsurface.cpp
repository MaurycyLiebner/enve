#include "animatedsurface.h"

AnimatedSurface::AnimatedSurface() : GraphAnimator("canvas"),
    mBaseValue(SPtrCreate(DrawableAutoTiledSurface)()),
    mCurrent_d(mBaseValue.get()) {}

ASKey::ASKey(AnimatedSurface * const parent) :
    GraphKey(parent),
    mValue(SPtrCreate(DrawableAutoTiledSurface)()) {}

ASKey::ASKey(const int &frame, AnimatedSurface * const parent) :
    GraphKey(frame, parent),
    mValue(SPtrCreate(DrawableAutoTiledSurface)()) {}

ASKey::ASKey(const DrawableAutoTiledSurface &value,
             const int &frame, AnimatedSurface * const parent) :
    ASKey(frame, parent) {
    mValue->deepCopy(value);
}
