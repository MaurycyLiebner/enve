#ifndef ANIMATORSCOLLECTION_H
#define ANIMATORSCOLLECTION_H
#include "Animators/complexanimator.h"

class AnimatorsCollection : public ComplexAnimator
{
public:
    AnimatorsCollection();
    void addAnimator(QrealAnimator *animator);
    void removeAnimator(QrealAnimator *animator);
    void updateKeysPath();
    void sortKeys();
private:
    QList<QrealAnimator*> mChildAnimators;
};

#endif // ANIMATORSCOLLECTION_H
