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

    void setCurrentValue(Color colorValue, bool finish = false);
    void setCurrentValue(QColor qcolorValue, bool finish = false);

    Color getCurrentValue() const;
    void setColorMode(ColorMode colorMode);

    void startVal1Transform();
    void startVal2Transform();
    void startVal3Transform();
    void startAlphaTransform();

    QrealKey *getKeyAtPos(qreal relX, qreal relY,
                          int minViewedFrame, qreal pixelsPerFrame);
    void openContextMenu(QPoint pos);
    void loadFromSql(int sqlId);
    int saveToSql();
private:
    ColorMode mColorMode = RGBMODE;
    QrealAnimator mVal1Animator;
    QrealAnimator mVal2Animator;
    QrealAnimator mVal3Animator;
    QrealAnimator mAlphaAnimator;
};

#endif // COLORANIMATOR_H
