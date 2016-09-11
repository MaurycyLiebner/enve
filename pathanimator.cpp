#include "pathanimator.h"
#include "pathpoint.h"

PathAnimator::PathAnimator() : ComplexAnimator()
{
    setName("path");
}

void PathAnimator::addPathPoint(PathPoint *pathPoint)
{
    PathPointAnimators *animators = pathPoint->getPathPointAnimatorsPtr();
    mPathPointAnimators << animators;
    addPointPosAnimator(animators->endPosAnimator);
    addPointPosAnimator(animators->startPosAnimator);
    addPointPosAnimator(animators->pathPointPosAnimator);
}

void PathAnimator::removePathPoint(PathPoint *pathPoint)
{
    PathPointAnimators *animators = pathPoint->getPathPointAnimatorsPtr();
    mPathPointAnimators.removeOne(animators);
    removePointPosAnimator(animators->endPosAnimator);
    removePointPosAnimator(animators->startPosAnimator);
    removePointPosAnimator(animators->pathPointPosAnimator);
}

void PathAnimator::addPointPosAnimator(QrealAnimator *pointPosAnimator)
{
    pointPosAnimator->setParentAnimator(this);

    pointPosAnimator->addAllKeysToComplexAnimator();
}

void PathAnimator::removePointPosAnimator(QrealAnimator *pointPosAnimator)
{
    pointPosAnimator->removeAllKeysFromComplexAnimator();

    pointPosAnimator->setParentAnimator(NULL);
}

void PathAnimator::updateKeysPath()
{
    QrealAnimator::updateKeysPath();
    foreach(PathPointAnimators *animator, mPathPointAnimators) {
        animator->updateKeysPath();
    }
}

void PathAnimator::sortKeys()
{
    QrealAnimator::sortKeys();
    foreach(PathPointAnimators *animator, mPathPointAnimators) {
        animator->sortKeys();
    }
}
