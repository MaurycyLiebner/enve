#ifndef QREALVALUEEFFECT_H
#define QREALVALUEEFFECT_H
#include "complexanimator.h"

class QrealValueEffect : public ComplexAnimator {
public:
    QrealValueEffect();
    virtual qreal getDevAtRelFrame(const int &relFrame) = 0;
};


#endif // QREALVALUEEFFECT_H
