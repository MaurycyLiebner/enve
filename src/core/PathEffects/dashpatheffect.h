#ifndef DASHPATHEFFECT_H
#define DASHPATHEFFECT_H
#include "PathEffects/patheffect.h"

class DashPathEffect : public PathEffect {
    e_OBJECT
protected:
    DashPathEffect();
public:
    void apply(const qreal relFrame,
               const SkPath &src,
               SkPath * const dst);
private:
    qsptr<QrealAnimator> mSize;
};

#endif // DASHPATHEFFECT_H
