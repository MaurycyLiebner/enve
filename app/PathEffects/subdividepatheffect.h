#ifndef SUBDIVIDEPATHEFFECT_H
#define SUBDIVIDEPATHEFFECT_H
#include "PathEffects/patheffect.h"
class IntAnimator;

class SubdividePathEffect : public PathEffect {
    friend class SelfRef;
protected:
    SubdividePathEffect(const bool outlinePathEffect);
public:
    void apply(const qreal relFrame,
               const SkPath &src,
               SkPath * const dst);
    void writeProperty(QIODevice * const dst) const;
    void readProperty(QIODevice * const src);
private:
    qsptr<IntAnimator> mCount;
};

#endif // SUBDIVIDEPATHEFFECT_H
