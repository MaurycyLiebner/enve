#ifndef BOOLPROPERTY_H
#define BOOLPROPERTY_H
#include "property.h"

class BoolProperty : public Property {
    Q_OBJECT
public:
    BoolProperty();

    bool getValue();

    void makeDuplicate(Property *property);
    Property *makeDuplicate();

    bool SWT_isBoolProperty() { return true; }
    void writeBoolProperty(std::fstream *file);
    void readBoolProperty(std::fstream *file);
public slots:
    void setValue(const bool &value);
private:
    bool mValue = false;
};

#endif // BOOLPROPERTY_H
