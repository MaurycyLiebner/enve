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
            RenderCacheHandler *handler = mTarget->getRenderHandler();
            QObject::disconnect(handler, 0, mParentBox->getRenderHandler(), 0);
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
            RenderCacheHandler *handler = mTarget->getRenderHandler();
            QObject::connect(handler,
                             SIGNAL(clearedCacheForAbsFrameRange(int,int)),
                             mParentBox->getRenderHandler(),
                             SLOT(clearCacheForAbsFrameRange(int,int)));
        }
        mTarget->incUsedAsTarget();
    }
    prp_callUpdater();
}

void BoxTargetProperty::setParentBox(BoundingBox *box) {
    mParentBox = box;
}

void BoxTargetProperty::makeDuplicate(Property *property) {
    ((BoxTargetProperty*)property)->setTarget(mTarget);
}

Property *BoxTargetProperty::makeDuplicate() {
    BoxTargetProperty *prop = new BoxTargetProperty();
    makeDuplicate(prop);
    return prop;
}
