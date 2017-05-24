#ifndef BOXTARGETPROPERTY_H
#define BOXTARGETPROPERTY_H
#include <QtCore>
#include "property.h"
class BoundingBox;

class BoxTargetProperty : public Property {
public:
    BoxTargetProperty();

    BoundingBox *getTarget();
    void setTarget(BoundingBox *box);
    void setParentBox(BoundingBox *box);

    void makeDuplicate(BoxTargetProperty *property);
    BoxTargetProperty *makeDuplicate();

    bool SWT_isBoxTargetProperty() { return true; }
private:
    BoundingBox *mTarget = NULL;
    BoundingBox *mParentBox = NULL;
};

#endif // BOXTARGETPROPERTY_H
