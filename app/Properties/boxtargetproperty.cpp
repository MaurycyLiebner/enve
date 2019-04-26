#include "boxtargetproperty.h"
#include "Animators/complexanimator.h"
#include "Boxes/boundingbox.h"

BoxTargetProperty::BoxTargetProperty(const QString &name) :
    Property(name) {}

BoundingBox* BoxTargetProperty::getTarget() const {
    return mTarget_d;
}

void BoxTargetProperty::setTarget(BoundingBox* box) {
    if(mTarget_d) {
        QObject::disconnect(mTarget_d, nullptr, this, nullptr);
    }
    if(!box) {
        mTarget_d.clear();
    } else {
        mTarget_d = box;
    }
    if(mTarget_d) {
//        QObject::connect(mTarget_d, SIGNAL(scheduledUpdate()),
//                         this, SLOT(prp_callUpdater()));
    }
    prp_updateInfluenceRangeAfterChanged();
    emit targetSet(box);
}
