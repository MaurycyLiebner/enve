#ifndef BOOLPROPERTY_H
#define BOOLPROPERTY_H
#include "property.h"

class BoolProperty : public Property {
    e_OBJECT
protected:
    BoolProperty(const QString& name);
public:
    bool getValue();

    bool SWT_isBoolProperty() const { return true; }
    void writeProperty(eWriteStream& dst) const;
    void readProperty(eReadStream& src);
    void setValue(const bool value);
private:
    bool mValue = false;
};

#include "../Animators/staticcomplexanimator.h"
class BoolPropertyContainer : public StaticComplexAnimator {
    e_OBJECT
protected:
    BoolPropertyContainer(const QString& name);
public:
    bool SWT_isBoolPropertyContainer() const { return true; }
    void writeProperty(eWriteStream& dst) const;
    void readProperty(eReadStream& src);

    void setValue(const bool value);
    bool getValue();

    using ComplexAnimator::ca_addChild;
    using ComplexAnimator::ca_insertChild;
    using ComplexAnimator::ca_prependChildAnimator;
private:
    bool mValue = false;
};

#endif // BOOLPROPERTY_H
