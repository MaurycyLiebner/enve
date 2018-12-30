#ifndef INTPROPERTY_H
#define INTPROPERTY_H
#include "Properties/property.h"

class IntProperty : public Property {
    Q_OBJECT
    friend class SelfRef;
public:
    void setValueRange(const int &minValue,
                       const int &maxValue);

    void setCurrentValue(const int &value);

    bool SWT_isIntProperty();

    int getValue();

    int getMaxValue();
    int getMinValue();
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
    void prp_startTransform();
    void prp_finishTransform();
protected:
    IntProperty(const QString& name);

    bool mTransformed = false;
    int mValue = 0;
    int mMinValue = 0;
    int mMaxValue = 9999;
signals:
    void valueChangedSignal(qreal);
};

#endif // INTPROPERTY_H
