#ifndef SUMPATHEFFECT_H
#define SUMPATHEFFECT_H
#include "PathEffects/patheffect.h"

class SumPathEffect : public PathEffect {
    e_OBJECT
protected:
    SumPathEffect();
public:
    void apply(const qreal relFrame,
               const SkPath &src,
               SkPath * const dst);
};


#endif // SUMPATHEFFECT_H
