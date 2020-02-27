#include "qrealanimatorbinding.h"

#include "Animators/qrealanimator.h"

QrealAnimatorBinding::QrealAnimatorBinding(const Validator& validator,
                                           const Property* const context) :
    PropertyBinding(sWrapValidatorForClass<QrealAnimator>(validator),
                    context) {}

QJSValue QrealAnimatorBinding::getJSValue(QJSEngine& e) {
    Q_UNUSED(e)
    return getValue();
}

QJSValue QrealAnimatorBinding::getJSValue(QJSEngine& e, const qreal relFrame) {
    Q_UNUSED(e)
    return getValue(relFrame);
}

void QrealAnimatorBinding::updateValue() {
    const auto anim = static_cast<QrealAnimator*>(getBindProperty());
    if(!anim || !isBindPathValid()) return;
    mCurrentValue = anim->getEffectiveValue();
}

qreal QrealAnimatorBinding::getValue() {
    updateValueIfNeeded();
    return mCurrentValue;
}

qreal QrealAnimatorBinding::getValue(const qreal relFrame) {
    const auto anim = static_cast<QrealAnimator*>(getBindProperty());
    return anim->getEffectiveValue(relFrame);
}
