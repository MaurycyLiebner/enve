#include "comboboxproperty.h"

ComboBoxProperty::ComboBoxProperty(const QString &name,
                                   const QStringList &list) :
    Property(name) {
    mValueNames = list;
}
