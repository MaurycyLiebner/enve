#include "interpolationkey.h"

InterpolationKey::InterpolationKey(const int frame,
                                   Animator * const parentAnimator) :
    GraphKey(frame, parentAnimator) {}

InterpolationKey::InterpolationKey(Animator * const parentAnimator) :
    GraphKey(0, parentAnimator) {}
