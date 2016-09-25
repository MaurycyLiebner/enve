#include "coloranimator.h"
#include "Colors/helpers.h"
#include "boxeslist.h"
#include <QDebug>

ColorAnimator::ColorAnimator() : ComplexAnimator()
{
    setName("color");
    mAlphaAnimator.setName("alpha");
    setColorMode(RGBMODE);

    mVal1Animator.setParentAnimator(this);
    mVal2Animator.setParentAnimator(this);
    mVal3Animator.setParentAnimator(this);
    mAlphaAnimator.setParentAnimator(this);
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

void ColorAnimator::drawBoxesList(QPainter *p,
                                  qreal drawX, qreal drawY,
                                  qreal pixelsPerFrame,
                                  int startFrame, int endFrame,
                                  bool animationBar)
{
    QrealAnimator::drawBoxesList(p, drawX, drawY,
                                 pixelsPerFrame, startFrame, endFrame,
                                 animationBar);
    if(mBoxesListDetailVisible) {
        drawX += LIST_ITEM_CHILD_INDENT;
        drawY += LIST_ITEM_HEIGHT;
        mVal1Animator.drawBoxesList(p, drawX, drawY,
                                pixelsPerFrame,
                                startFrame, endFrame,
                                animationBar);
        drawY += mVal1Animator.getBoxesListHeight();

        mVal2Animator.drawBoxesList(p, drawX, drawY,
                                pixelsPerFrame,
                                startFrame, endFrame,
                                animationBar);
        drawY += mVal2Animator.getBoxesListHeight();

        mVal3Animator.drawBoxesList(p, drawX, drawY,
                                pixelsPerFrame,
                                startFrame, endFrame,
                                animationBar);
        drawY += mVal3Animator.getBoxesListHeight();

        mAlphaAnimator.drawBoxesList(p, drawX, drawY,
                                pixelsPerFrame,
                                startFrame, endFrame,
                                animationBar);
        drawY += mAlphaAnimator.getBoxesListHeight();
    }
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

qreal ColorAnimator::getBoxesListHeight()
{
    if(mBoxesListDetailVisible) {
        return mVal1Animator.getBoxesListHeight() +
               mVal2Animator.getBoxesListHeight() +
               mVal3Animator.getBoxesListHeight() +
               mAlphaAnimator.getBoxesListHeight() +
               QrealAnimator::getBoxesListHeight();
    } else {
        return QrealAnimator::getBoxesListHeight();
    }
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

void ColorAnimator::startTransform()
{
    mVal1Animator.startTransform();
    mVal2Animator.startTransform();
    mVal3Animator.startTransform();
    mAlphaAnimator.startTransform();
}

void ColorAnimator::finishTransform()
{
    mConnectedToMainWindow->startNewUndoRedoSet();

    mVal1Animator.finishTransform();
    mVal2Animator.finishTransform();
    mVal3Animator.finishTransform();
    mAlphaAnimator.finishTransform();

    mConnectedToMainWindow->finishUndoRedoSet();
}

void ColorAnimator::setConnectedToMainWindow(ConnectedToMainWindow *connected)
{
    QrealAnimator::setConnectedToMainWindow(connected);
    mVal1Animator.setConnectedToMainWindow(connected);
    mVal2Animator.setConnectedToMainWindow(connected);
    mVal3Animator.setConnectedToMainWindow(connected);
    mAlphaAnimator.setConnectedToMainWindow(connected);
}

void ColorAnimator::setUpdater(AnimatorUpdater *updater)
{
    QrealAnimator::setUpdater(updater);
    mVal1Animator.setUpdater(updater);
    mVal2Animator.setUpdater(updater);
    mVal3Animator.setUpdater(updater);
    mAlphaAnimator.setUpdater(updater);
}

void ColorAnimator::setFrame(int frame)
{
    QrealAnimator::setFrame(frame);
    mVal1Animator.setFrame(frame);
    mVal2Animator.setFrame(frame);
    mVal3Animator.setFrame(frame);
    mAlphaAnimator.setFrame(frame);
}

void ColorAnimator::sortKeys()
{
    QrealAnimator::sortKeys();
    mVal1Animator.sortKeys();
    mVal2Animator.sortKeys();
    mVal3Animator.sortKeys();
    mAlphaAnimator.sortKeys();
}

void ColorAnimator::updateKeysPath()
{
    QrealAnimator::updateKeysPath();
    mVal1Animator.updateKeysPath();
    mVal2Animator.updateKeysPath();
    mVal3Animator.updateKeysPath();
    mAlphaAnimator.updateKeysPath();
}

void ColorAnimator::cancelTransform() {
    mVal1Animator.cancelTransform();
    mVal2Animator.cancelTransform();
    mVal3Animator.cancelTransform();
    mAlphaAnimator.cancelTransform();
}

void ColorAnimator::retrieveSavedValue()
{
    mVal1Animator.retrieveSavedValue();
    mVal2Animator.retrieveSavedValue();
    mVal3Animator.retrieveSavedValue();
    mAlphaAnimator.retrieveSavedValue();
}
