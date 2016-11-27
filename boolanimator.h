#ifndef BOOLANIMATOR_H
#define BOOLANIMATOR_H
#include "intanimator.h"

class BoolAnimator : public IntAnimator
{
public:
    BoolAnimator();

    bool getCurrentBoolValue();
    void setCurrentBoolValue(bool bT, bool finish = false);
};

#endif // BOOLANIMATOR_H
