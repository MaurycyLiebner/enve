#include "animatorupdater.h"
#include <QDebug>
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
