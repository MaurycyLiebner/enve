#ifndef BOOLPROPERTY_H
#define BOOLPROPERTY_H
#include "property.h"

class BoolProperty : public Property {
    Q_OBJECT
public:
    BoolProperty();

    bool getValue();

    bool SWT_isBoolProperty() { return true; }
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
public slots:
    void setValue(const bool &value);
private:
    bool mValue = false;
};

#include "Animators/complexanimator.h"
class BoolPropertyContainer : public ComplexAnimator {
    Q_OBJECT
public:
    BoolPropertyContainer();

    bool getValue();

    bool SWT_isBoolPropertyContainer() { return true; }
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
public slots:
    void setValue(const bool &value);
private:
    bool mValue = false;
};

#endif // BOOLPROPERTY_H
