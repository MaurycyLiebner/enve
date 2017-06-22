#include "patheffectanimators.h"
#include "patheffect.h"
#include "Boxes/boundingbox.h"
#include <QSqlError>
#include <QDebug>

PathEffectAnimators::PathEffectAnimators() :
    ComplexAnimator() {

}

//void PathEffectAnimators::addPathEffect(PathEffect *effect) {
//    mParentBox->addPathEffect(effect);
//}

//int PathEffectAnimators::prp_saveToSql(QSqlQuery *query,
//                                    const int &boundingBoxSqlId) {
//    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
//        ((PathEffect*)effect.data())->
//                prp_saveToSql(query, boundingBoxSqlId);
//    }
//    return boundingBoxSqlId;
//}

//void PathEffectAnimators::prp_loadFromSql(const int &boundingBoxSqlId) {
//    QSqlQuery query;
//    QString queryStr;
//    queryStr = "SELECT * FROM PathEffect WHERE boundingboxid = " +
//               QString::number(boundingBoxSqlId);
//    if(query.exec(queryStr) ) {
//        int idId = query.record().indexOf("id");
//        int typeId = query.record().indexOf("type");

//        while(query.next() ) {
//            PathEffectType typeT = static_cast<PathEffectType>(
//                                            query.value(typeId).toInt());
//            PathEffect *effect;
//            if(typeT == EFFECT_BLUR) {
//                effect = new BlurEffect();
//            } else if(typeT == EFFECT_SHADOW) {
//                effect = new ShadowEffect();
//            } else if(typeT == EFFECT_LINES) {
//                effect = new LinesEffect();
//            } else if(typeT == EFFECT_CIRCLES) {
//                effect = new CirclesEffect();
//            } else if(typeT == EFFECT_SWIRL) {
//                effect = new SwirlEffect();
//            } else if(typeT == EFFECT_DESATURATE) {
//                effect = new DesaturateEffect();
//            } else if(typeT == EFFECT_IMPLODE) {
//                effect = new ImplodeEffect();
//            } else if(typeT == EFFECT_OIL) {
//                effect = new OilEffect();
//            } else if(typeT == EFFECT_ALPHA_MATTE) {
//                effect = new AlphaMatteEffect(mParentBox);
//            } else {
//                continue;
//            }
//            effect->prp_loadFromSql(query.value(idId).toInt());
//            addEffect(effect);
//        }
//    } else {
//        qDebug() << query.lastError() << endl << query.lastQuery();
//    }
//}

//void PathEffectAnimators::applyEffectsSk(SkPaint *paint) {
//    updatePathEffectsSum();
//    if(hasEffects()) {
//        paint->setPathEffect(mPathEffectsSum);
//    }
//}

qreal PathEffectAnimators::getEffectsMargin() const {
    qreal newMargin = 0.;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        qreal effectMargin = ((PathEffect*)effect.data())->getMargin();
        newMargin += effectMargin;
    }
    return newMargin;
}

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
