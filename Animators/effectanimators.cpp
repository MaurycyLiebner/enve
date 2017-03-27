#include "effectanimators.h"
#include "PixmapEffects/pixmapeffect.h"
#include "Boxes/boundingbox.h"
#include <QSqlError>
#include <QDebug>

EffectAnimators::EffectAnimators() :
    ComplexAnimator() {

}

void EffectAnimators::addEffect(PixmapEffect *effect) {
    mParentBox->addEffect(effect);
}

void EffectAnimators::prp_saveToSql(QSqlQuery *query,
                                    const int &boundingBoxSqlId) {
    foreach(Property *effect, ca_mChildAnimators) {
        ((PixmapEffect*)effect)->prp_saveToSql(query, boundingBoxSqlId);
    }
}

void EffectAnimators::prp_loadFromSql(const int &boundingBoxSqlId) {
    QSqlQuery query;
    QString queryStr;
    queryStr = "SELECT * FROM pixmapeffect WHERE boundingboxid = " +
               QString::number(boundingBoxSqlId);
    if(query.exec(queryStr) ) {
        int idId = query.record().indexOf("id");
        int typeId = query.record().indexOf("type");

        while(query.next() ) {
            PixmapEffectType typeT = static_cast<PixmapEffectType>(
                                            query.value(typeId).toInt());
            PixmapEffect *effect = PixmapEffect::loadFromSql(
                                            query.value(idId).toInt(),
                                            typeT);
            addEffect(effect);
        }
    } else {
        qDebug() << query.lastError() << endl << query.lastQuery();
    }
}

void EffectAnimators::setParentBox(BoundingBox *box) {
    mParentBox = box;
}

void EffectAnimators::applyEffects(BoundingBox *target,
                                   QImage *imgPtr,
                                   const fmt_filters::image &img,
                                   const qreal &scale) {
    foreach(Property *effect, ca_mChildAnimators) {
        ((PixmapEffect*)effect)->apply(target, imgPtr, img, scale);
    }
}

qreal EffectAnimators::getEffectsMargin() const {
    qreal newMargin = 2.;
    foreach(Property *effect, ca_mChildAnimators) {
        qreal effectMargin = ((PixmapEffect*)effect)->getMargin();
        newMargin += effectMargin;
    }
    return newMargin;
}

bool EffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}

void EffectAnimators::prp_makeDuplicate(Property *target) {
    EffectAnimators *eaTarget = ((EffectAnimators*)target);
    foreach(Property *effect, ca_mChildAnimators) {
        eaTarget->addEffect((PixmapEffect*)
                    ((PixmapEffect*)effect)->prp_makeDuplicate());
    }
}
