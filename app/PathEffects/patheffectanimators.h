#ifndef PATHEFFECTANIMATORS_H
#define PATHEFFECTANIMATORS_H
class PathEffect;
class BoundingBox;
#include "Animators/complexanimator.h"
#include "skia/skiaincludes.h"
class PathBox;

class PathEffectAnimators : public ComplexAnimator {
    friend class SelfRef;
protected:
    PathEffectAnimators(const bool &isOutline,
                        const bool &isFill,
                        BoundingBox * const parentPath);
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
private:
    bool mIsOutline;
    bool mIsFill;
    qptr<BoundingBox> mParentBox;
};


#endif // PATHEFFECTANIMATORS_H
