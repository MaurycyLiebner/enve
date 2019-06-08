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

void BoolProperty::writeProperty(QIODevice * const dst) const {
    dst->write(rcConstChar(&mValue), sizeof(bool));
}

void BoolProperty::readProperty(QIODevice * const src) {
    src->read(rcChar(&mValue), sizeof(bool));
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


void BoolPropertyContainer::writeProperty(QIODevice * const dst) const {
    dst->write(rcConstChar(&mValue), sizeof(bool));
    StaticComplexAnimator::writeProperty(dst);
}

void BoolPropertyContainer::readProperty(QIODevice * const src) {
    bool value;
    src->read(rcChar(&value), sizeof(bool));
    setValue(value);
    //r StaticComplexAnimator::readProperty(src);
}
