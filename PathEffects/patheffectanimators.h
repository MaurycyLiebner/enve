#ifndef PATHEFFECTANIMATORS_H
#define PATHEFFECTANIMATORS_H
class PathEffect;
class BoundingBox;
#include "Animators/complexanimator.h"
#include "skiaincludes.h"
class PathBox;

class PathEffectAnimators : public ComplexAnimator
{
public:
    PathEffectAnimators(const bool &isOutline,
                        const bool &isFill,
                        BoundingBox *parentPath);
    void addEffect(PathEffect *effect);
    bool hasEffects();

    bool SWT_isPathEffectAnimators() {
        return true;
    }

    void filterPathForRelFrame(const int &relFrame,
                               SkPath *srcDstPath,
                               const bool &groupPathSum = false);
    void filterPathForRelFrameUntilGroupSum(const int &relFrame,
                                            SkPath *srcDstPath);
    void readProperty(QIODevice *target);
    void writeProperty(QIODevice *target);
    void filterPathForRelFrameBeforeThickness(const int &relFrame,
                                              SkPath *srcDstPath);

    void removeEffect(PathEffect *effect);
    BoundingBox *getParentBox() {
        return mParentPath;
    }
    const bool &isOutline() const {
        return mIsOutline;
    }
    const bool &isFill() const {
        return mIsFill;
    }
    void readPathEffect(QIODevice *target);
protected:
    bool mIsOutline;
    bool mIsFill;
    BoundingBox *mParentPath = NULL;
};


#endif // PATHEFFECTANIMATORS_H
