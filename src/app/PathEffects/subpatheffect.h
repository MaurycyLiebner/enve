#ifndef SUBPATHEFFECT_H
#define SUBPATHEFFECT_H
#include "PathEffects/patheffect.h"
class SubPathEffect : public PathEffect {
    friend class SelfRef;
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
