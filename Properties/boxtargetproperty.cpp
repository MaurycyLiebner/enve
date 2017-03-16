#include "boxtargetproperty.h"
#include "Animators/complexanimator.h"

BoxTargetProperty::BoxTargetProperty() :
    Property() {
    setName("target");
}

BoundingBox *BoxTargetProperty::getTarget() {
    return mTarget;
}

void BoxTargetProperty::setTarget(BoundingBox *box) {
    mTarget = box;
    mParentAnimator->callUpdater();
}
