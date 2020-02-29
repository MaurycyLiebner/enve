#include "propertybindingparser.h"

#include "exceptions.h"

void skipSpaces(const QString& exp, int& position) {
    while(position < exp.count() && exp.at(position) == ' ') {
        position++;
    }
}

bool parseBindingAssignment(const QString& exp, int& pos) {
    if(pos < exp.count()) {
        const auto& character = exp.at(pos);
        if(character == '=') {
            pos++;
            return true;
        }
    }
    return false;
}

bool parseBindingName(const QString& exp, int& pos, QString& name) {
    int newPos = pos;
    name.clear();
    while(newPos < exp.count()) {
        const auto& character = exp.at(newPos++);
        if(!character.isLetter() && !character.isDigit()
           && character != '_') break;
        name.append(character);
    }
    if(name.isEmpty()) return false;
    pos = newPos;
    return true;
}

bool checkComment(const QString& exp, int& pos) {
    if(pos < exp.count() + 1) {
        int newPos = pos;
        if(exp.at(newPos++) == '/' && exp.at(newPos++) == '/') {
            pos = newPos;
            return true;
        }
    }
    return false;
}

void parseBinding(const QString& exp, int& pos, QString& binding) {
    while(pos < exp.count()) {
        const auto& c = exp.at(pos++);
        if(!c.isLetterOrNumber() && c != ' ' && c != '.' && c != '_') break;
        binding.append(c);
    }
}

qsptr<PropertyBinding> PropertyBindingParser::parseBinding(
        QString& name,
        const QString& exp,
        const PropertyBinding::Validator& validator,
        const Property* const context) {
    int pos = 0;
    skipSpaces(exp, pos);
    if(checkComment(exp, pos)) return nullptr;
    if(!parseBindingName(exp, pos, name))
        PrettyRuntimeThrow("Invalid binding definition:\n'" + exp + "'");
    skipSpaces(exp, pos);
    if(!parseBindingAssignment(exp, pos))
        PrettyRuntimeThrow("Invalid binding definition:\n'" + exp + "'");
    QString binding;
    parseBinding(exp, pos, binding);
    const auto result = PropertyBinding::sCreate(binding.trimmed(),
                                                 validator, context);
    if(!result) PrettyRuntimeThrow("Binding could not be resolved:\n'" +
                                   binding + "'");
    return result;
}

PropertyBindingMap PropertyBindingParser::parseBindings(
        QString exp,
        const PropertyBinding::Validator& validator,
        const Property* const context) {
    const auto lines = exp.split(QRegExp("\n|\r\n|\r|;"),
                                 QString::SkipEmptyParts);
    PropertyBindingMap result;
    for(const auto& line : lines) {
        QString name;
        const auto binding = parseBinding(name, line, validator, context);
        if(binding) {
            if(result.find(name) != result.end())
                PrettyRuntimeThrow(name + " is already defined");
            result[name] = binding;
        }
    }
    return result;
}
