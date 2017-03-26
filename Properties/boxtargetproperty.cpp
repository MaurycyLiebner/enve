#include "boxtargetproperty.h"
#include "Animators/complexanimator.h"
#include "Boxes/boundingbox.h"

BoxTargetProperty::BoxTargetProperty() :
    Property() {
    prp_setName("target");
}

BoundingBox *BoxTargetProperty::getTarget() {
    return mTarget;
}

void BoxTargetProperty::setTarget(BoundingBox *box) {
    if(mParentBox != NULL) {
        if(mTarget != NULL) {
            QObject::disconnect(mTarget, SIGNAL(scheduledUpdate()),
                                mParentBox, SLOT(scheduleUpdate()));
            mTarget->decUsedAsTarget();
        }
    }
    mTarget = box;
    if(mParentBox != NULL) {
        if(mTarget != NULL) {
            QObject::connect(mTarget, SIGNAL(scheduledUpdate()),
                             mParentBox, SLOT(scheduleUpdate()));
            mTarget->incUsedAsTarget();
        }
    }
    prp_mParentAnimator->prp_callUpdater();
}

void BoxTargetProperty::setParentBox(BoundingBox *box) {
    mParentBox = box;
}
