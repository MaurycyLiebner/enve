#ifndef ZIGZAGPATHEFFECT_H
#define ZIGZAGPATHEFFECT_H
#include "PathEffects/patheffect.h"

class ZigZagPathEffect : public PathEffect {
    e_OBJECT
protected:
    ZigZagPathEffect();
public:
    void apply(const qreal relFrame,
               const SkPath &src,
               SkPath * const dst);
private:
    qsptr<QrealAnimator> mAngle;
    qsptr<QrealAnimator> mDistance;
};

#endif // ZIGZAGPATHEFFECT_H
