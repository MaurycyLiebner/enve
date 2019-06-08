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
    void writeProperty(QIODevice * const dst) const;
    void readProperty(QIODevice * const src);
    void setValue(const bool value);
private:
    bool mValue = false;
};

#include "Animators/staticcomplexanimator.h"
class BoolPropertyContainer : public StaticComplexAnimator {
    friend class SelfRef;
protected:
    BoolPropertyContainer(const QString& name);
public:
    bool SWT_isBoolPropertyContainer() const { return true; }
    void writeProperty(QIODevice * const dst) const;
    void readProperty(QIODevice * const src);

    void setValue(const bool value);
    bool getValue();
private:
    bool mValue = false;
};

#endif // BOOLPROPERTY_H
