#ifndef QPOINTFANIMATORBINDING_H
#define QPOINTFANIMATORBINDING_H

#include "propertybinding.h"

class QPointFAnimator;

class QPointFAnimatorBinding : public PropertyBinding {
public:
    QPointFAnimatorBinding(const Validator& validator,
                           const Property* const context);

    QJSValue getJSValue(QJSEngine& e);
    QJSValue getJSValue(QJSEngine& e, const qreal relFrame);

    void updateValue();

    QPointF getValue();
    QPointF getValue(const qreal relFrame);
private:
    QPointF mCurrentValue;
};

#endif // QPOINTFANIMATORBINDING_H
