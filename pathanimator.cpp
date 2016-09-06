#include "pathanimator.h"
#include "pathpoint.h"

PathAnimator::PathAnimator() : ComplexAnimator()
{

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
