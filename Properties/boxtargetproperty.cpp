#include "boxtargetproperty.h"
#include "Animators/complexanimator.h"
#include "Boxes/boundingbox.h"

BoxTargetProperty::BoxTargetProperty() :
    Property() {
    setName("target");
}

BoundingBox *BoxTargetProperty::getTarget() {
    return mTarget;
}

void BoxTargetProperty::setTarget(BoundingBox *box) {
    if(mParentBox != NULL) {
        if(mTarget != NULL) {
            QObject::disconnect(mTarget, SIGNAL(scheduledUpdate()),
                                mParentBox, SLOT(scheduleUpdate()));
        }
    }
    mTarget = box;
    if(mParentBox != NULL) {
        if(mTarget != NULL) {
            QObject::connect(mTarget, SIGNAL(scheduledUpdate()),
                             mParentBox, SLOT(scheduleUpdate()));
        }
    }
    mParentAnimator->callUpdater();
}

void BoxTargetProperty::setParentBox(BoundingBox *box) {
    mParentBox = box;
}
