#include "comboboxproperty.h"

ComboBoxProperty::ComboBoxProperty(const QString &name,
                                   const QStringList &list) :
    Property(name) {
    mValueNames = list;
}

void ComboBoxProperty::writeProperty(eWriteStream& dst) const {
    dst << mCurrentValue;
}

void ComboBoxProperty::readProperty(eReadStream& src) {
    src >> mCurrentValue;
}
