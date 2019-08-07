#ifndef BOXPATHPOINTUPDATER_H
#define BOXPATHPOINTUPDATER_H
#include "PropertyUpdaters/transformupdater.h"

class BoxPathPoint;
class BoundingBox;

class BoxPathPointUpdater : public TransformUpdater {
    e_OBJECT
protected:
    BoxPathPointUpdater(BasicTransformAnimator * const transAnim,
                        BoundingBox * const targetBox);
public:
    void frameChangeUpdate();
private:
    BoundingBox * const mTargetBox;
};

#endif // BOXPATHPOINTUPDATER_H
