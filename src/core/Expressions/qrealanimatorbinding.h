#ifndef QREALANIMATORBINDING_H
#define QREALANIMATORBINDING_H

#include "propertybinding.h"

class QrealAnimator;

class QrealAnimatorBinding : public PropertyBinding {
public:
    QrealAnimatorBinding(const Validator& validator,
                         const Property* const context);

    QJSValue getJSValue(QJSEngine& e);
    QJSValue getJSValue(QJSEngine& e, const qreal relFrame);

    void updateValue();

    qreal getValue();
    qreal getValue(const qreal relFrame);
private:
    qreal mCurrentValue;
};

#endif // QREALANIMATORBINDING_H
