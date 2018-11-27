#ifndef QREALVALUEEFFECT_H
#define QREALVALUEEFFECT_H
#include "complexanimator.h"

class QrealValueEffect : public ComplexAnimator {
public:
    virtual qreal getDevAtRelFrame(const int &relFrame) = 0;

protected:
    QrealValueEffect(const QString& name);
};


#endif // QREALVALUEEFFECT_H
