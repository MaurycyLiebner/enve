#include "pixmapeffect.h"
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include <QSqlError>
#include "Boxes/boxesgroup.h"
#include "fmt_filters.h"
#include "pointhelpers.h"

QDataStream & operator << (QDataStream & s, const PixmapEffect *ptr) {
    qulonglong ptrval(*reinterpret_cast<qulonglong *>(&ptr));
    return s << ptrval;
}

QDataStream & operator >> (QDataStream & s, PixmapEffect *& ptr) {
    qulonglong ptrval;
    s >> ptrval;
    ptr = *reinterpret_cast<PixmapEffect **>(&ptrval);
    return s;
}

PixmapEffect::PixmapEffect(const PixmapEffectType &type) :
    ComplexAnimator() {
    mType = type;
}

void PixmapEffect::prp_startDragging() {
    QMimeData *mimeData = new QMimeData;

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << (PixmapEffect*)this;

    mimeData->setData("effect", itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec();
}

int PixmapEffect::prp_saveToSql(QSqlQuery *query,
                                const int &boundingBoxSqlId) {
    if(!query->exec(
        QString("INSERT INTO pixmapeffect (boundingboxid, type) "
                "VALUES (%1, %2)").
                arg(boundingBoxSqlId).
                arg(mType) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    return query->lastInsertId().toInt();
}

BlurEffect::BlurEffect(qreal radius) : PixmapEffect(EFFECT_BLUR) {
    prp_setName("blur");
    mBlurRadius->qra_setCurrentValue(radius);
    mBlurRadius->prp_setName("radius");
    mBlurRadius->qra_setValueRange(0., 1000.);

    mHighQuality->setValue(false);
    mHighQuality->prp_setName("high quality");

    ca_addChildAnimator(mHighQuality.data());
    ca_addChildAnimator(mBlurRadius.data());
}

void BlurEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                          const fmt_filters::image &img,
                                          const qreal &scale) {
    qreal radius = blurRadius * scale;
    if(highQuality) {
        if(hasKeys) {
            fmt_filters::anim_fast_blur(img, radius*0.5);
            fmt_filters::anim_fast_blur(img, radius*0.5);
        } else {
            fmt_filters::fast_blur(img, radius*0.5);
            fmt_filters::fast_blur(img, radius*0.5);
        }
    } else {
        if(hasKeys) {
            fmt_filters::anim_fast_blur(img, radius*0.8);
        } else {
            fmt_filters::fast_blur(img, radius*0.8);
        }
    }
}

void applyBlur(const fmt_filters::image &img,
               const qreal &scale,
               const qreal &blurRadius,
               const bool &highQuality,
               const bool &hasKeys) {
    qreal radius = blurRadius*scale;

    if(highQuality) {
        if(hasKeys) {
            fmt_filters::anim_fast_blur(img, radius*0.5);
            fmt_filters::anim_fast_blur(img, radius*0.5);
        } else {
            fmt_filters::fast_blur(img, radius*0.5);
            fmt_filters::fast_blur(img, radius*0.5);
        }
    } else {
        if(hasKeys) {
            fmt_filters::anim_fast_blur(img, radius*0.8);
        } else {
            fmt_filters::fast_blur(img, radius*0.8);
        }
    }
}

void BlurEffect::applySk(const SkBitmap &imgPtr,
                         const fmt_filters::image &img,
                         qreal scale) {
    Q_UNUSED(imgPtr);
    applyBlur(img, scale,
              mBlurRadius->qra_getCurrentValue(),
              mHighQuality->getValue(),
              mBlurRadius->prp_hasKeys());
}

qreal BlurEffect::getMargin() {
    return mBlurRadius->qra_getCurrentValue();
}

qreal BlurEffect::getMarginAtRelFrame(const int &relFrame) {
    return mBlurRadius->qra_getValueAtRelFrame(relFrame);
}

int BlurEffect::prp_saveToSql(QSqlQuery *query,
                              const int &boundingBoxSqlId) {
    int pixmapEffectId = PixmapEffect::prp_saveToSql(query,
                                                     boundingBoxSqlId);
    int radiusId = mBlurRadius->prp_saveToSql(query);
    if(!query->exec(
        QString("INSERT INTO blureffect (pixmapeffectid, radiusid) "
                "VALUES (%1, %2)").
                arg(pixmapEffectId).
                arg(radiusId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }
    return pixmapEffectId;
}

void BlurEffect::prp_loadFromSql(const int &pixmapEffectId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM blureffect WHERE pixmapeffectid = " +
            QString::number(pixmapEffectId);
    if(query.exec(queryStr)) {
        query.next();
        mBlurRadius->prp_loadFromSql(query.value("radiusid").toInt() );
    } else {
        qDebug() << "Could not load blureffect with id " << pixmapEffectId;
    }
}

Property *BlurEffect::makeDuplicate() {
    BlurEffect *newBlur = new BlurEffect();
    makeDuplicate(newBlur);
    return newBlur;
}

void BlurEffect::makeDuplicate(Property *target) {
    BlurEffect *blurTarget = (BlurEffect*)target;

    blurTarget->duplicateBlurRadiusAnimatorFrom(mBlurRadius.data());
}

void BlurEffect::duplicateBlurRadiusAnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(mBlurRadius.data());
}

PixmapEffectRenderData *BlurEffect::getPixmapEffectRenderDataForRelFrame(
                                    const int &relFrame) {
    BlurEffectRenderData *renderData = new BlurEffectRenderData();
    renderData->blurRadius = mBlurRadius->getCurrentValueAtRelFrame(relFrame);
    renderData->hasKeys = mBlurRadius->prp_hasKeys();
    renderData->highQuality = mHighQuality->getValue();
    return renderData;
}

ShadowEffect::ShadowEffect(qreal radius) : PixmapEffect(EFFECT_SHADOW) {
    mBlurRadius->qra_setCurrentValue(radius);
    prp_setName("shadow");

    mHighQuality->setValue(false);
    mHighQuality->prp_setName("high quality");
    ca_addChildAnimator(mHighQuality.data());

    mBlurRadius->prp_setName("blur radius");
    mBlurRadius->qra_setValueRange(0., 1000.);
    ca_addChildAnimator(mBlurRadius.data());

    mTranslation->setCurrentPointValue(QPointF(0., 0.));
    mTranslation->prp_setName("translation");
    mTranslation->setValuesRange(-1000., 1000.);
    ca_addChildAnimator(mTranslation.data());

    mColor->qra_setCurrentValue(Qt::black);
    mColor->prp_setName("color");
    ca_addChildAnimator(mColor.data());

    mOpacity->qra_setCurrentValue(100.);
    mOpacity->prp_setName("opacity");
    mOpacity->qra_setValueRange(0., 100.);
    ca_addChildAnimator(mOpacity.data());
//    mScale.setCurrentValue(1.);
//    mScale.setName("scale");
//    mScale.blockPointer();
//    mScale.setValueRange(0., 10.);
//    addChildAnimator(&mScale);
}

int ShadowEffect::prp_saveToSql(QSqlQuery *query, const int &boundingBoxSqlId) {
    int pixmapEffectId = PixmapEffect::prp_saveToSql(query,
                                                     boundingBoxSqlId);
    int radiusId = mBlurRadius->prp_saveToSql(query);
    int colorId = mColor->prp_saveToSql(query);
    int opacityId = mOpacity->prp_saveToSql(query);
    int translationId = mTranslation->prp_saveToSql(query);
    if(!query->exec(
        QString("INSERT INTO shadoweffect (pixmapeffectid, blurradiusid, "
                "colorid, opacityid, translationid) "
                "VALUES (%1, %2, %3, %4, %5)").
                arg(pixmapEffectId).
                arg(radiusId).
                arg(colorId).
                arg(opacityId).
                arg(translationId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }
    return pixmapEffectId;
}

void ShadowEffect::prp_loadFromSql(const int &identifyingId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM shadoweffect WHERE pixmapeffectid = " +
            QString::number(identifyingId);
    if(query.exec(queryStr)) {
        query.next();
        mBlurRadius->prp_loadFromSql(query.value("blurradiusid").toInt() );
        mColor->prp_loadFromSql(query.value("colorid").toInt() );
        mOpacity->prp_loadFromSql(query.value("opacityid").toInt() );
        mTranslation->prp_loadFromSql(query.value("translationid").toInt() );
    } else {
        qDebug() << "Could not load shadowffect with id " << identifyingId;
    }
}

PixmapEffectRenderData *ShadowEffect::getPixmapEffectRenderDataForRelFrame(
                                    const int &relFrame) {
    ShadowEffectRenderData *renderData = new ShadowEffectRenderData();
    renderData->blurRadius = mBlurRadius->getCurrentValueAtRelFrame(relFrame);
    renderData->hasKeys = mBlurRadius->prp_hasKeys();
    renderData->highQuality = mHighQuality->getValue();
    renderData->color = mColor->getColorAtRelFrame(relFrame);
    renderData->translation = mTranslation->
            getCurrentPointValueAtRelFrame(relFrame);
    return renderData;
}

void applyShadow(const SkBitmap &imgPtr,
                 const qreal &scale,
                 const qreal &blurRadius,
                 const Color &currentColor,
                 const QPointF &trans,
                 const bool &hasKeys,
                 const bool &highQuality) {
    SkBitmap shadowBitmap;
    shadowBitmap.allocPixels(imgPtr.info());

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kDstIn);
    SkCanvas *shadowCanvas = new SkCanvas(shadowBitmap);
    shadowCanvas->clear(currentColor.getSkColor());
    shadowCanvas->drawBitmap(imgPtr, 0, 0, &paint);
    shadowCanvas->flush();
    delete shadowCanvas;

    SkPixmap shadowPixmap;
    shadowBitmap.peekPixels(&shadowPixmap);
    fmt_filters::image shadowImg((uint8_t*)shadowPixmap.writable_addr(),
                           shadowBitmap.width(), shadowBitmap.height());

    applyBlur(shadowImg, scale,
              blurRadius, highQuality,
              hasKeys);

    SkCanvas *dstCanvas = new SkCanvas(imgPtr);
    paint.setBlendMode(SkBlendMode::kDstOver);
    dstCanvas->drawBitmap(shadowBitmap,
                          trans.x()*scale, trans.y()*scale,
                          &paint);
    dstCanvas->flush();
    delete dstCanvas;
}

void ShadowEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                            const fmt_filters::image &img,
                                            const qreal &scale) {
    applyShadow(imgPtr, scale,
                blurRadius,
                color,
                translation,
                hasKeys,
                highQuality);
}

void ShadowEffect::applySk(const SkBitmap &imgPtr,
                           const fmt_filters::image &img,
                           qreal scale) {
    Q_UNUSED(img);
    applyShadow(imgPtr, scale,
                mBlurRadius->qra_getCurrentValue(),
                mColor->getCurrentColor(),
                mTranslation->getCurrentPointValue(),
                mBlurRadius->prp_hasKeys(),
                mHighQuality->getValue());
}

qreal ShadowEffect::getMargin() {
    return mBlurRadius->qra_getCurrentValue() +
            pointToLen(mTranslation->getCurrentPointValue());
}

qreal ShadowEffect::getMarginAtRelFrame(const int &relFrame) {
    return mBlurRadius->qra_getValueAtRelFrame(relFrame) +
            pointToLen(mTranslation->getCurrentPointValueAtRelFrame(relFrame));
}

Property *ShadowEffect::makeDuplicate() {
    ShadowEffect *newShadow = new ShadowEffect();
    makeDuplicate(newShadow);
    return newShadow;
}

void ShadowEffect::makeDuplicate(Property *target) {
    ShadowEffect *shadowTarget = (ShadowEffect*)target;

    shadowTarget->duplicateTranslationAnimatorFrom(mTranslation.data());
    shadowTarget->duplicateBlurRadiusAnimatorFrom(mBlurRadius.data());
    shadowTarget->duplicateColorAnimatorFrom(mColor.data());
    shadowTarget->duplicateOpacityAnimatorFrom(mOpacity.data());
}

void ShadowEffect::duplicateTranslationAnimatorFrom(QPointFAnimator *source) {
    source->makeDuplicate(mTranslation.data());
}

void ShadowEffect::duplicateBlurRadiusAnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(mBlurRadius.data());
}

void ShadowEffect::duplicateColorAnimatorFrom(ColorAnimator *source) {
    source->makeDuplicate(mColor.data());
}

void ShadowEffect::duplicateOpacityAnimatorFrom(QrealAnimator *source) {
    source->makeDuplicate(mOpacity.data());
}

LinesEffect::LinesEffect(qreal linesWidth, qreal linesDistance) :
    PixmapEffect(EFFECT_LINES) {
    prp_setName("lines");

    mLinesWidth->qra_setValueRange(0., 100000.);
    mLinesWidth->qra_setCurrentValue(linesWidth);
    mLinesWidth->prp_setName("width");
    ca_addChildAnimator(mLinesWidth.data());

    mLinesDistance->qra_setValueRange(0., 100000.);
    mLinesDistance->qra_setCurrentValue(linesDistance);
    mLinesDistance->prp_setName("distance");
    ca_addChildAnimator(mLinesDistance.data());
}

int LinesEffect::prp_saveToSql(QSqlQuery *query,
                               const int &boundingBoxSqlId) {
    int pixmapEffectId = PixmapEffect::prp_saveToSql(query,
                                                     boundingBoxSqlId);
    int distId = mLinesDistance->prp_saveToSql(query);
    int widthId = mLinesWidth->prp_saveToSql(query);

    if(!query->exec(
        QString("INSERT INTO lineseffect (pixmapeffectid, distanceid, "
                "widthid) "
                "VALUES (%1, %2, %3)").
                arg(pixmapEffectId).
                arg(distId).
                arg(widthId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }
    return pixmapEffectId;
}

void LinesEffect::prp_loadFromSql(const int &identifyingId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM lineseffect WHERE pixmapeffectid = " +
            QString::number(identifyingId);
    if(query.exec(queryStr)) {
        query.next();
        mLinesDistance->prp_loadFromSql(query.value("distanceid").toInt() );
        mLinesWidth->prp_loadFromSql(query.value("widthid").toInt() );
    } else {
        qDebug() << "Could not load lineseffect with id " << identifyingId;
    }
}

Property *LinesEffect::makeDuplicate() {
    LinesEffect *linesTarget = new LinesEffect();
    makeDuplicate(linesTarget);
    return linesTarget;
}

void LinesEffect::makeDuplicate(Property *target) {
    LinesEffect *linesTarget = (LinesEffect*)target;

    linesTarget->duplicateDistanceAnimatorFrom(mLinesDistance.data());
    linesTarget->duplicateWidthAnimatorFrom(mLinesWidth.data());
}

void LinesEffect::duplicateDistanceAnimatorFrom(QrealAnimator *source) {
    source->makeDuplicate(mLinesDistance.data());
}

void LinesEffect::duplicateWidthAnimatorFrom(QrealAnimator *source) {
    source->makeDuplicate(mLinesWidth.data());
}

void LinesEffect::apply(QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale) {
    Q_UNUSED(img);
    qreal linesWidth = mLinesWidth->qra_getCurrentValue()*scale;
    qreal linesDistance = mLinesDistance->qra_getCurrentValue()*scale;
    if((linesWidth < 0.1 && linesDistance < linesWidth) ||
            (linesDistance <= linesWidth*0.5)) return;

    QImage linesImg = QImage(imgPtr->size(),
                             QImage::Format_ARGB32_Premultiplied);
    linesImg.fill(Qt::transparent);

    int height = imgPtr->height();
    int width = imgPtr->width();

    QPainter linesImgP(&linesImg);
    linesImgP.setPen(QPen(Qt::white, linesWidth));
    if(linesWidth < 0.1 && linesDistance >= linesWidth) {

    } else {
        if(mVertical) {
            qreal lineX = (linesDistance + linesWidth)*0.5;
            while(lineX - linesWidth*0.5 < width) {
                linesImgP.drawLine(lineX, 0, lineX, height);
                lineX += linesDistance + linesWidth;
            }
        } else {
            qreal lineY = (linesDistance + linesWidth)*0.5;
            while(lineY - linesWidth*0.5 < height) {
                linesImgP.drawLine(0, lineY, width, lineY);
                lineY += linesDistance + linesWidth;
            }
        }
    }
    linesImgP.end();

    QPainter p(imgPtr);

    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawImage(0, 0, linesImg);

    p.end();
}

CirclesEffect::CirclesEffect(qreal circlesRadius,
                             qreal circlesDistance) :
    PixmapEffect(EFFECT_CIRCLES) {
    prp_setName("circles");

    mCirclesRadius->qra_setValueRange(0., 1000.);
    mCirclesRadius->qra_setCurrentValue(circlesRadius);
    mCirclesRadius->prp_setName("radius");
    ca_addChildAnimator(mCirclesRadius.data());

    mCirclesDistance->qra_setValueRange(-1000., 1000.);
    mCirclesDistance->qra_setCurrentValue(circlesDistance);
    mCirclesDistance->prp_setName("distance");
    ca_addChildAnimator(mCirclesDistance.data());
}

int CirclesEffect::prp_saveToSql(QSqlQuery *query,
                                 const int &boundingBoxSqlId) {
    int pixmapEffectId = PixmapEffect::prp_saveToSql(query,
                                                     boundingBoxSqlId);
    int distId = mCirclesDistance->prp_saveToSql(query);
    int radId = mCirclesRadius->prp_saveToSql(query);

    if(!query->exec(
        QString("INSERT INTO circleseffect (pixmapeffectid, distanceid, "
                "radiusid) "
                "VALUES (%1, %2, %3)").
                arg(pixmapEffectId).
                arg(distId).
                arg(radId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }
    return pixmapEffectId;
}

void CirclesEffect::prp_loadFromSql(const int &identifyingId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM circleseffect WHERE pixmapeffectid = " +
            QString::number(identifyingId);
    if(query.exec(queryStr)) {
        query.next();
        mCirclesDistance->prp_loadFromSql(query.value("distanceid").toInt() );
        mCirclesRadius->prp_loadFromSql(query.value("radiusid").toInt() );
    } else {
        qDebug() << "Could not load circleseffect with id " << identifyingId;
    }
}

Property *CirclesEffect::makeDuplicate() {
    CirclesEffect *linesTarget = new CirclesEffect();
    makeDuplicate(linesTarget);
    return linesTarget;
}

void CirclesEffect::makeDuplicate(Property *target) {
    CirclesEffect *circlesTarget = (CirclesEffect*)target;

    circlesTarget->duplicateDistanceAnimatorFrom(mCirclesDistance.data());
    circlesTarget->duplicateRadiusAnimatorFrom(mCirclesRadius.data());
}

void CirclesEffect::duplicateDistanceAnimatorFrom(QrealAnimator *source) {
    source->makeDuplicate(mCirclesDistance.data());
}

void CirclesEffect::duplicateRadiusAnimatorFrom(QrealAnimator *source) {
    source->makeDuplicate(mCirclesRadius.data());
}

#include "Boxes/boundingbox.h"
void CirclesEffect::apply(QImage *imgPtr,
                          const fmt_filters::image &img,
                          qreal scale) {
    Q_UNUSED(img);
    qreal radius = mCirclesRadius->qra_getCurrentValue()*scale;
    qreal distance = mCirclesDistance->qra_getCurrentValue()*scale;
    if((radius < 0.1 && distance < radius) || (distance <= -0.6*radius)) return;

    QImage circlesImg = QImage(imgPtr->size(),
                               QImage::Format_ARGB32_Premultiplied);
    circlesImg.fill(Qt::transparent);

    int height = imgPtr->height();
    int width = imgPtr->width();

    QPainter circlesImgP(&circlesImg);
    circlesImgP.setRenderHint(QPainter::Antialiasing);
    circlesImgP.setPen(Qt::NoPen);
    circlesImgP.setBrush(Qt::white);

    if(radius < 0.1 && distance >= radius) {

    } else {
        //QPointF topleft = target->getRelBoundingRect().topLeft();
        qreal circleX = radius + distance*0.5;// - fmod(topleft.x(),
                                              //        2*radius + distance);
        qreal circleY = radius + distance*0.5;// - fmod(topleft.y(),
                                              //        2*radius + distance);
        while(circleY - radius < height) {
            while(circleX - radius < width) {
                circlesImgP.drawEllipse(QPointF(circleX, circleY),
                                        radius, radius);
                circleX += 2*radius + distance;
            }
            circleY += 2*radius + distance;
            circleX = radius + distance*0.5;// - fmod(topleft.x(),
                                           //         2*radius + distance);
        }
    }

    circlesImgP.end();

    QPainter p(imgPtr);

    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawImage(0, 0, circlesImg);

    p.end();
}

SwirlEffect::SwirlEffect(qreal degrees) :
    PixmapEffect(EFFECT_SWIRL) {
    prp_setName("swirl");

    mDegreesAnimator->qra_setValueRange(-3600., 3600.);
    mDegreesAnimator->qra_setCurrentValue(degrees);
    mDegreesAnimator->prp_setName("degrees");
    ca_addChildAnimator(mDegreesAnimator.data());
}

int SwirlEffect::prp_saveToSql(QSqlQuery *query, const int &boundingBoxSqlId) {
    int pixmapEffectId = PixmapEffect::prp_saveToSql(query,
                                                     boundingBoxSqlId);
    int degId = mDegreesAnimator->prp_saveToSql(query);

    if(!query->exec(
        QString("INSERT INTO swirleffect (pixmapeffectid, "
                "degreesid) "
                "VALUES (%1, %2)").
                arg(pixmapEffectId).
                arg(degId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }
    return pixmapEffectId;
}

void SwirlEffect::prp_loadFromSql(const int &identifyingId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM swirleffect WHERE pixmapeffectid = " +
            QString::number(identifyingId);
    if(query.exec(queryStr)) {
        query.next();
        mDegreesAnimator->prp_loadFromSql(query.value("degreesid").toInt() );
    } else {
        qDebug() << "Could not load swirleffect with id " << identifyingId;
    }
}

Property *SwirlEffect::makeDuplicate() {
    SwirlEffect *linesTarget = new SwirlEffect();
    makeDuplicate(linesTarget);
    return linesTarget;
}

void SwirlEffect::makeDuplicate(Property *target) {
    SwirlEffect *swirlTarget = (SwirlEffect*)target;

    swirlTarget->duplicateDegreesAnimatorFrom(mDegreesAnimator.data());
}

void SwirlEffect::duplicateDegreesAnimatorFrom(QrealAnimator *source) {
    source->makeDuplicate(mDegreesAnimator.data());
}

void SwirlEffect::apply(QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    fmt_filters::swirl(img,
                       mDegreesAnimator->qra_getCurrentValue(),
                       fmt_filters::rgba(0, 0, 0, 0));
}

OilEffect::OilEffect(qreal radius) : PixmapEffect(EFFECT_OIL) {
    prp_setName("oil");

    mRadiusAnimator->qra_setValueRange(1., 5.);
    mRadiusAnimator->qra_setCurrentValue(radius);
    mRadiusAnimator->prp_setName("radius");
    ca_addChildAnimator(mRadiusAnimator.data());
}

int OilEffect::prp_saveToSql(QSqlQuery *query,
                             const int &boundingBoxSqlId) {
    int pixmapEffectId = PixmapEffect::prp_saveToSql(query,
                                                     boundingBoxSqlId);
    int radId = mRadiusAnimator->prp_saveToSql(query);

    if(!query->exec(
        QString("INSERT INTO oileffect (pixmapeffectid, "
                "radiusid) "
                "VALUES (%1, %2)").
                arg(pixmapEffectId).
                arg(radId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }
    return pixmapEffectId;
}

void OilEffect::prp_loadFromSql(const int &identifyingId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM oileffect WHERE pixmapeffectid = " +
            QString::number(identifyingId);
    if(query.exec(queryStr)) {
        query.next();
        mRadiusAnimator->prp_loadFromSql(query.value("radiusid").toInt() );
    } else {
        qDebug() << "Could not load oileffect with id " << identifyingId;
    }
}

Property *OilEffect::makeDuplicate() {
    OilEffect *oilTarget = new OilEffect();
    makeDuplicate(oilTarget);
    return oilTarget;
}

void OilEffect::makeDuplicate(Property *target) {
    OilEffect *oilTarget = (OilEffect*)target;

    oilTarget->duplicateRadiusAnimatorFrom(mRadiusAnimator.data());
}

void OilEffect::duplicateRadiusAnimatorFrom(QrealAnimator *source) {
    source->makeDuplicate(mRadiusAnimator.data());
}

void OilEffect::apply(QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    fmt_filters::oil(img,
                     mRadiusAnimator->qra_getCurrentValue());
}

ImplodeEffect::ImplodeEffect(qreal radius) :
    PixmapEffect(EFFECT_IMPLODE) {
    prp_setName("implode");

    mFactorAnimator->qra_setValueRange(0., 100.);
    mFactorAnimator->qra_setCurrentValue(radius);
    mFactorAnimator->prp_setName("factor");
    ca_addChildAnimator(mFactorAnimator.data());
}

int ImplodeEffect::prp_saveToSql(QSqlQuery *query,
                                 const int &boundingBoxSqlId) {
    int pixmapEffectId = PixmapEffect::prp_saveToSql(query,
                                                     boundingBoxSqlId);
    int facId = mFactorAnimator->prp_saveToSql(query);

    if(!query->exec(
        QString("INSERT INTO implodeeffect (pixmapeffectid, "
                "factorid) "
                "VALUES (%1, %2)").
                arg(pixmapEffectId).
                arg(facId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }
    return pixmapEffectId;
}

void ImplodeEffect::prp_loadFromSql(const int &identifyingId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM implodeeffect WHERE pixmapeffectid = " +
            QString::number(identifyingId);
    if(query.exec(queryStr)) {
        query.next();
        mFactorAnimator->prp_loadFromSql(query.value("factorid").toInt() );
    } else {
        qDebug() << "Could not load implodeeffect with id " << identifyingId;
    }
}

Property *ImplodeEffect::makeDuplicate() {
    ImplodeEffect *implodeTarget = new ImplodeEffect();
    makeDuplicate(implodeTarget);
    return implodeTarget;
}

void ImplodeEffect::makeDuplicate(Property *target) {
    ImplodeEffect *implodeTarget = (ImplodeEffect*)target;

    implodeTarget->duplicateFactorAnimatorFrom(mFactorAnimator.data());
}

void ImplodeEffect::duplicateFactorAnimatorFrom(QrealAnimator *source) {
    source->makeDuplicate(mFactorAnimator.data());
}

void ImplodeEffect::apply(QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    fmt_filters::implode(img,
                         mFactorAnimator->qra_getCurrentValue(),
                         fmt_filters::rgba(0, 0, 0, 0));
}

DesaturateEffect::DesaturateEffect(qreal radius) :
    PixmapEffect(EFFECT_DESATURATE) {
    prp_setName("desaturate");

    mInfluenceAnimator->qra_setValueRange(0., 1.);
    mInfluenceAnimator->qra_setCurrentValue(radius);
    mInfluenceAnimator->prp_setName("factor");
    ca_addChildAnimator(mInfluenceAnimator.data());
}

int DesaturateEffect::prp_saveToSql(QSqlQuery *query,
                                    const int &boundingBoxSqlId) {
    int pixmapEffectId = PixmapEffect::prp_saveToSql(query,
                                                     boundingBoxSqlId);
    int infId = mInfluenceAnimator->prp_saveToSql(query);

    if(!query->exec(
        QString("INSERT INTO desaturateeffect (pixmapeffectid, "
                "influenceid) "
                "VALUES (%1, %2)").
                arg(pixmapEffectId).
                arg(infId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }
    return pixmapEffectId;
}

void DesaturateEffect::prp_loadFromSql(const int &identifyingId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM desaturateeffect WHERE pixmapeffectid = " +
            QString::number(identifyingId);
    if(query.exec(queryStr)) {
        query.next();
        mInfluenceAnimator->prp_loadFromSql(query.value("influenceid").toInt() );
    } else {
        qDebug() << "Could not load desaturateeffect with id " << identifyingId;
    }
}

Property *DesaturateEffect::makeDuplicate() {
    DesaturateEffect *desaturateTarget = new DesaturateEffect();
    makeDuplicate(desaturateTarget);
    return desaturateTarget;
}

void DesaturateEffect::makeDuplicate(Property *target) {
    DesaturateEffect *desaturateTarget = (DesaturateEffect*)target;

    desaturateTarget->duplicateInfluenceAnimatorFrom(mInfluenceAnimator.data());
}

void DesaturateEffect::duplicateInfluenceAnimatorFrom(QrealAnimator *source) {
    source->makeDuplicate(mInfluenceAnimator.data());
}

void DesaturateEffect::apply(QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    fmt_filters::desaturate(img,
                            mInfluenceAnimator->qra_getCurrentValue());
}

void DesaturateEffect::applySk(const SkBitmap &imgPtr,
                               const fmt_filters::image &img,
                               qreal scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    fmt_filters::desaturate(img,
                            mInfluenceAnimator->qra_getCurrentValue());
}



ColorizeEffect::ColorizeEffect() :
    PixmapEffect(EFFECT_COLORIZE) {
    prp_setName("colorize");

    mColorAnimator->prp_setName("color");
    ca_addChildAnimator(mColorAnimator.data());
}

int ColorizeEffect::prp_saveToSql(QSqlQuery *query,
                                    const int &boundingBoxSqlId) {
    int pixmapEffectId = PixmapEffect::prp_saveToSql(query,
                                                     boundingBoxSqlId);
    int infId = mColorAnimator->prp_saveToSql(query);

    if(!query->exec(
        QString("INSERT INTO colorizeeffect (pixmapeffectid, "
                "influenceid) "
                "VALUES (%1, %2)").
                arg(pixmapEffectId).
                arg(infId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }
    return pixmapEffectId;
}

void ColorizeEffect::prp_loadFromSql(const int &identifyingId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM colorizeeffect WHERE pixmapeffectid = " +
            QString::number(identifyingId);
    if(query.exec(queryStr)) {
        query.next();
        mColorAnimator->prp_loadFromSql(query.value("influenceid").toInt() );
    } else {
        qDebug() << "Could not load colorizeeffect with id " << identifyingId;
    }
}

Property *ColorizeEffect::makeDuplicate() {
    ColorizeEffect *colorizeTarget = new ColorizeEffect();
    makeDuplicate(colorizeTarget);
    return colorizeTarget;
}

void ColorizeEffect::makeDuplicate(Property *target) {
    ColorizeEffect *colorizeTarget = (ColorizeEffect*)target;

    colorizeTarget->duplicateInfluenceAnimatorFrom(mColorAnimator.data());
}

void ColorizeEffect::duplicateInfluenceAnimatorFrom(ColorAnimator *source) {
    source->makeDuplicate(mColorAnimator.data());
}

void ColorizeEffect::apply(QImage *imgPtr,
                           const fmt_filters::image &img,
                           qreal scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    Color color = mColorAnimator->getCurrentColor();
    fmt_filters::colorize(img, color.gl_r,
                          color.gl_g,
                          color.gl_b,
                          color.gl_a);
}

void ColorizeEffect::applySk(const SkBitmap &imgPtr,
                             const fmt_filters::image &img,
                             qreal scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    Color color = mColorAnimator->getCurrentColor();
    fmt_filters::colorize(img, color.gl_r,
                          color.gl_g,
                          color.gl_b,
                          color.gl_a);
}
