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
    void setIntValueRange(const int &minVal, const int &maxVal);

    bool SWT_isIntAnimator() { return true; }
    int getCurrentIntValueAtRelFrame(const int &relFrame) const;
    int getCurrentIntValueAtRelFrameF(const qreal &relFrame) const;
};

#endif // INTANIMATOR_H
