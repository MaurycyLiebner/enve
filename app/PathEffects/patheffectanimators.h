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

    void filterPathForRelFrame(const int &relFrame,
                               SkPath *srcDstPath,
                               const qreal &scale = 1.,
                               const bool &groupPathSum = false);
    void filterPathForRelFrameUntilGroupSum(const int &relFrame,
                                            SkPath *srcDstPath,
                                            const qreal &scale = 1.);
    void filterPathForRelFrameBeforeThickness(const int &relFrame,
                                              SkPath *srcDstPath,
                                              const qreal &scale = 1.);
    void filterPathForRelFrameF(const qreal &relFrame,
                               SkPath *srcDstPath,
                               const bool &groupPathSum = false);
    void filterPathForRelFrameUntilGroupSumF(const qreal &relFrame,
                                            SkPath *srcDstPath);
    void filterPathForRelFrameBeforeThicknessF(const qreal &relFrame,
                                              SkPath *srcDstPath);


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
