#include "Animators/coloranimator.h"
#include "Colors/helpers.h"
#include <QDebug>

ColorAnimator::ColorAnimator() : ComplexAnimator() {
    prp_setName("color");
    mAlphaAnimator->prp_setName("alpha");
    setColorMode(RGBMODE);

    ca_addChildAnimator(mVal1Animator.data());
    ca_addChildAnimator(mVal2Animator.data());
    ca_addChildAnimator(mVal3Animator.data());
    ca_addChildAnimator(mAlphaAnimator.data());

    mVal1Animator->qra_setValueRange(0., 1.);
    mVal1Animator->setPrefferedValueStep(0.05);
    mVal1Animator->freezeMinMaxValues();
    mVal2Animator->qra_setValueRange(0., 1.);
    mVal2Animator->setPrefferedValueStep(0.05);
    mVal2Animator->freezeMinMaxValues();
    mVal3Animator->qra_setValueRange(0., 1.);
    mVal3Animator->setPrefferedValueStep(0.05);
    mVal3Animator->freezeMinMaxValues();
    mAlphaAnimator->qra_setValueRange(0., 1.);
    mAlphaAnimator->setPrefferedValueStep(0.05);
    mAlphaAnimator->freezeMinMaxValues();
}

void ColorAnimator::qra_setCurrentValue(const Color &colorValue,
                                        const bool &saveUndoRedo,
                                        const bool &finish) {
    if(mColorMode == RGBMODE) {
        mVal1Animator->qra_setCurrentValue(colorValue.gl_r,
                                           saveUndoRedo,
                                           finish);
        mVal2Animator->qra_setCurrentValue(colorValue.gl_g,
                                           saveUndoRedo,
                                           finish);
        mVal3Animator->qra_setCurrentValue(colorValue.gl_b,
                                           saveUndoRedo,
                                           finish);
    } else if(mColorMode == HSVMODE) {
        mVal1Animator->qra_setCurrentValue(colorValue.gl_h,
                                           saveUndoRedo,
                                           finish);
        mVal2Animator->qra_setCurrentValue(colorValue.gl_s,
                                           saveUndoRedo,
                                           finish);
        mVal3Animator->qra_setCurrentValue(colorValue.gl_v,
                                           saveUndoRedo,
                                           finish);
    } else { // HSLMODE
        float h = colorValue.gl_h;
        float s = colorValue.gl_s;
        float l = colorValue.gl_v;
        hsv_to_hsl(&h, &s, &l);

        mVal1Animator->qra_setCurrentValue(h, saveUndoRedo, finish);
        mVal2Animator->qra_setCurrentValue(s, saveUndoRedo, finish);
        mVal3Animator->qra_setCurrentValue(l, saveUndoRedo, finish);
    }
    mAlphaAnimator->qra_setCurrentValue(colorValue.gl_a, saveUndoRedo, finish);
}

void ColorAnimator::qra_setCurrentValue(const QColor &qcolorValue,
                                        const bool &saveUndoRedo,
                                        const bool &finish) {
    Color color;
    color.setQColor(qcolorValue);
    qra_setCurrentValue(color, saveUndoRedo, finish);
}

Color ColorAnimator::getCurrentColor() const {
    Color color;
    if(mColorMode == RGBMODE) {
        color.setRGB(mVal1Animator->qra_getCurrentEffectiveValue(),
                     mVal2Animator->qra_getCurrentEffectiveValue(),
                     mVal3Animator->qra_getCurrentEffectiveValue(),
                     mAlphaAnimator->qra_getCurrentEffectiveValue() );
    } else if(mColorMode == HSVMODE) {
        color.setHSV(mVal1Animator->qra_getCurrentEffectiveValue(),
                     mVal2Animator->qra_getCurrentEffectiveValue(),
                     mVal3Animator->qra_getCurrentEffectiveValue(),
                     mAlphaAnimator->qra_getCurrentEffectiveValue() );
    } else { // HSLMODE
        color.setHSL(mVal1Animator->qra_getCurrentEffectiveValue(),
                     mVal2Animator->qra_getCurrentEffectiveValue(),
                     mVal3Animator->qra_getCurrentEffectiveValue(),
                     mAlphaAnimator->qra_getCurrentEffectiveValue() );
    }
    return color;
}

Color ColorAnimator::getColorAtRelFrame(const int &relFrame) {
    Color color;
    if(mColorMode == RGBMODE) {
        color.setRGB(mVal1Animator->qra_getEffectiveValueAtRelFrame(relFrame),
                     mVal2Animator->qra_getEffectiveValueAtRelFrame(relFrame),
                     mVal3Animator->qra_getEffectiveValueAtRelFrame(relFrame),
                     mAlphaAnimator->qra_getEffectiveValueAtRelFrame(relFrame) );
    } else if(mColorMode == HSVMODE) {
        color.setHSV(mVal1Animator->qra_getEffectiveValueAtRelFrame(relFrame),
                     mVal2Animator->qra_getEffectiveValueAtRelFrame(relFrame),
                     mVal3Animator->qra_getEffectiveValueAtRelFrame(relFrame),
                     mAlphaAnimator->qra_getEffectiveValueAtRelFrame(relFrame) );
    } else { // HSLMODE
        color.setHSL(mVal1Animator->qra_getEffectiveValueAtRelFrame(relFrame),
                     mVal2Animator->qra_getEffectiveValueAtRelFrame(relFrame),
                     mVal3Animator->qra_getEffectiveValueAtRelFrame(relFrame),
                     mAlphaAnimator->qra_getEffectiveValueAtRelFrame(relFrame) );
    }
    return color;
}

Color ColorAnimator::getColorAtRelFrameF(const qreal &relFrame) {
    Color color;
    if(mColorMode == RGBMODE) {
        color.setRGB(mVal1Animator->qra_getEffectiveValueAtRelFrameF(relFrame),
                     mVal2Animator->qra_getEffectiveValueAtRelFrameF(relFrame),
                     mVal3Animator->qra_getEffectiveValueAtRelFrameF(relFrame),
                     mAlphaAnimator->qra_getEffectiveValueAtRelFrameF(relFrame) );
    } else if(mColorMode == HSVMODE) {
        color.setHSV(mVal1Animator->qra_getEffectiveValueAtRelFrameF(relFrame),
                     mVal2Animator->qra_getEffectiveValueAtRelFrameF(relFrame),
                     mVal3Animator->qra_getEffectiveValueAtRelFrameF(relFrame),
                     mAlphaAnimator->qra_getEffectiveValueAtRelFrameF(relFrame) );
    } else { // HSLMODE
        color.setHSL(mVal1Animator->qra_getEffectiveValueAtRelFrameF(relFrame),
                     mVal2Animator->qra_getEffectiveValueAtRelFrameF(relFrame),
                     mVal3Animator->qra_getEffectiveValueAtRelFrameF(relFrame),
                     mAlphaAnimator->qra_getEffectiveValueAtRelFrameF(relFrame) );
    }
    return color;
}

void ColorAnimator::setColorMode(const ColorMode &colorMode) {
    if(colorMode == RGBMODE) {
        mVal1Animator->prp_setName("red");
        mVal2Animator->prp_setName("green");
        mVal3Animator->prp_setName("blue");
    } else if(colorMode == HSVMODE) {
        mVal1Animator->prp_setName("hue");
        mVal2Animator->prp_setName("saturation");
        mVal3Animator->prp_setName("value");
    } else { // HSLMODE
        mVal1Animator->prp_setName("hue");
        mVal2Animator->prp_setName("saturation");
        mVal3Animator->prp_setName("lightness");
    }
    if(mColorMode == colorMode) return;

    void (*foo)(qreal*, qreal*, qreal*);
    if(mColorMode == RGBMODE && colorMode == HSVMODE) {
        foo = &qrgb_to_hsv;
    } else if(mColorMode == RGBMODE && colorMode == HSLMODE) {
        foo = &qrgb_to_hsl;
    } else if(mColorMode == HSVMODE && colorMode == RGBMODE) {
        foo = &qhsv_to_rgb;
    } else if(mColorMode == HSVMODE && colorMode == HSLMODE) {
        foo = &qhsv_to_hsl;
    } else if(mColorMode == HSLMODE && colorMode == RGBMODE) {
        foo = &qhsl_to_rgb;
    } else if(mColorMode == HSLMODE && colorMode == HSVMODE) {
        foo = &qhsl_to_hsv;
    } else {
        return;
    }

    Q_FOREACH(const std::shared_ptr<Key> &key, anim_mKeys) {
        int frame = key->getAbsFrame();

        qreal rF = mVal1Animator->getCurrentValueAtAbsFrame(frame);
        qreal gF = mVal2Animator->getCurrentValueAtAbsFrame(frame);
        qreal bF = mVal3Animator->getCurrentValueAtAbsFrame(frame);

        foo(&rF, &gF, &bF);

        mVal1Animator->qra_saveValueToKey(frame, rF);
        mVal2Animator->qra_saveValueToKey(frame, gF);
        mVal3Animator->qra_saveValueToKey(frame, bF);
    }

    if(!anim_mKeys.isEmpty()) {
        mVal1Animator->anim_setRecordingWithoutChangingKeys(true);
        mVal2Animator->anim_setRecordingWithoutChangingKeys(true);
        mVal3Animator->anim_setRecordingWithoutChangingKeys(true);
    } else {
        qreal crF = mVal1Animator->qra_getCurrentValue();
        qreal cgF = mVal2Animator->qra_getCurrentValue();
        qreal cbF = mVal3Animator->qra_getCurrentValue();

        foo(&crF, &cgF, &cbF);

        mVal1Animator->qra_setCurrentValue(crF);
        mVal2Animator->qra_setCurrentValue(cgF);
        mVal3Animator->qra_setCurrentValue(cbF);
    }

    mColorMode = colorMode;

    emit colorModeChanged(mColorMode);
}

void ColorAnimator::startVal1Transform() {
    mVal1Animator->prp_startTransform();
}

void ColorAnimator::startVal2Transform() {
    mVal2Animator->prp_startTransform();
}

void ColorAnimator::startVal3Transform() {
    mVal3Animator->prp_startTransform();
}

void ColorAnimator::startAlphaTransform() {
    mAlphaAnimator->prp_startTransform();
}

void ColorAnimator::setCurrentVal1Value(const qreal &val1,
                                        const bool &finish) {
    mVal1Animator->qra_setCurrentValue(val1, finish);
}

void ColorAnimator::setCurrentVal2Value(const qreal &val2,
                                        const bool &finish) {
    mVal2Animator->qra_setCurrentValue(val2, finish);
}

void ColorAnimator::setCurrentVal3Value(const qreal &val3,
                                        const bool &finish) {
    mVal3Animator->qra_setCurrentValue(val3, finish);
}

void ColorAnimator::setCurrentAlphaValue(const qreal &alpha,
                                         const bool &finish) {
    mAlphaAnimator->qra_setCurrentValue(alpha, finish);
}

#include <QMenu>
void ColorAnimator::prp_openContextMenu(const QPoint &pos) {
    QMenu menu;
    menu.addAction("Add Key");

    QMenu colorModeMenu;
    colorModeMenu.setTitle("Color Mode");

    QAction *rgbAction = new QAction("RGB");
    rgbAction->setCheckable(true);
    rgbAction->setChecked(mColorMode == RGBMODE);

    QAction *hsvAction = new QAction("HSV");
    hsvAction->setCheckable(true);
    hsvAction->setChecked(mColorMode == HSVMODE);

    QAction *hslAction = new QAction("HSL");
    hslAction->setCheckable(true);
    hslAction->setChecked(mColorMode == HSLMODE);

    colorModeMenu.addAction(rgbAction);
    colorModeMenu.addAction(hsvAction);
    colorModeMenu.addAction(hslAction);
    menu.addMenu(&colorModeMenu);
    QAction *selected_action = menu.exec(pos);
    if(selected_action != nullptr)
    {
        if(selected_action->text() == "Add Key")
        {
            anim_saveCurrentValueAsKey();
        } else if(selected_action == rgbAction) {
            setColorMode(RGBMODE);
        } else if(selected_action == hsvAction) {
            setColorMode(HSVMODE);
        } else if(selected_action == hslAction) {
            setColorMode(HSLMODE);
        }
    } else {

    }
}
