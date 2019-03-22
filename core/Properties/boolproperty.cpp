#include "boolproperty.h"

BoolProperty::BoolProperty(const QString &name) :
    Property(name) {}

bool BoolProperty::getValue() {
    return mValue;
}

void BoolProperty::setValue(const bool &value) {
    mValue = value;
    prp_updateInfluenceRangeAfterChanged();
}


BoolPropertyContainer::BoolPropertyContainer(const QString &name) :
    ComplexAnimator(name) {}

bool BoolPropertyContainer::getValue() {
    return mValue;
}

void BoolPropertyContainer::setValue(const bool &value) {
    mValue = value;
    prp_updateInfluenceRangeAfterChanged();

    for(const auto& prop : ca_mChildAnimators) {
        //prop->SWT_setVisible(value);
        prop->SWT_setDisabled(!value);
    }
}
