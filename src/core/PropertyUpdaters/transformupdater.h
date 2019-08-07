#ifndef TRANSFORMUPDATER_H
#define TRANSFORMUPDATER_H
#include "PropertyUpdaters/propertyupdater.h"
class BasicTransformAnimator;

class TransformUpdater : public PropertyUpdater {
    e_OBJECT
protected:
    TransformUpdater(BasicTransformAnimator * const target);
public:
    void update();
    void finishedChange();

    void frameChangeUpdate();
private:
    BasicTransformAnimator * const mTarget;
};

#endif // TRANSFORMUPDATER_H
