#include "qpointfanimatorbinding.h"

#include <QJSEngine>

#include "Animators/qpointfanimator.h"

QPointFAnimatorBinding::QPointFAnimatorBinding(const Validator& validator,
                                               const Property* const context) :
    PropertyBinding(sWrapValidatorForClass<QPointFAnimator>(validator),
                    context) {}

QJSValue toArray(QJSEngine& e, const QPointF& value) {
    auto array = e.newArray(2);
    array.setProperty(0, value.x());
    array.setProperty(1, value.y());
    return array;
}

QJSValue QPointFAnimatorBinding::getJSValue(QJSEngine& e) {
    return toArray(e, getValue());
}

QJSValue QPointFAnimatorBinding::getJSValue(QJSEngine& e, const qreal relFrame) {
    return toArray(e, getValue(relFrame));
}

void QPointFAnimatorBinding::updateValue() {
    const auto anim = static_cast<QPointFAnimator*>(getBindProperty());
    if(!anim || !isBindPathValid()) return;
    mCurrentValue = anim->getEffectiveValue();
}

QPointF QPointFAnimatorBinding::getValue() {
    updateValueIfNeeded();
    return mCurrentValue;
}

QPointF QPointFAnimatorBinding::getValue(const qreal relFrame) {
    const auto anim = static_cast<QPointFAnimator*>(getBindProperty());
    return anim->getEffectiveValue(relFrame);
}
