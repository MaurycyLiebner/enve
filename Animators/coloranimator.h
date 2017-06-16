#ifndef COLORANIMATOR_H
#define COLORANIMATOR_H

#include "Animators/complexanimator.h"
#include "Colors/color.h"
#include "qrealanimator.h"

enum ColorMode : short {
    RGBMODE,
    HSVMODE,
    HSLMODE
};

class ColorAnimator : public ComplexAnimator {
    Q_OBJECT
public:
    ColorAnimator();

    void qra_setCurrentValue(const Color &colorValue,
                             const bool &saveUndoRedo = false,
                             const bool &finish = false);
    void qra_setCurrentValue(const QColor &qcolorValue,
                             const bool &saveUndoRedo = false,
                             const bool &finish = false);

    Color getCurrentColor() const;
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
    void prp_loadFromSql(const int &sqlId);
    int prp_saveToSql(QSqlQuery *query, const int &parentId = 0);
    void makeDuplicate(Property *target);
    Property *makeDuplicate();

    void anim_saveCurrentValueAsKey();
    void duplicateVal1AnimatorFrom(QrealAnimator *source);
    void duplicateVal2AnimatorFrom(QrealAnimator *source);
    void duplicateVal3AnimatorFrom(QrealAnimator *source);
    void duplicateAlphaAnimatorFrom(QrealAnimator *source);

    ColorMode getColorMode() {
        return mColorMode;
    }

    QrealAnimator *getVal1Animator() {
        return mVal1Animator.data();
    }

    QrealAnimator *getVal2Animator() {
        return mVal2Animator.data();
    }

    QrealAnimator *getVal3Animator() {
        return mVal3Animator.data();
    }

    QrealAnimator *getAlphaAnimator() {
        return mAlphaAnimator.data();
    }

    bool SWT_isColorAnimator() { return true; }
private:
    ColorMode mColorMode = RGBMODE;
    QSharedPointer<QrealAnimator> mVal1Animator =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mVal2Animator =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mVal3Animator =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mAlphaAnimator =
            (new QrealAnimator())->ref<QrealAnimator>();
signals:
    void colorModeChanged(ColorMode);
};

#endif // COLORANIMATOR_H
