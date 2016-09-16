#include "pathanimator.h"
#include "pathpoint.h"
#include "boxeslist.h"

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

qreal PathAnimator::getBoxesListHeight()
{
    if(mBoxesListDetailVisible) {
        qreal height = LIST_ITEM_HEIGHT;
        foreach(PathPointAnimators *animator, mPathPointAnimators) {
            height += animator->getBoxesListHeight();
        }
        return height;
    } else {
        return LIST_ITEM_HEIGHT;
    }
}

void PathAnimator::drawBoxesList(QPainter *p,
                                 qreal drawX, qreal drawY,
                                 qreal pixelsPerFrame,
                                 int startFrame, int endFrame)
{
    QrealAnimator::drawBoxesList(p, drawX, drawY,
                                 pixelsPerFrame, startFrame, endFrame);
    if(mBoxesListDetailVisible) {
        drawX += LIST_ITEM_CHILD_INDENT;
        drawY += LIST_ITEM_HEIGHT;

        foreach(PathPointAnimators *animator, mPathPointAnimators) {
            animator->drawBoxesList(p, drawX, drawY,
                                    pixelsPerFrame,
                                    startFrame, endFrame);
            drawY += animator->getBoxesListHeight();
        }
    }
}

QrealKey *PathAnimator::getKeyAtPos(qreal relX, qreal relY,
                                     int minViewedFrame,
                                     qreal pixelsPerFrame) {
    if(relY <= LIST_ITEM_HEIGHT) {
        return QrealAnimator::getKeyAtPos(relX, relY,
                                   minViewedFrame, pixelsPerFrame);
    } else if(mBoxesListDetailVisible) {
        relY -= LIST_ITEM_HEIGHT;
        foreach(PathPointAnimators *animator, mPathPointAnimators) {
            qreal height = animator->getBoxesListHeight();
            if(relY <= height) {
                return animator->getKeyAtPos(relX, relY,
                                      minViewedFrame, pixelsPerFrame);
            }
            relY -= height;
        }
    }
    return NULL;
}
