#ifndef PATHEFFECTANIMATORS_H
#define PATHEFFECTANIMATORS_H
class PathEffect;
#include "Animators/complexanimator.h"
#include "skiaincludes.h"
class PathBox;

class PathEffectAnimators : public ComplexAnimator
{
public:
    PathEffectAnimators(const bool &isOutline,
                        const bool &isFill,
                        PathBox *parentPath);
    void addEffect(PathEffect *effect);
    bool hasEffects();

    //void makeDuplicate(Property *target);

    bool SWT_isEffectAnimators();


    void filterPath(SkPath *srcDstPath);
    void filterPathForRelFrame(const int &relFrame, SkPath *srcDstPath);

    void readProperty(QIODevice *target, const bool &outline);
    void writeProperty(QIODevice *target);
    void filterPathForRelFrameBeforeThickness(const int &relFrame,
                                              SkPath *srcDstPath);
protected:
    bool mIsOutline;
    bool mIsFill;
    PathBox *mParentPath = NULL;
};


#endif // PATHEFFECTANIMATORS_H
