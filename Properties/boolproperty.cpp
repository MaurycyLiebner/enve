#include "boolproperty.h"
#include "Animators/complexanimator.h"

BoolProperty::BoolProperty() : Property() {

}

bool BoolProperty::getValue() {
    return mValue;
}

void BoolProperty::prp_makeDuplicate(Property *property) {
    ((BoolProperty*)property)->setValue(mValue);
}

Property *BoolProperty::prp_makeDuplicate() {
    BoolProperty *newProp = new BoolProperty();
    prp_makeDuplicate(newProp);
    return newProp;
}

void BoolProperty::setValue(const bool &value) {
    mValue = value;
    prp_updateAfterChangedAbsFrameRange(INT_MIN, INT_MAX);
    prp_callUpdater();
    prp_callFinishUpdater();
}
