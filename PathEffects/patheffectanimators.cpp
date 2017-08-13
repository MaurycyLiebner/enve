#include "patheffectanimators.h"
#include "patheffect.h"
#include "Boxes/boundingbox.h"
#include <QSqlError>
#include <QDebug>

PathEffectAnimators::PathEffectAnimators(PathBox *parentPath) :
    ComplexAnimator() {
    mParentPath = parentPath;
}

//void PathEffectAnimators::addPathEffect(PathEffect *effect) {
//    mParentBox->addPathEffect(effect);
//}

int PathEffectAnimators::saveToSql(QSqlQuery *query,
                                   const int &boundingBoxSqlId,
                                   const bool &outline) {
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        ((PathEffect*)effect.data())->saveToSql(query,
                                                boundingBoxSqlId,
                                                outline);
    }
    return boundingBoxSqlId;
}
#include "pointhelpers.h"
void PathEffectAnimators::loadFromSql(const int &boundingBoxSqlId,
                                      const bool &outline) {
    QSqlQuery query;
    QString queryStr;
    queryStr = "SELECT * FROM patheffect WHERE boundingboxid = " +
               QString::number(boundingBoxSqlId) +
               " AND outline = " + boolToSql(outline);
    if(query.exec(queryStr) ) {
        int idId = query.record().indexOf("id");
        int typeId = query.record().indexOf("type");

        while(query.next() ) {
            PathEffectType typeT = static_cast<PathEffectType>(
                                            query.value(typeId).toInt());
            PathEffect *effect;
            if(typeT == DISCRETE_PATH_EFFECT) {
                effect = new DisplacePathEffect();
            } /*else if(typeT == DASH_PATH_EFFECT) {
                effect = new dashpat();
            } */else if(typeT == DUPLICATE_PATH_EFFECT) {
                effect = new DuplicatePathEffect();
            } else if(typeT == SUM_PATH_EFFECT) {
                effect = new SumPathEffect(mParentPath);
            } else {
                continue;
            }
            effect->loadFromSql(query.value(idId).toInt());
            ca_addChildAnimator(effect);
        }
    } else {
        qDebug() << query.lastError() << endl << query.lastQuery();
    }
}

//void PathEffectAnimators::applyEffectsSk(SkPaint *paint) {
//    updatePathEffectsSum();
//    if(hasEffects()) {
//        paint->setPathEffect(mPathEffectsSum);
//    }
//}

bool PathEffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}

bool PathEffectAnimators::SWT_isEffectAnimators() { return true; }

void PathEffectAnimators::filterPath(SkPath *srcDstPath) {
    SkPath dstPath = *srcDstPath;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        SkPath srcPath = dstPath;
        ((PathEffect*)effect.data())->filterPath(srcPath, &dstPath);
    }
    *srcDstPath = dstPath;
}

void PathEffectAnimators::filterPathForRelFrame(const int &relFrame,
                                                SkPath *srcDstPath) {
    SkPath dstPath = *srcDstPath;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        SkPath srcPath = dstPath;
        ((PathEffect*)effect.data())->filterPathForRelFrame(relFrame,
                                                            srcPath,
                                                            &dstPath);
    }
    *srcDstPath = dstPath;
}

//void PathEffectAnimators::makeDuplicate(Property *target) {
//    PathEffectAnimators *eaTarget = ((PathEffectAnimators*)target);
//    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
//        eaTarget->addEffect((PathEffect*)
//                    ((PathEffect*)effect.data())->makeDuplicate());
//    }
//}
