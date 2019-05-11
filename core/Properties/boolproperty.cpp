#include "boolproperty.h"

BoolProperty::BoolProperty(const QString &name) :
    Property(name) {}

bool BoolProperty::getValue() {
    return mValue;
}

void BoolProperty::setValue(const bool &value) {
    mValue = value;
    prp_afterWholeInfluenceRangeChanged();
}

void BoolProperty::writeProperty(QIODevice * const target) const {
    target->write(rcConstChar(&mValue), sizeof(bool));
}

void BoolProperty::readProperty(QIODevice *target) {
    target->read(rcChar(&mValue), sizeof(bool));
}

BoolPropertyContainer::BoolPropertyContainer(const QString &name) :
    ComplexAnimator(name) {}

bool BoolPropertyContainer::getValue() {
    return mValue;
}

void BoolPropertyContainer::setValue(const bool &value) {
    mValue = value;
    prp_afterWholeInfluenceRangeChanged();

    for(const auto& prop : ca_mChildAnimators) {
        //prop->SWT_setVisible(value);
        prop->SWT_setDisabled(!value);
    }
}


void BoolPropertyContainer::writeProperty(QIODevice * const target) const {
    target->write(rcConstChar(&mValue), sizeof(bool));
}

void BoolPropertyContainer::readProperty(QIODevice *target) {
    bool value;
    target->read(rcChar(&value), sizeof(bool));
    setValue(value);
}
