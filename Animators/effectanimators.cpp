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

int EffectAnimators::prp_saveToSql(QSqlQuery *query,
                                    const int &boundingBoxSqlId) {
    foreach(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        ((PixmapEffect*)effect.data())->
                prp_saveToSql(query, boundingBoxSqlId);
    }
    return boundingBoxSqlId;
}
//  EFFECT_BLUR
//  EFFECT_SHADOW
//  EFFECT_LINES
//  EFFECT_CIRCLES
//  EFFECT_SWIRL
//  EFFECT_DESATURATE
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
            PixmapEffect *effect;
            if(typeT == EFFECT_BLUR) {
                effect = new BlurEffect();
            } else if(typeT == EFFECT_SHADOW) {
                effect = new ShadowEffect();
            } else if(typeT == EFFECT_LINES) {
                effect = new LinesEffect();
            } else if(typeT == EFFECT_CIRCLES) {
                effect = new CirclesEffect();
            } else if(typeT == EFFECT_SWIRL) {
                effect = new SwirlEffect();
            } else if(typeT == EFFECT_DESATURATE) {
                effect = new DesaturateEffect();
            } else if(typeT == EFFECT_IMPLODE) {
                effect = new ImplodeEffect();
            } else if(typeT == EFFECT_OIL) {
                effect = new OilEffect();
            } else if(typeT == EFFECT_ALPHA_MATTE) {
                effect = new AlphaMatteEffect(mParentBox);
            } else {
                continue;
            }
            effect->prp_loadFromSql(query.value(idId).toInt());
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
    foreach(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        ((PixmapEffect*)effect.data())->apply(target, imgPtr, img, scale);
    }
}

qreal EffectAnimators::getEffectsMargin() const {
    qreal newMargin = 2.;
    foreach(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        qreal effectMargin = ((PixmapEffect*)effect.data())->getMargin();
        newMargin += effectMargin;
    }
    return newMargin;
}

bool EffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}

void EffectAnimators::prp_makeDuplicate(Property *target) {
    EffectAnimators *eaTarget = ((EffectAnimators*)target);
    foreach(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        eaTarget->addEffect((PixmapEffect*)
                    ((PixmapEffect*)effect.data())->prp_makeDuplicate());
    }
}
