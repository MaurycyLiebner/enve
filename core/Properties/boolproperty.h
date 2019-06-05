#ifndef BOOLPROPERTY_H
#define BOOLPROPERTY_H
#include "Properties/property.h"

class BoolProperty : public Property {
    friend class SelfRef;
protected:
    BoolProperty(const QString& name);
public:
    bool getValue();

    bool SWT_isBoolProperty() const { return true; }
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
    void setValue(const bool value);
private:
    bool mValue = false;
};

#include "Animators/complexanimator.h"
class BoolPropertyContainer : public ComplexAnimator {
    friend class SelfRef;
protected:
    BoolPropertyContainer(const QString& name);
public:
    bool SWT_isBoolPropertyContainer() const { return true; }
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);

    void setValue(const bool value);
    bool getValue();
private:
    bool mValue = false;
};

#endif // BOOLPROPERTY_H
