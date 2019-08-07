#ifndef INTPROPERTY_H
#define INTPROPERTY_H
#include "property.h"

class IntProperty : public Property {
    Q_OBJECT
    e_OBJECT
protected:
    IntProperty(const QString& name);
public:
    bool SWT_isIntProperty() const;

    void writeProperty(QIODevice * const dst) const;
    void readProperty(QIODevice * const src);
    void prp_startTransform();
    void prp_finishTransform();

    void setValueRange(const int minValue, const int maxValue);

    void setCurrentValue(const int value);
    int getValue();

    int getMaxValue();
    int getMinValue();
protected:
    bool mTransformed = false;
    int mValue = 0;
    int mMinValue = 0;
    int mMaxValue = 9999;
signals:
    void valueChangedSignal(int);
};

#endif // INTPROPERTY_H
