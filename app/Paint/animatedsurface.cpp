#include "animatedsurface.h"

AnimatedSurface::AnimatedSurface() : GraphAnimator("canvas") {}

ASKey::ASKey(const int &frame, AnimatedSurface * const parent) :
    GraphKey(frame, parent) {}

ASKey::ASKey(const DrawableAutoTiledSurface &value,
             const int &frame, AnimatedSurface * const parent) :
    GraphKey(frame, parent), mValue(value) {}
