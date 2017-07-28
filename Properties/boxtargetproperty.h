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
    void setParentBox(BoundingBox *box);

    void makeDuplicate(Property *property);
    Property *makeDuplicate();
//    int saveToSql(QSqlQuery *query,
//                      const int &parentId) {}
//    void loadFromSql(const int &identifyingId) {}

    bool SWT_isBoxTargetProperty() { return true; }
private:
    QWeakPointer<BoundingBox> mTarget;
    BoundingBox *mParentBox = NULL;
};

#endif // BOXTARGETPROPERTY_H
