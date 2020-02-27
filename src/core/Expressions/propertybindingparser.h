#ifndef PROPERTYBINDINGPARSER_H
#define PROPERTYBINDINGPARSER_H

#include "propertybinding.h"

using PropertyBindingMap = std::map<QString, qsptr<PropertyBinding>>;

namespace PropertyBindingParser {
    qsptr<PropertyBinding> parseBinding(
            QString& name,
            const QString& exp,
            const PropertyBinding::Validator& validator,
            const Property* const context);
    PropertyBindingMap parseBindings(
            QString exp,
            const PropertyBinding::Validator& validator,
            const Property* const context);
};

#endif // PROPERTYBINDINGPARSER_H
