#ifndef COLORANIMATOR_H
#define COLORANIMATOR_H
#include "complexanimator.h"
#include "Colors/color.h"

enum ColorMode {
    HSVMODE,
    HSLMODE,
    RGBMODE
};

class ColorAnimator : public ComplexAnimator
{
public:
    ColorAnimator();

    Color getCurrentValue();
    void setColorMode(ColorMode colorMode);
private:
    ColorMode mColorMode;
    QrealAnimator mVal1Animator;
    QrealAnimator mVal2Animator;
    QrealAnimator mVal3Animator;
    QrealAnimator mAlphaAnimator;
};

#endif // COLORANIMATOR_H
