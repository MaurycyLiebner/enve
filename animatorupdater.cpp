#include "animatorupdater.h"

#include "vectorpath.h"

TransUpdater::TransUpdater(BoundingBox *boundingBox) : AnimatorUpdater()
{
    mTarget = boundingBox;
}

void TransUpdater::update() {
    mTarget->updateCombinedTransform();
    mTarget->schedulePivotUpdate();
}

PathPointUpdater::PathPointUpdater(VectorPath *vectorPath)
{
    mTarget = vectorPath;
}

void PathPointUpdater::update()
{
    mTarget->schedulePathUpdate();
}
