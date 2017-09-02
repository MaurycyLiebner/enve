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
    if(mTarget.data() != NULL) {
        QObject::disconnect(mTarget.data(), 0, this, 0);
    }
    mTarget = box->weakRef<BoundingBox>();
    if(mTarget.data() != NULL) {
        QObject::connect(mTarget.data(), SIGNAL(scheduledUpdate()),
                         this, SLOT(prp_callUpdater()));
    }
    prp_callFinishUpdater();
}

void BoxTargetProperty::makeDuplicate(Property *property) {
    ((BoxTargetProperty*)property)->setTarget(mTarget.data());
}

Property *BoxTargetProperty::makeDuplicate() {
    BoxTargetProperty *prop = new BoxTargetProperty();
    makeDuplicate(prop);
    return prop;
}

SumPathEffectForBoxLoad::SumPathEffectForBoxLoad(const int &boxIdT,
                                                 BoxTargetProperty *targetPropertyT) :
    FunctionWaitingForBoxLoad(boxIdT) {
    targetProperty = targetPropertyT;
}

void SumPathEffectForBoxLoad::boxLoaded(BoundingBox *box) {
    targetProperty->setTarget(box);
}
