#ifndef PATHEFFECTANIMATORS_H
#define PATHEFFECTANIMATORS_H
class PathEffect;
class BoundingBox;
#include "Animators/complexanimator.h"
#include "skia/skiaincludes.h"
class PathBox;

class PathEffectAnimators : public ComplexAnimator {
    friend class SelfRef;
public:
    void addEffect(const qsptr<PathEffect> &effect);
    bool hasEffects();

    bool SWT_isPathEffectAnimators() const;

    void apply(const qreal &relFrame, SkPath * const srcDstPath);
    void applyBeforeThickness(const qreal &relFrame,
                              SkPath * const srcDstPath);


    void readProperty(QIODevice *target);
    void writeProperty(QIODevice * const target) const;
    void removeEffect(const qsptr<PathEffect>& effect);
    BoundingBox *getParentBox();
    const bool &isOutline() const;
    const bool &isFill() const;
    void readPathEffect(QIODevice *target);
protected:
    PathEffectAnimators(const bool &isOutline,
                        const bool &isFill,
                        BoundingBox *parentPath);

    bool mIsOutline;
    bool mIsFill;
    qptr<BoundingBox> mParentBox;
};


#endif // PATHEFFECTANIMATORS_H
