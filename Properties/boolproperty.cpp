#include "boolproperty.h"

BoolProperty::BoolProperty() : Property() {

}

bool BoolProperty::getValue() {
    return mValue;
}

void BoolProperty::setValue(const bool &value) {
    mValue = value;
    prp_updateInfluenceRangeAfterChanged();
    prp_callUpdater();
    prp_callFinishUpdater();
}


BoolPropertyContainer::BoolPropertyContainer() : ComplexAnimator() {

}

bool BoolPropertyContainer::getValue() {
    return mValue;
}

void BoolPropertyContainer::setValue(const bool &value) {
    mValue = value;
    prp_updateInfluenceRangeAfterChanged();
    prp_callUpdater();
    prp_callFinishUpdater();

    Q_FOREACH(const QSharedPointer<Property> &prop, ca_mChildAnimators) {
        //prop->SWT_setVisible(value);
        prop->SWT_setDisabled(!value);
    }
}
