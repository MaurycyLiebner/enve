#ifndef BOOLANIMATOR_H
#define BOOLANIMATOR_H
#include "intanimator.h"

class BoolAnimator : public IntAnimator {
    e_OBJECT
public:
    bool getCurrentBoolValue();
    void setCurrentBoolValue(const bool bT);
    bool getCurrentBoolValueAtRelFrame(const qreal relFrame) {
        return getBaseIntValue(relFrame) == 1;
    }

    bool SWT_isBoolAnimator() const { return true; }
protected:
    BoolAnimator(const QString& name);
};

#endif // BOOLANIMATOR_H
