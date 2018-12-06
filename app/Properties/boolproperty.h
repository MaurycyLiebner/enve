#ifndef BOOLPROPERTY_H
#define BOOLPROPERTY_H
#include "Properties/property.h"

class BoolProperty : public Property {
    Q_OBJECT
    friend class SelfRef;
public:
    bool getValue();

    bool SWT_isBoolProperty() { return true; }
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
public slots:
    void setValue(const bool &value);
protected:
    BoolProperty(const QString& name);
private:
    bool mValue = false;
};

#include "Animators/complexanimator.h"
class BoolPropertyContainer : public ComplexAnimator {
    Q_OBJECT
    friend class SelfRef;
public:
    bool getValue();

    bool SWT_isBoolPropertyContainer() { return true; }
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
public slots:
    void setValue(const bool &value);
protected:
    BoolPropertyContainer(const QString& name);
private:
    bool mValue = false;
};

#endif // BOOLPROPERTY_H
