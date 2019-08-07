#ifndef LINESPATHEFFECT_H
#define LINESPATHEFFECT_H
#include "PathEffects/patheffect.h"

class LinesPathEffect : public PathEffect {
    e_OBJECT
protected:
    LinesPathEffect();
public:
    void apply(const qreal relFrame,
               const SkPath &src,
               SkPath * const dst);
private:
    qsptr<QrealAnimator> mAngle;
    qsptr<QrealAnimator> mDistance;
};

#endif // LINESPATHEFFECT_H
