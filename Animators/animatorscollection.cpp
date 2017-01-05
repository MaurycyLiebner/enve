#include "Animators/animatorscollection.h"

AnimatorsCollection::AnimatorsCollection() : ComplexAnimator()
{

}

void AnimatorsCollection::addAnimator(QrealAnimator *animator)
{
    animator->setParentAnimator(this);

    animator->addAllKeysToComplexAnimator();

    mChildAnimators.append(animator);
}

void AnimatorsCollection::removeAnimator(QrealAnimator *animator)
{
    animator->removeAllKeysFromComplexAnimator();

    animator->setParentAnimator(NULL);

    mChildAnimators.removeOne(animator);
}

void AnimatorsCollection::updateKeysPath()
{
    QrealAnimator::updateKeysPath();
    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->updateKeysPath();
    }
}

void AnimatorsCollection::sortKeys()
{
    QrealAnimator::sortKeys();
    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->sortKeys();
    }
}
