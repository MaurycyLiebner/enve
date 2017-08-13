#ifndef BOXTARGETPROPERTY_H
#define BOXTARGETPROPERTY_H
#include <QtCore>
#include "property.h"
class BoundingBox;
class QrealAnimator;

class BoxTargetProperty;

#include "Boxes/boundingbox.h"
struct SumPathEffectForBoxLoad : public FunctionWaitingForBoxLoad {
    SumPathEffectForBoxLoad(const int &sqlBoxIdT,
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
//    int saveToSql(QSqlQuery *query,
//                      const int &parentId) {}
//    void loadFromSql(const int &identifyingId) {}

    bool SWT_isBoxTargetProperty() { return true; }

    int saveToSql(QSqlQuery *query);
    void loadFromSql(const int &identifyingId);
private:
    QWeakPointer<BoundingBox> mTarget;
};

#endif // BOXTARGETPROPERTY_H
