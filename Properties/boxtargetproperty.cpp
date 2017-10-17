#include "boxtargetproperty.h"
#include "Animators/complexanimator.h"
#include "Boxes/boundingbox.h"

BoxTargetProperty::BoxTargetProperty() :
    Property() {
    prp_setName("target");
}

BoundingBox *BoxTargetProperty::getTarget() const {
    return mTarget.data();
}

void BoxTargetProperty::setTarget(BoundingBox *box) {
    if(mTarget.data() != NULL) {
        QObject::disconnect(mTarget.data(), 0, this, 0);
    }
    if(box == NULL) {
        mTarget.clear();
    } else {
        mTarget = box->weakRef<BoundingBox>();
    }
    if(mTarget.data() != NULL) {
        QObject::connect(mTarget.data(), SIGNAL(scheduledUpdate()),
                         this, SLOT(prp_callUpdater()));
    }
    prp_updateInfluenceRangeAfterChanged();
    prp_callUpdater();
    prp_callFinishUpdater();
    emit targetSet(box);
}

BoxTargetPropertyWaitingForBoxLoad::BoxTargetPropertyWaitingForBoxLoad(const int &boxIdT,
                                                 BoxTargetProperty *targetPropertyT) :
    FunctionWaitingForBoxLoad(boxIdT) {
    targetProperty = targetPropertyT;
}

void BoxTargetPropertyWaitingForBoxLoad::boxLoaded(BoundingBox *box) {
    targetProperty->setTarget(box);
}
