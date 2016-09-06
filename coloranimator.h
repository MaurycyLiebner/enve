#ifndef COLORANIMATOR_H
#define COLORANIMATOR_H
#include "complexanimator.h"
#include "Colors/color.h"

class ColorAnimator : public ComplexAnimator
{
public:
    ColorAnimator();

private:
    QrealAnimator mHueAnimator;
    QrealAnimator mSaturationAnimator;
    QrealAnimator mValueAnimator;
    QrealAnimator mAlphaAnimator;
};

#endif // COLORANIMATOR_H
