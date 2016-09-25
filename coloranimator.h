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

    void setCurrentValue(Color colorValue);
    void setCurrentValue(QColor qcolorValue);

    Color getCurrentValue();
    void setColorMode(ColorMode colorMode);

    void startVal1Transform();
    void startVal2Transform();
    void startVal3Transform();
    void startAlphaTransform();

    void startTransform();
    void finishTransform();
    void setConnectedToMainWindow(ConnectedToMainWindow *connected);
    void setUpdater(AnimatorUpdater *updater);
    void setFrame(int frame);
    void sortKeys();
    void updateKeysPath();
    void cancelTransform();
    void retrieveSavedValue();
    void drawBoxesList(QPainter *p,
                       qreal drawX, qreal drawY,
                       qreal pixelsPerFrame,
                       int startFrame, int endFrame, bool animationBar);
    qreal getBoxesListHeight();
    QrealKey *getKeyAtPos(qreal relX, qreal relY,
                          int minViewedFrame, qreal pixelsPerFrame);
private:
    ColorMode mColorMode;
    QrealAnimator mVal1Animator;
    QrealAnimator mVal2Animator;
    QrealAnimator mVal3Animator;
    QrealAnimator mAlphaAnimator;
};

#endif // COLORANIMATOR_H
