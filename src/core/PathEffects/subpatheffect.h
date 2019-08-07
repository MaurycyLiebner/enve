#ifndef SUBPATHEFFECT_H
#define SUBPATHEFFECT_H
#include "PathEffects/patheffect.h"
class SubPathEffect : public PathEffect {
    e_OBJECT
protected:
    SubPathEffect();
public:
    void apply(const qreal relFrame, const SkPath &src,
               SkPath * const dst);
private:
    qsptr<QrealAnimator> mMin;
    qsptr<QrealAnimator> mMax;
};

#endif // SUBPATHEFFECT_H
