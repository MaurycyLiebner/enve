#include "Animators/coloranimator.h"
#include "colorhelpers.h"
#include "pointtypemenu.h"

ColorAnimator::ColorAnimator(const QString &name) : StaticComplexAnimator(name) {
    setColorMode(ColorMode::rgb);

    ca_addChild(mVal1Animator);
    ca_addChild(mVal2Animator);
    ca_addChild(mVal3Animator);
    ca_addChild(mAlphaAnimator);
}

void ColorAnimator::writeProperty(QIODevice * const dst) const {
    StaticComplexAnimator::writeProperty(dst);
    dst->write(rcConstChar(&mColorMode), sizeof(ColorMode));
}

void ColorAnimator::readProperty(QIODevice * const src) {
    StaticComplexAnimator::readProperty(src);
    src->read(rcChar(&mColorMode), sizeof(ColorMode));
    setColorMode(mColorMode);
}

void ColorAnimator::qra_setCurrentValue(const QColor &colorValue) {
    qreal val1, val2, val3;
    if(mColorMode == ColorMode::rgb) {
        val1 = colorValue.redF();
        val2 = colorValue.greenF();
        val3 = colorValue.blueF();
    } else if(mColorMode == ColorMode::hsv) {
        val1 = colorValue.hsvHueF();
        val2 = colorValue.hsvSaturationF();
        val3 = colorValue.valueF();
    } else { // HSLMODE
        val1 = colorValue.hslHueF();
        val2 = colorValue.hslSaturationF();
        val3 = colorValue.lightnessF();
    }
    const qreal alpha = colorValue.alphaF();

    mVal1Animator->setCurrentBaseValue(val1);
    mVal2Animator->setCurrentBaseValue(val2);
    mVal3Animator->setCurrentBaseValue(val3);
    mAlphaAnimator->setCurrentBaseValue(alpha);
}

QColor ColorAnimator::getColor() const {
    qreal val1 = mVal1Animator->getCurrentEffectiveValue();
    qreal val2 = mVal2Animator->getCurrentEffectiveValue();
    qreal val3 = mVal3Animator->getCurrentEffectiveValue();
    qreal alpha = mAlphaAnimator->getCurrentEffectiveValue();

    QColor color;
    if(mColorMode == ColorMode::rgb) {
        color.setRgbF(val1, val2, val3, alpha);
    } else if(mColorMode == ColorMode::hsv) {
        color.setHsvF(val1, val2, val3, alpha);
    } else { // HSLMODE
        color.setHslF(val1, val2, val3, alpha);
    }
    return color;
}

QColor ColorAnimator::getColor(const qreal relFrame) {
    const qreal val1 = mVal1Animator->getEffectiveValue(relFrame);
    const qreal val2 = mVal2Animator->getEffectiveValue(relFrame);
    const qreal val3 = mVal3Animator->getEffectiveValue(relFrame);
    const qreal alpha = mAlphaAnimator->getEffectiveValue(relFrame);

    QColor color;
    if(mColorMode == ColorMode::rgb) {
        color.setRgbF(val1, val2, val3, alpha);
    } else if(mColorMode == ColorMode::hsv) {
        color.setHsvF(val1, val2, val3, alpha);
    } else { // HSLMODE
        color.setHslF(val1, val2, val3, alpha);
    }
    return color;
}

void ColorAnimator::setColor(const QColor &col) {
    qreal val1, val2, val3;
    const qreal alpha = col.alphaF();
    if(mColorMode == ColorMode::rgb) {
        val1 = col.redF();
        val2 = col.greenF();
        val3 = col.blueF();
    } else if(mColorMode == ColorMode::hsv) {
        val1 = col.hueF();
        val2 = col.hsvSaturationF();
        val3 = col.valueF();
    } else { // HSLMODE
        val1 = col.hueF();
        val2 = col.hslSaturationF();
        val3 = col.lightnessF();
    }

    mVal1Animator->setCurrentBaseValue(val1);
    mVal2Animator->setCurrentBaseValue(val2);
    mVal3Animator->setCurrentBaseValue(val3);
    mAlphaAnimator->setCurrentBaseValue(alpha);
}

void ColorAnimator::setColorMode(const ColorMode colorMode) {
    if(colorMode == ColorMode::rgb) {
        mVal1Animator->prp_setName("red");
        mVal2Animator->prp_setName("green");
        mVal3Animator->prp_setName("blue");
    } else if(colorMode == ColorMode::hsv) {
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
    if(mColorMode == ColorMode::rgb && colorMode == ColorMode::hsv) {
        foo = &qrgb_to_hsv;
    } else if(mColorMode == ColorMode::rgb && colorMode == ColorMode::hsl) {
        foo = &qrgb_to_hsl;
    } else if(mColorMode == ColorMode::hsv && colorMode == ColorMode::rgb) {
        foo = &qhsv_to_rgb;
    } else if(mColorMode == ColorMode::hsv && colorMode == ColorMode::hsl) {
        foo = &qhsv_to_hsl;
    } else if(mColorMode == ColorMode::hsl && colorMode == ColorMode::rgb) {
        foo = &qhsl_to_rgb;
    } else if(mColorMode == ColorMode::hsl && colorMode == ColorMode::hsv) {
        foo = &qhsl_to_hsv;
    } else {
        return;
    }

    for(const auto &key : anim_mKeys) {
        const int frame = key->getAbsFrame();

        qreal rF = mVal1Animator->getBaseValueAtAbsFrame(frame);
        qreal gF = mVal2Animator->getBaseValueAtAbsFrame(frame);
        qreal bF = mVal3Animator->getBaseValueAtAbsFrame(frame);

        foo(rF, gF, bF);

        mVal1Animator->saveValueToKey(frame, rF);
        mVal2Animator->saveValueToKey(frame, gF);
        mVal3Animator->saveValueToKey(frame, bF);
    }

    if(!anim_mKeys.isEmpty()) {
        mVal1Animator->anim_setRecordingWithoutChangingKeys(true);
        mVal2Animator->anim_setRecordingWithoutChangingKeys(true);
        mVal3Animator->anim_setRecordingWithoutChangingKeys(true);
    } else {
        qreal crF = mVal1Animator->getCurrentBaseValue();
        qreal cgF = mVal2Animator->getCurrentBaseValue();
        qreal cbF = mVal3Animator->getCurrentBaseValue();

        foo(crF, cgF, cbF);

        mVal1Animator->setCurrentBaseValue(crF);
        mVal2Animator->setCurrentBaseValue(cgF);
        mVal3Animator->setCurrentBaseValue(cbF);
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

void ColorAnimator::setCurrentVal1Value(const qreal val1) {
    mVal1Animator->setCurrentBaseValue(val1);
}

void ColorAnimator::setCurrentVal2Value(const qreal val2) {
    mVal2Animator->setCurrentBaseValue(val2);
}

void ColorAnimator::setCurrentVal3Value(const qreal val3) {
    mVal3Animator->setCurrentBaseValue(val3);
}

void ColorAnimator::setCurrentAlphaValue(const qreal alpha) {
    mAlphaAnimator->setCurrentBaseValue(alpha);
}

void ColorAnimator::setupTreeViewMenu(PropertyMenu * const menu) {
    const auto colorModeMenu = menu->addMenu("Color Mode");

    const PropertyMenu::CheckSelectedOp<ColorAnimator> rgbOp =
    [](ColorAnimator * anim, bool checked) {
        Q_UNUSED(checked);
        anim->setColorMode(ColorMode::rgb);
    };
    colorModeMenu->addCheckableAction("RGB", mColorMode == ColorMode::rgb, rgbOp);

    const PropertyMenu::CheckSelectedOp<ColorAnimator> hsvOp =
    [](ColorAnimator * anim, bool checked) {
        Q_UNUSED(checked);
        anim->setColorMode(ColorMode::rgb);
    };
    colorModeMenu->addCheckableAction("HSV", mColorMode == ColorMode::hsv, hsvOp);

    const PropertyMenu::CheckSelectedOp<ColorAnimator> hslOp =
    [](ColorAnimator * anim, bool checked) {
        Q_UNUSED(checked);
        anim->setColorMode(ColorMode::rgb);
    };
    colorModeMenu->addCheckableAction("HSL", mColorMode == ColorMode::hsl, hslOp);
}
