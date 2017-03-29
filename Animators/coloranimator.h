#ifndef COLORANIMATOR_H
#define COLORANIMATOR_H
#include "Animators/complexanimator.h"
#include "Colors/color.h"

enum ColorMode {
    RGBMODE,
    HSVMODE,
    HSLMODE
};

class ColorAnimator : public ComplexAnimator
{
    Q_OBJECT
public:
    ColorAnimator();

    void qra_setCurrentValue(Color colorValue, bool finish = false);
    void qra_setCurrentValue(QColor qcolorValue, bool finish = false);

    Color qra_getCurrentValue() const;
    void setColorMode(ColorMode colorMode);

    void startVal1Transform();
    void startVal2Transform();
    void startVal3Transform();
    void startAlphaTransform();

    void setCurrentVal1Value(const qreal &val1,
                             const bool &finish = false);
    void setCurrentVal2Value(const qreal &val2,
                             const bool &finish = false);
    void setCurrentVal3Value(const qreal &val3,
                             const bool &finish = false);
    void setCurrentAlphaValue(const qreal &alpha,
                              const bool &finish = false);

    void prp_openContextMenu(QPoint pos);
    void prp_loadFromSql(const int &sqlId);
    int prp_saveToSql(QSqlQuery *query, const int &parentId = 0);
    void prp_makeDuplicate(Property *target);
    Property *prp_makeDuplicate();

    void anim_saveCurrentValueAsKey();
    void duplicateVal1AnimatorFrom(QrealAnimator *source);
    void duplicateVal2AnimatorFrom(QrealAnimator *source);
    void duplicateVal3AnimatorFrom(QrealAnimator *source);
    void duplicateAlphaAnimatorFrom(QrealAnimator *source);

    ColorMode getColorMode() {
        return mColorMode;
    }

    QrealAnimator *getVal1Animator() {
        return &mVal1Animator;
    }

    QrealAnimator *getVal2Animator() {
        return &mVal2Animator;
    }

    QrealAnimator *getVal3Animator() {
        return &mVal3Animator;
    }

    QrealAnimator *getAlphaAnimator() {
        return &mAlphaAnimator;
    }

    SWT_Type SWT_getType() { return SWT_ColorAnimator; }
private:
    ColorMode mColorMode = RGBMODE;
    QrealAnimator mVal1Animator;
    QrealAnimator mVal2Animator;
    QrealAnimator mVal3Animator;
    QrealAnimator mAlphaAnimator;
signals:
    void colorModeChanged(ColorMode);
};

#endif // COLORANIMATOR_H
