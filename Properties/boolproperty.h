#ifndef BOOLPROPERTY_H
#define BOOLPROPERTY_H
#include "property.h"

class BoolProperty : public Property {
    Q_OBJECT
public:
    BoolProperty();

    bool getValue();
    SWT_Type SWT_getType() { return SWT_BoolProperty; }

    void prp_makeDuplicate(Property *property);
    Property *prp_makeDuplicate();

    int prp_saveToSql(QSqlQuery *, const int &) {}
    void prp_loadFromSql(const int &) {}
public slots:
    void setValue(const bool &value);
private:
    bool mValue = false;
};

#endif // BOOLPROPERTY_H
