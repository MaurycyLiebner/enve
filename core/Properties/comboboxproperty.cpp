#include "comboboxproperty.h"

ComboBoxProperty::ComboBoxProperty(const QString &name,
                                   const QStringList &list) :
    Property(name) {
    mValueNames = list;
}

void ComboBoxProperty::writeProperty(QIODevice * const target) const {
    target->write(rcConstChar(&mCurrentValue), sizeof(int));
}

void ComboBoxProperty::readProperty(QIODevice * const src) {
    src->read(rcChar(&mCurrentValue), sizeof(int));
}
