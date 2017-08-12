#ifndef PATHEFFECTANIMATORS_H
#define PATHEFFECTANIMATORS_H
class PathEffect;
#include "Animators/complexanimator.h"
#include "skiaincludes.h"

class PathEffectAnimators : public ComplexAnimator
{
public:
    PathEffectAnimators();

    //void addPathEffect(PathEffect *effect);

//    int saveToSql(QSqlQuery *query, const int &boundingBoxSqlId);
//    void loadFromSql(const int &boundingBoxSqlId);

    bool hasEffects();

    //void makeDuplicate(Property *target);

    bool SWT_isEffectAnimators();


    void filterPath(SkPath *srcDstPath);
    void filterPathForRelFrame(const int &relFrame, SkPath *srcDstPath);
};


#endif // PATHEFFECTANIMATORS_H
