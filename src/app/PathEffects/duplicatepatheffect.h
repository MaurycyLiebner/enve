#ifndef DUPLICATEPATHEFFECT_H
#define DUPLICATEPATHEFFECT_H
#include "PathEffects/patheffect.h"
class IntAnimator;
class QPointFAnimator;

class DuplicatePathEffect : public PathEffect {
    friend class SelfRef;
protected:
    DuplicatePathEffect();
public:
    void apply(const qreal relFrame,
               const SkPath &src,
               SkPath * const dst);
private:
    qsptr<IntAnimator> mCount;
    qsptr<QPointFAnimator> mTranslation;
};

#endif // DUPLICATEPATHEFFECT_H
