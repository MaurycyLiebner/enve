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
    if(mTarget != NULL) {
        if(mParentBox != NULL) {
            QObject::disconnect(mTarget, SIGNAL(scheduledUpdate()),
                                mParentBox, SLOT(scheduleHardUpdate()));
            QObject::disconnect(mTarget, SIGNAL(replaceChacheSet()),
                                mParentBox, SLOT(replaceCurrentFrameCache()));
        }
        mTarget->decUsedAsTarget();
    }
    mTarget = box;
    if(mTarget != NULL) {
        if(mParentBox != NULL) {
            QObject::connect(mTarget, SIGNAL(scheduledUpdate()),
                             mParentBox, SLOT(scheduleHardUpdate()));
            QObject::connect(mTarget, SIGNAL(replaceChacheSet()),
                             mParentBox, SLOT(replaceCurrentFrameCache()));
        }
        mTarget->incUsedAsTarget();
    }
    prp_mParentAnimator->prp_callUpdater();
}

void BoxTargetProperty::setParentBox(BoundingBox *box) {
    mParentBox = box;
}

void BoxTargetProperty::prp_makeDuplicate(Property *property) {
    ((BoxTargetProperty*)property)->setTarget(mTarget);
}

Property *BoxTargetProperty::prp_makeDuplicate() {
    BoxTargetProperty *prop = new BoxTargetProperty();
    prp_makeDuplicate(prop);
    return prop;
}
