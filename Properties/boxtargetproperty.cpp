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

int BoxTargetProperty::saveToSql(QSqlQuery *query) {
    QString boxTargetStr = "NULL";
    BoundingBox *targetBox = mTarget.data();
    bool addAwaitForBox = false;
    if(targetBox != NULL) {
        if(targetBox->wasBoxSaved()) {
            boxTargetStr = QString::number(targetBox->getSqlId());
        } else {
            addAwaitForBox = true;
        }
    }
    if(!query->exec(
                QString("INSERT INTO boxtargetproperty (targetboxid) "
                        "VALUES (%1)").
                arg(boxTargetStr) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }
    int boxId = query->lastInsertId().toInt();

    if(addAwaitForBox) {
        BoundingBox::addSqlInsertAwaitingBox(
             "UPDATE boxtargetproperty SET targetboxid=%1 "
             "WHERE id=" + QString::number(boxId),
             targetBox);
    }

    return boxId;
}

void BoxTargetProperty::loadFromSql(const int &identifyingId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM boxtargetproperty WHERE id = " +
            QString::number(identifyingId);
    if(query.exec(queryStr)) {
        query.next();
        int targetBoxId = query.value("targetboxid").toInt();
        BoundingBox *targetBox =
                BoundingBox::getLoadedBoxById(targetBoxId);
        if(targetBox == NULL) {
            BoundingBox::addFunctionWaitingForBoxLoad(
                        new SumPathEffectForBoxLoad(targetBoxId,
                                                    this));
        } else {
            setTarget(targetBox);
        }
    } else {
        qDebug() << "Could not load boxtargetproperty with id " <<
                    identifyingId;
    }
}

SumPathEffectForBoxLoad::SumPathEffectForBoxLoad(const int &sqlBoxIdT,
                                                 BoxTargetProperty *targetPropertyT) :
    FunctionWaitingForBoxLoad(sqlBoxIdT) {
    sqlBoxId = sqlBoxIdT;
    targetProperty = targetPropertyT;
}

void SumPathEffectForBoxLoad::boxLoaded(BoundingBox *box) {
    targetProperty->setTarget(box);
}
