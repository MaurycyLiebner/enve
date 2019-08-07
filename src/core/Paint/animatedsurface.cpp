#include "animatedsurface.h"

AnimatedSurface::AnimatedSurface() : Animator("canvas"),
    mBaseValue(enve::make_shared<DrawableAutoTiledSurface>()),
    mCurrent_d(mBaseValue.get()) {}

ASKey::ASKey(AnimatedSurface * const parent) :
    Key(parent),
    mValue(enve::make_shared<DrawableAutoTiledSurface>()) {}

ASKey::ASKey(const int frame, AnimatedSurface * const parent) :
    Key(frame, parent),
    mValue(enve::make_shared<DrawableAutoTiledSurface>()) {}

ASKey::ASKey(const DrawableAutoTiledSurface &value,
             const int frame, AnimatedSurface * const parent) :
    ASKey(frame, parent) {
    mValue->deepCopy(value);
}
