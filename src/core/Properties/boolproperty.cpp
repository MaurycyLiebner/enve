#include "boolproperty.h"

BoolProperty::BoolProperty(const QString &name) :
    Property(name) {}

bool BoolProperty::getValue() {
    return mValue;
}

void BoolProperty::setValue(const bool value) {
    mValue = value;
    prp_afterWholeInfluenceRangeChanged();
}

void BoolProperty::writeProperty(eWriteStream& dst) const {
    dst << mValue;
}

void BoolProperty::readProperty(eReadStream& src) {
    src >> mValue;
}

BoolPropertyContainer::BoolPropertyContainer(const QString &name) :
    StaticComplexAnimator(name) {}

bool BoolPropertyContainer::getValue() {
    return mValue;
}

void BoolPropertyContainer::setValue(const bool value) {
    mValue = value;
    prp_afterWholeInfluenceRangeChanged();

    for(const auto& prop : ca_mChildAnimators) {
        //prop->SWT_setVisible(value);
        prop->SWT_setDisabled(!value);
    }
}


void BoolPropertyContainer::writeProperty(eWriteStream& dst) const {
    dst << mValue;
    StaticComplexAnimator::writeProperty(dst);
}

void BoolPropertyContainer::readProperty(eReadStream& src) {
    bool value;
    src >> value;
    setValue(value);
    StaticComplexAnimator::readProperty(src);
}
