#ifndef ANIMATORSCOLLECTION_H
#define ANIMATORSCOLLECTION_H
#include "Animators/complexanimator.h"

class AnimatorsCollection : public ComplexAnimator
{
public:
    AnimatorsCollection();
    void setParentBoundingBox(BoundingBox *box);
    void addAnimator(QrealAnimator *animator);
    void removeAnimator(QrealAnimator *animator);
    void updateKeysPath();
    void sortKeys();
    void childAnimatorIsRecordingChanged();
private:
    BoundingBox *mParentBox;
    //QList<QrealAnimator*> mChildAnimators;
};

#endif // ANIMATORSCOLLECTION_H
