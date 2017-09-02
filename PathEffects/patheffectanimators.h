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
                        PathBox *parentPath);
    void addEffect(PathEffect *effect);
    bool hasEffects();

    //void makeDuplicate(Property *target);

    bool SWT_isEffectAnimators();


    void filterPath(SkPath *srcDstPath);
    void filterPathForRelFrame(const int &relFrame, SkPath *srcDstPath);

    void readPathEffectAnimators(std::fstream *file);
    void writePathEffectAnimators(std::fstream *file);
protected:
    bool mIsOutline;
    PathBox *mParentPath = NULL;
};


#endif // PATHEFFECTANIMATORS_H
