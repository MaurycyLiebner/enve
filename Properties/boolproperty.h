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

    int prp_saveToSql(QSqlQuery *, const int &) {}
    void prp_loadFromSql(const int &) {}

    bool SWT_isBoolProperty() { return true; }
public slots:
    void setValue(const bool &value);
private:
    bool mValue = false;
};

#endif // BOOLPROPERTY_H
