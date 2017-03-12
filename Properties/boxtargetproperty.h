#ifndef BOXTARGETPROPERTY_H
#define BOXTARGETPROPERTY_H
#include <QtCore>
#include "property.h"
class BoundingBox;
class QrealAnimator;

class BoxTargetProperty : public Property {
public:
    BoxTargetProperty();

    BoundingBox *getTarget();
    void setTarget(BoundingBox *box);
    SWT_Type SWT_getType() { return SWT_BoxTarget; }

private:
    BoundingBox *mTarget = NULL;
};

#endif // BOXTARGETPROPERTY_H
