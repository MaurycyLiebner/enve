#ifndef SOLIDIFYPATHEFFECT_H
#define SOLIDIFYPATHEFFECT_H
#include "PathEffects/patheffect.h"

class SolidifyPathEffect : public PathEffect {
    friend class SelfRef;
protected:
    SolidifyPathEffect();
public:
    void apply(const qreal relFrame,
               const SkPath &src,
               SkPath * const dst);
private:
    qsptr<QrealAnimator> mDisplacement;
};

#endif // SOLIDIFYPATHEFFECT_H
