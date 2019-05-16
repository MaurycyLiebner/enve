#include "animatedsurface.h"

AnimatedSurface::AnimatedSurface() : GraphAnimator("canvas"),
    mBaseValue(SPtrCreate(DrawableAutoTiledSurface)()),
    mCurrent_d(mBaseValue.get()) {}

ASKey::ASKey(AnimatedSurface * const parent) :
    GraphKey(parent) {}

ASKey::ASKey(const int &frame, AnimatedSurface * const parent) :
    GraphKey(frame, parent) {}

ASKey::ASKey(const DrawableAutoTiledSurface &value,
             const int &frame, AnimatedSurface * const parent) :
    GraphKey(frame, parent),
    mValue(SPtrCreate(DrawableAutoTiledSurface)()) {
    mValue->deepCopy(value);
}
