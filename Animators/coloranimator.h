#ifndef COLORANIMATOR_H
#define COLORANIMATOR_H

#include "Animators/complexanimator.h"
#include "Colors/color.h"
#include "qrealanimator.h"

class ColorAnimator : public ComplexAnimator {
    Q_OBJECT
    friend class SelfRef;
public:
    void qra_setCurrentValue(const QColor &qcolorValue,
                             const bool &saveUndoRedo = false,
                             const bool &finish = false);

    QColor getCurrentColor() const;
    QColor getColorAtRelFrame(const int &relFrame);
    QColor getColorAtRelFrameF(const qreal &relFrame);
    void setColorMode(const ColorMode &colorMode);

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

    void prp_openContextMenu(const QPoint &pos);

    void duplicateVal1AnimatorFrom(QrealAnimator *source);
    void duplicateVal2AnimatorFrom(QrealAnimator *source);
    void duplicateVal3AnimatorFrom(QrealAnimator *source);
    void duplicateAlphaAnimatorFrom(QrealAnimator *source);

    ColorMode getColorMode() {
        return mColorMode;
    }

    QrealAnimator *getVal1Animator() {
        return mVal1Animator.get();
    }

    QrealAnimator *getVal2Animator() {
        return mVal2Animator.get();
    }

    QrealAnimator *getVal3Animator() {
        return mVal3Animator.get();
    }

    QrealAnimator *getAlphaAnimator() {
        return mAlphaAnimator.get();
    }

    bool SWT_isColorAnimator() { return true; }

    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
protected:
    ColorAnimator(const QString& name = "color");
private:
    ColorMode mColorMode = RGBMODE;

    QSharedPointer<QrealAnimator> mVal1Animator =
            SPtrCreate(QrealAnimator)("");
    QSharedPointer<QrealAnimator> mVal2Animator =
            SPtrCreate(QrealAnimator)("");
    QSharedPointer<QrealAnimator> mVal3Animator =
            SPtrCreate(QrealAnimator)("");
    QSharedPointer<QrealAnimator> mAlphaAnimator =
            SPtrCreate(QrealAnimator)("alpha");
signals:
    void colorModeChanged(ColorMode);
};

#endif // COLORANIMATOR_H
