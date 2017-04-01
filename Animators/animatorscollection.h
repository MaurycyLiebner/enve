#ifndef ANIMATORSCOLLECTION_H
#define ANIMATORSCOLLECTION_H
#include "Animators/complexanimator.h"

class AnimatorsCollection : public ComplexAnimator
{
public:
    AnimatorsCollection();
    void setParentBoundingBox(BoundingBox *box);
//    void updateKeysPath();
//    void sortKeys();
    void ca_childAnimatorIsRecordingChanged();
    int prp_getParentFrameShift() const;
private:
    BoundingBox *mParentBox;
    //QList<QrealAnimator*> mChildAnimators;
};

#endif // ANIMATORSCOLLECTION_H
