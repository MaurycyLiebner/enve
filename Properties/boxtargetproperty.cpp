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
            QObject::disconnect(mTarget, 0, mParentBox, 0);
        }
        mTarget->decUsedAsTarget();
    }
    mTarget = box;
    if(mTarget != NULL) {
        if(mParentBox != NULL) {
            QObject::connect(mTarget, SIGNAL(scheduledUpdate()),
                             mParentBox, SLOT(scheduleSoftUpdate()));
            QObject::connect(mTarget, SIGNAL(replaceChacheSet()),
                             mParentBox, SLOT(replaceCurrentFrameCache()));
        }
        mTarget->incUsedAsTarget();
    }
    prp_callUpdater();
}

void BoxTargetProperty::setParentBox(BoundingBox *box) {
    mParentBox = box;
}

void BoxTargetProperty::makeDuplicate(BoxTargetProperty *property) {
    property->setTarget(mTarget);
}

BoxTargetProperty *BoxTargetProperty::makeDuplicate() {
    BoxTargetProperty *prop = new BoxTargetProperty();
    makeDuplicate(prop);
    return prop;
}
