#ifndef INTANIMATOR_H
#define INTANIMATOR_H
#include "Animators/qrealanimator.h"

class IntAnimator : public QrealAnimator
{
public:
    IntAnimator();

    int getCurrentIntValue() const;
    void setCurrentIntValue(const int &value, const bool &saveUndoRedo,
                            const bool &finish = false);

    bool SWT_isIntAnimator() { return true; }
};

#endif // INTANIMATOR_H
