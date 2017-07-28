#include "boxtargetproperty.h"
#include "Animators/complexanimator.h"
#include "Boxes/boundingbox.h"

BoxTargetProperty::BoxTargetProperty() :
    Property() {
    prp_setName("target");
}

BoundingBox *BoxTargetProperty::getTarget() {
    return mTarget.data();
}

void BoxTargetProperty::setTarget(BoundingBox *box) {
//    if(mTarget.data() != NULL) {
//        if(mParentBox != NULL) {
//            QObject::disconnect(mTarget.data(), 0, mParentBox, 0);
//        }
//    }
    mTarget = box->weakRef<BoundingBox>();
//    if(mTarget.data() != NULL) {
//        if(mParentBox != NULL) {
//            QObject::connect(mTarget.data(), SIGNAL(scheduledUpdate()),
//                             mParentBox, SLOT(scheduleHardUpdate()));
//            QObject::connect(mTarget.data(), SIGNAL(replaceChacheSet()),
//                             mParentBox, SLOT(replaceCurrentFrameCache()));
//        }
//    }
    prp_callUpdater();
}

void BoxTargetProperty::setParentBox(BoundingBox *box) {
    mParentBox = box;
}

void BoxTargetProperty::makeDuplicate(Property *property) {
    ((BoxTargetProperty*)property)->setTarget(mTarget.data());
}

Property *BoxTargetProperty::makeDuplicate() {
    BoxTargetProperty *prop = new BoxTargetProperty();
    makeDuplicate(prop);
    return prop;
}
