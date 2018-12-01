#ifndef BOXTARGETPROPERTY_H
#define BOXTARGETPROPERTY_H
#include <QtCore>
#include "property.h"
class BoundingBox;
class QrealAnimator;

class BoxTargetProperty;

#include "Boxes/boundingbox.h"
struct BoxTargetPropertyWaitingForBoxLoad : public FunctionWaitingForBoxLoad {
    BoxTargetPropertyWaitingForBoxLoad(
            const int &boxIdT, BoxTargetProperty* targetPropertyT);

    void boxLoaded(BoundingBox *box);

    qptr<BoxTargetProperty> targetProperty;
};

class BoxTargetProperty : public Property {
    Q_OBJECT
public:
    BoxTargetProperty(const QString& name);

    BoundingBox *getTarget() const;
    void setTarget(BoundingBox *box);

    bool SWT_isBoxTargetProperty() { return true; }
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
signals:
    void targetSet(BoundingBox*);
private:
    qptr<BoundingBox> mTarget_d;
};

#endif // BOXTARGETPROPERTY_H
