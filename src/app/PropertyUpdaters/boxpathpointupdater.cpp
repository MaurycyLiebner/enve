#include "boxpathpointupdater.h"
#include "Boxes/boundingbox.h"
BoxPathPointUpdater::BoxPathPointUpdater(
        BasicTransformAnimator * const transAnim,
        BoundingBox * const targetBox) :
    TransformUpdater(transAnim), mTargetBox(targetBox) {}

void BoxPathPointUpdater::frameChangeUpdate() {
    TransformUpdater::frameChangeUpdate();
    mTargetBox->requestGlobalPivotUpdateIfSelected();
}
