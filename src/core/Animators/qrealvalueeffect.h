#ifndef QREALVALUEEFFECT_H
#define QREALVALUEEFFECT_H
#include "staticcomplexanimator.h"

class QrealValueEffect : public StaticComplexAnimator {
public:
    virtual qreal getDevAtRelFrame(const qreal relFrame) = 0;

protected:
    QrealValueEffect(const QString& name);
};


#endif // QREALVALUEEFFECT_H
