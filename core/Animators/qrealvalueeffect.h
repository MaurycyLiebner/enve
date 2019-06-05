#ifndef QREALVALUEEFFECT_H
#define QREALVALUEEFFECT_H
#include "Animators/complexanimator.h"

class QrealValueEffect : public ComplexAnimator {
public:
    virtual qreal getDevAtRelFrame(const qreal relFrame) = 0;

protected:
    QrealValueEffect(const QString& name);
};


#endif // QREALVALUEEFFECT_H
