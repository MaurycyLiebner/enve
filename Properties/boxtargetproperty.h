#ifndef BOXTARGETPROPERTY_H
#define BOXTARGETPROPERTY_H
#include <QtCore>
#include "property.h"
class BoundingBox;
class QrealAnimator;

class BoxTargetProperty;

#include "Boxes/boundingbox.h"
struct SumPathEffectForBoxLoad : public FunctionWaitingForBoxLoad {
    SumPathEffectForBoxLoad(const int &boxIdT,
                            BoxTargetProperty *targetPropertyT);

    void boxLoaded(BoundingBox *box);

    BoxTargetProperty *targetProperty;
};

class BoxTargetProperty : public Property {
public:
    BoxTargetProperty();

    BoundingBox *getTarget();
    void setTarget(BoundingBox *box);

    void makeDuplicate(Property *property);
    Property *makeDuplicate();

    bool SWT_isBoxTargetProperty() { return true; }
    void writeBoxTargetProperty(QFile *file);
    void readBoxTargetProperty(QFile *file);
private:
    QWeakPointer<BoundingBox> mTarget;
};

#endif // BOXTARGETPROPERTY_H
