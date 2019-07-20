#ifndef SUBDIVIDEPATHEFFECT_H
#define SUBDIVIDEPATHEFFECT_H
#include "PathEffects/patheffect.h"
class IntAnimator;

class SubdividePathEffect : public PathEffect {
    friend class SelfRef;
protected:
    SubdividePathEffect();
public:
    void apply(const qreal relFrame,
               const SkPath &src,
               SkPath * const dst);
private:
    qsptr<IntAnimator> mCount;
};

#endif // SUBDIVIDEPATHEFFECT_H
