#include "Animators/coloranimator.h"
#include "colorhelpers.h"
#include <QDebug>

ColorAnimator::ColorAnimator(const QString &name) : ComplexAnimator(name) {
    setColorMode(RGBMODE);

    ca_addChildAnimator(mVal1Animator);
    ca_addChildAnimator(mVal2Animator);
    ca_addChildAnimator(mVal3Animator);
    ca_addChildAnimator(mAlphaAnimator);
}

void ColorAnimator::qra_setCurrentValue(const QColor &colorValue) {
    qreal val1, val2, val3;
    if(mColorMode == RGBMODE) {
        val1 = colorValue.redF();
        val2 = colorValue.greenF();
        val3 = colorValue.blueF();
    } else if(mColorMode == HSVMODE) {
        val1 = colorValue.hsvHueF();
        val2 = colorValue.hsvSaturationF();
        val3 = colorValue.valueF();
    } else { // HSLMODE
        val1 = colorValue.hslHueF();
        val2 = colorValue.hslSaturationF();
        val3 = colorValue.lightnessF();
    }
    qreal alpha = colorValue.alphaF();

    mVal1Animator->qra_setCurrentValue(val1);
    mVal2Animator->qra_setCurrentValue(val2);
    mVal3Animator->qra_setCurrentValue(val3);
    mAlphaAnimator->qra_setCurrentValue(alpha);
}

QColor ColorAnimator::getCurrentColor() const {
    qreal val1 = mVal1Animator->qra_getCurrentEffectiveValue();
    qreal val2 = mVal2Animator->qra_getCurrentEffectiveValue();
    qreal val3 = mVal3Animator->qra_getCurrentEffectiveValue();
    qreal alpha = mAlphaAnimator->qra_getCurrentEffectiveValue();

    QColor color;
    if(mColorMode == RGBMODE) {
        color.setRgbF(val1, val2, val3, alpha);
    } else if(mColorMode == HSVMODE) {
        color.setHsvF(val1, val2, val3, alpha);
    } else { // HSLMODE
        color.setHslF(val1, val2, val3, alpha);
    }
    return color;
}

QColor ColorAnimator::getColorAtRelFrame(const int &relFrame) {
    qreal val1 = mVal1Animator->qra_getEffectiveValueAtRelFrame(relFrame);
    qreal val2 = mVal2Animator->qra_getEffectiveValueAtRelFrame(relFrame);
    qreal val3 = mVal3Animator->qra_getEffectiveValueAtRelFrame(relFrame);
    qreal alpha = mAlphaAnimator->qra_getEffectiveValueAtRelFrame(relFrame);

    QColor color;
    if(mColorMode == RGBMODE) {
        color.setRgbF(val1, val2, val3, alpha);
    } else if(mColorMode == HSVMODE) {
        color.setHsvF(val1, val2, val3, alpha);
    } else { // HSLMODE
        color.setHslF(val1, val2, val3, alpha);
    }
    return color;
}

QColor ColorAnimator::getColorAtRelFrameF(const qreal &relFrame) {
    qreal val1 = mVal1Animator->qra_getEffectiveValueAtRelFrameF(relFrame);
    qreal val2 = mVal2Animator->qra_getEffectiveValueAtRelFrameF(relFrame);
    qreal val3 = mVal3Animator->qra_getEffectiveValueAtRelFrameF(relFrame);
    qreal alpha = mAlphaAnimator->qra_getEffectiveValueAtRelFrameF(relFrame);

    QColor color;
    if(mColorMode == RGBMODE) {
        color.setRgbF(val1, val2, val3, alpha);
    } else if(mColorMode == HSVMODE) {
        color.setHsvF(val1, val2, val3, alpha);
    } else { // HSLMODE
        color.setHslF(val1, val2, val3, alpha);
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

    void (*foo)(qreal&, qreal&, qreal&);
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

    Q_FOREACH(const auto &key, anim_mKeys) {
        int frame = key->getAbsFrame();

        qreal rF = mVal1Animator->getCurrentValueAtAbsFrame(frame);
        qreal gF = mVal2Animator->getCurrentValueAtAbsFrame(frame);
        qreal bF = mVal3Animator->getCurrentValueAtAbsFrame(frame);

        foo(rF, gF, bF);

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

        foo(crF, cgF, cbF);

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

void ColorAnimator::setCurrentVal1Value(const qreal &val1) {
    mVal1Animator->qra_setCurrentValue(val1);
}

void ColorAnimator::setCurrentVal2Value(const qreal &val2) {
    mVal2Animator->qra_setCurrentValue(val2);
}

void ColorAnimator::setCurrentVal3Value(const qreal &val3) {
    mVal3Animator->qra_setCurrentValue(val3);
}

void ColorAnimator::setCurrentAlphaValue(const qreal &alpha) {
    mAlphaAnimator->qra_setCurrentValue(alpha);
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
    if(selected_action != nullptr) {
        if(selected_action->text() == "Add Key") {
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
