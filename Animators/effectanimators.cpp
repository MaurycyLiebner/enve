#include "effectanimators.h"
#include "pixmapeffect.h"
#include "Boxes/boundingbox.h"
#include <QSqlError>
#include <QDebug>

EffectAnimators::EffectAnimators() :
    ComplexAnimator() {

}

void EffectAnimators::saveToSql(QSqlQuery *query, int boundingBoxSqlId) {
    foreach(QrealAnimator *effect, mChildAnimators) {
        ((PixmapEffect*)effect)->saveToSql(query, boundingBoxSqlId);
    }
}

void EffectAnimators::loadFromSql(int boundingBoxSqlId,
                                  BoundingBox *box) {
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
            box->addEffect(effect);
        }
    } else {
        qDebug() << query.lastError() << endl << query.lastQuery();
    }
}

void EffectAnimators::applyEffects(BoundingBox *target,
                                   QImage *imgPtr,
                                   const fmt_filters::image &img,
                                   const qreal &scale,
                                   const bool &highQuality) {
    foreach(QrealAnimator *effect, mChildAnimators) {
        ((PixmapEffect*)effect)->apply(target, imgPtr, img, scale, highQuality);
    }
}

qreal EffectAnimators::getEffectsMargin() const {
    qreal newMargin = 5.;
    foreach(QrealAnimator *effect, mChildAnimators) {
        qreal effectMargin = ((PixmapEffect*)effect)->getMargin();
        newMargin += effectMargin;
    }
    return newMargin;
}
