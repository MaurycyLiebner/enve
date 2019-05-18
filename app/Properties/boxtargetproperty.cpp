#include "boxtargetproperty.h"
#include "Animators/complexanimator.h"
#include "Boxes/boundingbox.h"

BoxTargetProperty::BoxTargetProperty(const QString &name) :
    Property(name) {}

BoundingBox* BoxTargetProperty::getTarget() const {
    return mTarget_d;
}

void BoxTargetProperty::setTarget(BoundingBox* box) {
    if(!box) {
        mTarget_d.clear();
    } else {
        mTarget_d = box;
    }

    prp_afterWholeInfluenceRangeChanged();
    emit targetSet(box);
}
