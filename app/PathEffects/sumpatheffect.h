#ifndef SUMPATHEFFECT_H
#define SUMPATHEFFECT_H
#include "PathEffects/patheffect.h"

class SumPathEffect : public PathEffect {
    friend class SelfRef;
protected:
    SumPathEffect(const bool outlinePathEffect);
public:
    void apply(const qreal relFrame,
               const SkPath &src,
               SkPath * const dst);
};


#endif // SUMPATHEFFECT_H
