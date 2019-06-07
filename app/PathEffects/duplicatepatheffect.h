#ifndef DUPLICATEPATHEFFECT_H
#define DUPLICATEPATHEFFECT_H
#include "PathEffects/patheffect.h"
class IntAnimator;
class QPointFAnimator;

class DuplicatePathEffect : public PathEffect {
    friend class SelfRef;
protected:
    DuplicatePathEffect(const bool outlinePathEffect);
public:
    void apply(const qreal relFrame,
               const SkPath &src,
               SkPath * const dst);
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice * const src);
private:
    qsptr<IntAnimator> mCount;
    qsptr<QPointFAnimator> mTranslation;
};

#endif // DUPLICATEPATHEFFECT_H
