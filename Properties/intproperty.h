#ifndef INTPROPERTY_H
#define INTPROPERTY_H
#include "property.h"

class IntProperty : public Property {
    Q_OBJECT
public:
    IntProperty();

    void setValueRange(const int &minValue,
                       const int &maxValue);

    void setCurrentValue(const int &value,
                         const bool &saveUndoRedo = false,
                         const bool &finish = false,
                         const bool &callUpdater = true);

    bool SWT_isIntProperty();

    int getValue();

    int getMaxValue();
    int getMinValue();
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
    void prp_startTransform();
    void prp_finishTransform();
protected:
    bool mTransformed = false;
    int mMinValue = 0;
    int mMaxValue = 9999;
    int mValue = 0;
signals:
    void valueChangedSignal(qreal);
};

#endif // INTPROPERTY_H
