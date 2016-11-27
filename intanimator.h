#ifndef INTANIMATOR_H
#define INTANIMATOR_H
#include "qrealanimator.h"

class IntAnimator : public QrealAnimator
{
public:
    IntAnimator();

    int getCurrentIntValue() const;
    void setCurrentIntValue(int value, bool finish = false);
};

#endif // INTANIMATOR_H
