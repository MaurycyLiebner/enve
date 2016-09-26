#include "coloranimator.h"
#include "Colors/helpers.h"
#include "boxeslist.h"
#include <QDebug>

ColorAnimator::ColorAnimator() : ComplexAnimator()
{
    setName("color");
    mAlphaAnimator.setName("alpha");
    setColorMode(RGBMODE);

    addChildAnimator(&mVal1Animator);
    addChildAnimator(&mVal2Animator);
    addChildAnimator(&mVal3Animator);
    addChildAnimator(&mAlphaAnimator);
}

void ColorAnimator::setCurrentValue(Color colorValue)
{
    if(mColorMode == RGBMODE) {
        mVal1Animator.setCurrentValue(colorValue.gl_r);
        mVal2Animator.setCurrentValue(colorValue.gl_g);
        mVal3Animator.setCurrentValue(colorValue.gl_b);
        mAlphaAnimator.setCurrentValue(colorValue.gl_a);
    } else if(mColorMode == HSVMODE) {
        mVal1Animator.setCurrentValue(colorValue.gl_h);
        mVal2Animator.setCurrentValue(colorValue.gl_s);
        mVal3Animator.setCurrentValue(colorValue.gl_v);
        mAlphaAnimator.setCurrentValue(colorValue.gl_a);
    } else { // HSLMODE
        float h = colorValue.gl_h;
        float s = colorValue.gl_s;
        float l = colorValue.gl_v;
        hsv_to_hsl(&h, &s, &l);

        mVal1Animator.setCurrentValue(h);
        mVal2Animator.setCurrentValue(s);
        mVal3Animator.setCurrentValue(l);
        mAlphaAnimator.setCurrentValue(colorValue.gl_a);
    }
}

void ColorAnimator::setCurrentValue(QColor qcolorValue)
{
    Color color;
    color.setQColor(qcolorValue);
    setCurrentValue(color);
}

Color ColorAnimator::getCurrentValue() const
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

QrealKey *ColorAnimator::getKeyAtPos(qreal relX, qreal relY,
                                     int minViewedFrame,
                                     qreal pixelsPerFrame) {
    if(relY <= LIST_ITEM_HEIGHT) {
        return QrealAnimator::getKeyAtPos(relX, relY,
                                   minViewedFrame, pixelsPerFrame);
    } else if(mBoxesListDetailVisible) {
        if(relY <= 2*LIST_ITEM_HEIGHT) {
            return mVal1Animator.getKeyAtPos(relX, relY,
                                     minViewedFrame, pixelsPerFrame);
        } else if(relY <= 3*LIST_ITEM_HEIGHT) {
            return mVal2Animator.getKeyAtPos(relX, relY,
                                       minViewedFrame, pixelsPerFrame);
        } else if(relY <= 4*LIST_ITEM_HEIGHT) {
            return mVal3Animator.getKeyAtPos(relX, relY,
                                     minViewedFrame, pixelsPerFrame);
        } else if(relY <= 5*LIST_ITEM_HEIGHT) {
            return mAlphaAnimator.getKeyAtPos(relX, relY,
                                       minViewedFrame, pixelsPerFrame);
        }
    }
    return NULL;
}

void ColorAnimator::setColorMode(ColorMode colorMode)
{
    if(colorMode == RGBMODE) {
        mVal1Animator.setName("red");
        mVal2Animator.setName("green");
        mVal3Animator.setName("blue");
    } else if(colorMode == HSVMODE) {
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

void ColorAnimator::startVal1Transform()
{
    mVal1Animator.startTransform();
}

void ColorAnimator::startVal2Transform()
{
    mVal2Animator.startTransform();
}

void ColorAnimator::startVal3Transform()
{
    mVal3Animator.startTransform();
}

void ColorAnimator::startAlphaTransform()
{
    mAlphaAnimator.startTransform();
}
