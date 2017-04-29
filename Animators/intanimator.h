#ifndef INTANIMATOR_H
#define INTANIMATOR_H
#include "Animators/qrealanimator.h"

class IntAnimator : public QrealAnimator
{
public:
    IntAnimator();

    int getCurrentIntValue() const;
    void setCurrentIntValue(const int &value,
                            const bool &finish = false);
};

#endif // INTANIMATOR_H
