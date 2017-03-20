#include "boolproperty.h"
#include "Animators/complexanimator.h"

BoolProperty::BoolProperty() : Property() {

}

bool BoolProperty::getValue() {
    return mValue;
}

void BoolProperty::setValue(const bool &value) {
    mValue = value;
    mParentAnimator->callUpdater();
}
