#ifndef BOOLANIMATOR_H
#define BOOLANIMATOR_H
#include "intanimator.h"

class BoolAnimator : public IntAnimator
{
public:
    BoolAnimator();

    bool getCurrentBoolValue();
    void setCurrentBoolValue(const bool &bT,
                             const bool &finish = false);
    bool SWT_isBoolAnimator() { return true; }
};

#endif // BOOLANIMATOR_H
