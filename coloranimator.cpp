#include "coloranimator.h"

ColorAnimator::ColorAnimator() : ComplexAnimator()
{
    setName("color");
    mAlphaAnimator.setName("alpha");
    setColorMode(RGBMODE);
}

Color ColorAnimator::getCurrentValue()
{
    Color color;
    if(mColorMode == RGBMODE) {
        color.setRGB(mVal1Animator.getCurrentValue(),
                     mVal2Animator.getCurrentValue(),
                     mVal3Animator.getCurrentValue(),
                     mAlphaAnimator.getCurrentValue() );
    } else if(mColorMode == HSVMODE) {
        color.setHSV(mVal1Animator.getCurrentValue(),
                     mVal2Animator.getCurrentValue(),
                     mVal3Animator.getCurrentValue(),
                     mAlphaAnimator.getCurrentValue() );
    } else { // HSLMODE
        color.setHSL(mVal1Animator.getCurrentValue(),
                     mVal2Animator.getCurrentValue(),
                     mVal3Animator.getCurrentValue(),
                     mAlphaAnimator.getCurrentValue() );
    }
    return color;
}

void ColorAnimator::setColorMode(ColorMode colorMode)
{
    if(mColorMode == RGBMODE) {
        mVal1Animator.setName("red");
        mVal2Animator.setName("green");
        mVal3Animator.setName("blue");
    } else if(mColorMode == HSVMODE) {
        mVal1Animator.setName("hue");
        mVal2Animator.setName("saturation");
        mVal3Animator.setName("value");
    } else { // HSLMODE
        mVal1Animator.setName("hue");
        mVal2Animator.setName("saturation");
        mVal3Animator.setName("lightness");
    }
    mColorMode = colorMode;
}
