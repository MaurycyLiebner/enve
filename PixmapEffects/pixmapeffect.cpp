#include "pixmapeffect.h"
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include <QSqlError>
#include "Boxes/boxesgroup.h"

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

PixmapEffect::PixmapEffect() : ComplexAnimator()
{

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
                            const int &boundingBoxSqlId,
                            const PixmapEffectType &type) {
    if(!query->exec(
        QString("INSERT INTO pixmapeffect (boundingboxid, type) "
                "VALUES (%1, %2)").
                arg(boundingBoxSqlId).
                arg(type) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    return query->lastInsertId().toInt();
}

PixmapEffect *PixmapEffect::loadFromSql(int pixmapEffectId,
                                        PixmapEffectType typeT) {
    if(typeT == EFFECT_BLUR) {
        BlurEffect *blurEffect = new BlurEffect();
        blurEffect->loadBlurEffectFromSql(pixmapEffectId);
        return blurEffect;
    }
}

BlurEffect::BlurEffect(qreal radius) {
    mBlurRadius.qra_setCurrentValue(radius);
    prp_setName("blur");
    mBlurRadius.prp_setName("radius");
    mBlurRadius.blockPointer();
    mBlurRadius.qra_setValueRange(0., 1000.);
    addChildAnimator(&mBlurRadius);
}

void BlurEffect::apply(BoundingBox *target,
                       QImage *imgPtr,
                       const fmt_filters::image &img,
                       qreal scale) {
    Q_UNUSED(imgPtr);
    qreal radius = mBlurRadius.qra_getCurrentValue()*scale;
    //fmt_filters::blur(img, radius, radius*0.3333);
    //return;
    //fmt_filters::fast_blur(img, radius*0.5);
    fmt_filters::fast_blur(img, radius*0.25);
    fmt_filters::fast_blur(img, radius*0.25);
    //fmt_filters::blur(img, radius, radius*0.3333);
}

qreal BlurEffect::getMargin()
{
    return mBlurRadius.qra_getCurrentValue();
}

#include <QSqlError>
void BlurEffect::prp_saveToSql(QSqlQuery *query, const int &boundingBoxSqlId) {
    int pixmapEffectId = PixmapEffect::prp_saveToSql(query,
                                                 boundingBoxSqlId,
                                                 EFFECT_BLUR);
    int radiusId = mBlurRadius.prp_saveToSql(query);
    if(!query->exec(
        QString("INSERT INTO blureffect (pixmapeffectid, radiusid) "
                "VALUES (%1, %2)").
                arg(pixmapEffectId).
                arg(radiusId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }
}

void BlurEffect::loadBlurEffectFromSql(int pixmapEffectId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM blureffect WHERE pixmapeffectid = " +
            QString::number(pixmapEffectId);
    if(query.exec(queryStr)) {
        query.next();
        mBlurRadius.loadFromSql(query.value("radiusid").toInt() );
    } else {
        qDebug() << "Could not load blureffect with id " << pixmapEffectId;
    }
}

Animator *BlurEffect::makeDuplicate() {
    BlurEffect *newBlur = new BlurEffect();
    makeDuplicate(newBlur);
    return newBlur;
}

void BlurEffect::makeDuplicate(Animator *target) {
    BlurEffect *blurTarget = (BlurEffect*)target;

    blurTarget->duplicateBlurRadiusAnimatorFrom(&mBlurRadius);
}

void BlurEffect::duplicateBlurRadiusAnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(&mBlurRadius);
}

ShadowEffect::ShadowEffect(qreal radius) {
    mBlurRadius.qra_setCurrentValue(radius);
    prp_setName("shadow");
    mBlurRadius.prp_setName("blur radius");
    mBlurRadius.blockPointer();
    mBlurRadius.qra_setValueRange(0., 1000.);
    addChildAnimator(&mBlurRadius);

    mTranslation.qra_setCurrentValue(QPointF(0., 0.));
    mTranslation.prp_setName("translation");
    mTranslation.blockPointer();
    mTranslation.qra_setValueRange(-1000., 1000.);
    addChildAnimator(&mTranslation);

    mColor.qra_setCurrentValue(Qt::black);
    mColor.prp_setName("color");
    mColor.blockPointer();
    addChildAnimator(&mColor);

    mOpacity.qra_setCurrentValue(100.);
    mOpacity.prp_setName("opacity");
    mOpacity.blockPointer();
    mOpacity.qra_setValueRange(0., 100.);
    addChildAnimator(&mOpacity);
//    mScale.setCurrentValue(1.);
//    mScale.setName("scale");
//    mScale.blockPointer();
//    mScale.setValueRange(0., 10.);
//    addChildAnimator(&mScale);
}

void ShadowEffect::apply(BoundingBox *target,
                         QImage *imgPtr,
                         const fmt_filters::image &img,
                         qreal scale) {
    Q_UNUSED(imgPtr);

    QImage shadowQImg = imgPtr->copy();
    fmt_filters::image shadowImg(shadowQImg.bits(),
                                 shadowQImg.width(),
                                 shadowQImg.height());

    QColor currentColor = mColor.qra_getCurrentValue().qcol;

    fmt_filters::replaceColor(shadowImg,
                              currentColor.red(),
                              currentColor.green(),
                              currentColor.blue());

//    QPainter p0(&shadowQImg);
//    p0.setCompositionMode(QPainter::CompositionMode_Xor);
//    p0.fillRect(shadowQImg.rect(), Qt::black);
//    p0.end();


    qreal radius = mBlurRadius.qra_getCurrentValue()*scale;
    //fmt_filters::blur(shadowImg, radius, radius*0.3333);
    fmt_filters::fast_blur(shadowImg, radius*0.25);
    fmt_filters::fast_blur(shadowImg, radius*0.25);
    //fmt_filters::fast_blur(shadowImg, radius*0.5);

    QPainter p(imgPtr);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    p.setOpacity(mOpacity.qra_getCurrentValue()*0.01);
    p.drawImage(mTranslation.qra_getCurrentValue()*scale, shadowQImg);
    p.end();
}

qreal ShadowEffect::getMargin() {
    return mBlurRadius.qra_getCurrentValue() +
            pointToLen(mTranslation.qra_getCurrentValue());
}

Animator *ShadowEffect::makeDuplicate() {
    ShadowEffect *newShadow = new ShadowEffect();
    makeDuplicate(newShadow);
    return newShadow;
}

void ShadowEffect::makeDuplicate(Animator *target) {
    ShadowEffect *shadowTarget = (ShadowEffect*)target;

    shadowTarget->duplicateTranslationAnimatorFrom(&mTranslation);
    shadowTarget->duplicateBlurRadiusAnimatorFrom(&mBlurRadius);
    shadowTarget->duplicateColorAnimatorFrom(&mColor);
    shadowTarget->duplicateOpacityAnimatorFrom(&mOpacity);
}

void ShadowEffect::duplicateTranslationAnimatorFrom(QPointFAnimator *source) {
    source->makeDuplicate(&mTranslation);
}

void ShadowEffect::duplicateBlurRadiusAnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(&mBlurRadius);
}

void ShadowEffect::duplicateColorAnimatorFrom(ColorAnimator *source) {
    source->makeDuplicate(&mColor);
}

void ShadowEffect::duplicateOpacityAnimatorFrom(QrealAnimator *source) {
    source->makeDuplicate(&mOpacity);
}

LinesEffect::LinesEffect(qreal linesWidth, qreal linesDistance) : PixmapEffect() {
    prp_setName("lines");

    mLinesWidth.qra_setValueRange(0., 100000.);
    mLinesWidth.qra_setCurrentValue(linesWidth);
    mLinesWidth.prp_setName("width");
    mLinesWidth.blockPointer();
    addChildAnimator(&mLinesWidth);

    mLinesDistance.qra_setValueRange(0., 100000.);
    mLinesDistance.qra_setCurrentValue(linesDistance);
    mLinesDistance.prp_setName("distance");
    mLinesDistance.blockPointer();
    addChildAnimator(&mLinesDistance);
}

void LinesEffect::apply(BoundingBox *target,
                        QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale)
{
    qreal linesWidth = mLinesWidth.qra_getCurrentValue()*scale;
    qreal linesDistance = mLinesDistance.qra_getCurrentValue()*scale;
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
    PixmapEffect() {
    prp_setName("circles");

    mCirclesRadius.qra_setValueRange(0., 1000.);
    mCirclesRadius.qra_setCurrentValue(circlesRadius);
    mCirclesRadius.prp_setName("radius");
    mCirclesRadius.blockPointer();
    addChildAnimator(&mCirclesRadius);

    mCirclesDistance.qra_setValueRange(-1000., 1000.);
    mCirclesDistance.qra_setCurrentValue(circlesDistance);
    mCirclesDistance.prp_setName("distance");
    mCirclesDistance.blockPointer();
    addChildAnimator(&mCirclesDistance);
}
#include "Boxes/boundingbox.h"
void CirclesEffect::apply(BoundingBox *target,
                          QImage *imgPtr,
                          const fmt_filters::image &img,
                          qreal scale)
{
    qreal radius = mCirclesRadius.qra_getCurrentValue()*scale;
    qreal distance = mCirclesDistance.qra_getCurrentValue()*scale;
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

SwirlEffect::SwirlEffect(qreal degrees) {
    prp_setName("swirl");

    mDegreesAnimator.qra_setValueRange(-3600., 3600.);
    mDegreesAnimator.qra_setCurrentValue(degrees);
    mDegreesAnimator.prp_setName("degrees");
    mDegreesAnimator.blockPointer();
    addChildAnimator(&mDegreesAnimator);
}

void SwirlEffect::apply(BoundingBox *target,
                        QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale) {
    Q_UNUSED(imgPtr);
    fmt_filters::swirl(img,
                       mDegreesAnimator.qra_getCurrentValue(),
                       fmt_filters::rgba(0, 0, 0, 0));
}

OilEffect::OilEffect(qreal radius) {
    prp_setName("oil");

    mRadiusAnimator.qra_setValueRange(1., 5.);
    mRadiusAnimator.qra_setCurrentValue(radius);
    mRadiusAnimator.prp_setName("radius");
    mRadiusAnimator.blockPointer();
    addChildAnimator(&mRadiusAnimator);
}

void OilEffect::apply(BoundingBox *target,
                        QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale) {
    Q_UNUSED(imgPtr);
    fmt_filters::oil(img,
                     mRadiusAnimator.qra_getCurrentValue());
}

ImplodeEffect::ImplodeEffect(qreal radius) {
    prp_setName("implode");

    mFactorAnimator.qra_setValueRange(0., 100.);
    mFactorAnimator.qra_setCurrentValue(radius);
    mFactorAnimator.prp_setName("factor");
    mFactorAnimator.blockPointer();
    addChildAnimator(&mFactorAnimator);
}

void ImplodeEffect::apply(BoundingBox *target,
                        QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale) {
    Q_UNUSED(imgPtr);
    fmt_filters::implode(img,
                         mFactorAnimator.qra_getCurrentValue(),
                         fmt_filters::rgba(0, 0, 0, 0));
}


DesaturateEffect::DesaturateEffect(qreal radius) {
    prp_setName("desaturate");

    mInfluenceAnimator.qra_setValueRange(0., 1.);
    mInfluenceAnimator.qra_setCurrentValue(radius);
    mInfluenceAnimator.prp_setName("factor");
    mInfluenceAnimator.blockPointer();
    addChildAnimator(&mInfluenceAnimator);
}

void DesaturateEffect::apply(BoundingBox *target,
                        QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale) {
    Q_UNUSED(imgPtr);
    fmt_filters::desaturate(img,
                            mInfluenceAnimator.qra_getCurrentValue());
}

AlphaMatteEffect::AlphaMatteEffect(BoundingBox *parentBox) {
    prp_setName("alpha matte");

    mInfluenceAnimator.qra_setValueRange(0., 1.);
    mInfluenceAnimator.qra_setCurrentValue(1.);
    mInfluenceAnimator.setPrefferedValueStep(0.05);
    mInfluenceAnimator.prp_setName("influence");
    mInfluenceAnimator.blockPointer();
    addChildAnimator(&mInfluenceAnimator);

    mBoxTarget.prp_setName("target");
    mBoxTarget.blockPointer();
    mBoxTarget.setParentBox(parentBox);
    addChildAnimator(&mBoxTarget);

    mInvertedProperty.prp_setName("invert");
    mInvertedProperty.blockPointer();
    addChildAnimator(&mInvertedProperty);
}

void AlphaMatteEffect::apply(BoundingBox *target,
                             QImage *imgPtr,
                             const fmt_filters::image &img,
                             qreal scale) {
    BoundingBox *boxTarget = mBoxTarget.getTarget();
    if(boxTarget) {
        qreal influence = mInfluenceAnimator.qra_getCurrentValue();
        QRectF targetRect = target->getUpdateRenderRect();

        QImage imgTmp = QImage(imgPtr->size(),
                               QImage::Format_ARGB32_Premultiplied);
        imgTmp.fill(Qt::transparent);
        QPainter p(&imgTmp);

        p.translate(-targetRect.topLeft());
        p.setTransform(
                    QTransform(target->getUpdatePaintTransform().inverted()),
                    true);

        boxTarget->drawUpdatePixmapForEffect(&p);
        p.end();

        QPainter p2(imgPtr);

        p2.setOpacity(influence);
        if(mInvertedProperty.getValue()) {
            p2.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        } else {
            p2.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        }

        p2.drawImage(0, 0, imgTmp);

        p2.end();
    }
}
