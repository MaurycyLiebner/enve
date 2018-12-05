#ifndef TRANSFORMUPDATER_H
#define TRANSFORMUPDATER_H
#include "propertyupdater.h"
class BasicTransformAnimator;

class TransformUpdater : public PropertyUpdater {
public:
    TransformUpdater(BasicTransformAnimator *transformAnimator);
    void update();
    void updateFinal();

    void frameChangeUpdate();
private:
    BasicTransformAnimator *mTarget;
};

#endif // TRANSFORMUPDATER_H
