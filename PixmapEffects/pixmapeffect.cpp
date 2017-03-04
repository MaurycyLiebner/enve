#include "pixmapeffect.h"
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include <QSqlError>

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

void PixmapEffect::startDragging() {
    QMimeData *mimeData = new QMimeData;

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << (PixmapEffect*)this;

    mimeData->setData("effect", itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec();
}

int PixmapEffect::saveToSql(QSqlQuery *query,
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
    mBlurRadius.setCurrentValue(radius);
    setName("blur");
    mBlurRadius.setName("radius");
    mBlurRadius.blockPointer();
    mBlurRadius.setValueRange(0., 1000.);
    addChildAnimator(&mBlurRadius);
}

void BlurEffect::apply(BoundingBox *target,
                       QImage *imgPtr,
                       const fmt_filters::image &img,
                       qreal scale,
                       bool highQuality) {
    Q_UNUSED(imgPtr);
    qreal radius = mBlurRadius.getCurrentValue()*scale;
    //fmt_filters::blur(img, radius, radius*0.3333);
    //return;
    if(highQuality) {
        //fmt_filters::fast_blur(img, radius*0.5);
        fmt_filters::fast_blur(img, radius*0.25);
        fmt_filters::fast_blur(img, radius*0.25);
        //fmt_filters::blur(img, radius, radius*0.3333);
    } else {
        fmt_filters::fast_blur(img, radius*0.25);
        fmt_filters::fast_blur(img, radius*0.25);
        //fmt_filters::fast_blur(img, radius*0.5);
    }
    imgPtr->save("/home/ailuropoda/test.png");
}

qreal BlurEffect::getMargin()
{
    return mBlurRadius.getCurrentValue();
}

#include <QSqlError>
void BlurEffect::saveToSql(QSqlQuery *query, const int &boundingBoxSqlId) {
    int pixmapEffectId = PixmapEffect::saveToSql(query,
                                                 boundingBoxSqlId,
                                                 EFFECT_BLUR);
    int radiusId = mBlurRadius.saveToSql(query);
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

QrealAnimator *BlurEffect::makeDuplicate() {
    BlurEffect *newBlur = new BlurEffect();
    makeDuplicate(newBlur);
    return newBlur;
}

void BlurEffect::makeDuplicate(QrealAnimator *target) {
    BlurEffect *blurTarget = (BlurEffect*)target;

    blurTarget->duplicateBlurRadiusAnimatorFrom(&mBlurRadius);
}

void BlurEffect::duplicateBlurRadiusAnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(&mBlurRadius);
}

ShadowEffect::ShadowEffect(qreal radius) {
    mBlurRadius.setCurrentValue(radius);
    setName("shadow");
    mBlurRadius.setName("blur radius");
    mBlurRadius.blockPointer();
    mBlurRadius.setValueRange(0., 1000.);
    addChildAnimator(&mBlurRadius);

    mTranslation.setCurrentValue(QPointF(0., 0.));
    mTranslation.setName("translation");
    mTranslation.blockPointer();
    mTranslation.setValueRange(-1000., 1000.);
    addChildAnimator(&mTranslation);

    mColor.setCurrentValue(Qt::black);
    mColor.setName("color");
    mColor.blockPointer();
    addChildAnimator(&mColor);

    mOpacity.setCurrentValue(100.);
    mOpacity.setName("opacity");
    mOpacity.blockPointer();
    mOpacity.setValueRange(0., 100.);
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
                         qreal scale,
                         bool highQuality) {
    Q_UNUSED(imgPtr);

    QImage shadowQImg = imgPtr->copy();
    fmt_filters::image shadowImg(shadowQImg.bits(),
                                 shadowQImg.width(),
                                 shadowQImg.height());

    QColor currentColor = mColor.getCurrentValue().qcol;

    fmt_filters::replaceColor(shadowImg,
                              currentColor.red(),
                              currentColor.green(),
                              currentColor.blue());

//    QPainter p0(&shadowQImg);
//    p0.setCompositionMode(QPainter::CompositionMode_Xor);
//    p0.fillRect(shadowQImg.rect(), Qt::black);
//    p0.end();


    qreal radius = mBlurRadius.getCurrentValue()*scale;
    if(highQuality) {
        //fmt_filters::blur(shadowImg, radius, radius*0.3333);
        fmt_filters::fast_blur(shadowImg, radius*0.25);
        fmt_filters::fast_blur(shadowImg, radius*0.25);
    } else {
        //fmt_filters::fast_blur(shadowImg, radius*0.5);
        fmt_filters::fast_blur(shadowImg, radius*0.25);
        fmt_filters::fast_blur(shadowImg, radius*0.25);
    }

    QPainter p(imgPtr);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    p.setOpacity(mOpacity.getCurrentValue()*0.01);
    p.drawImage(mTranslation.getCurrentValue()*scale, shadowQImg);
    p.end();
}

qreal ShadowEffect::getMargin() {
    return mBlurRadius.getCurrentValue() +
            pointToLen(mTranslation.getCurrentValue());
}

QrealAnimator *ShadowEffect::makeDuplicate() {
    ShadowEffect *newShadow = new ShadowEffect();
    makeDuplicate(newShadow);
    return newShadow;
}

void ShadowEffect::makeDuplicate(QrealAnimator *target) {
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
    setName("lines");

    mLinesWidth.setValueRange(0., 100000.);
    mLinesWidth.setCurrentValue(linesWidth);
    mLinesWidth.setName("width");
    mLinesWidth.blockPointer();
    addChildAnimator(&mLinesWidth);

    mLinesDistance.setValueRange(0., 100000.);
    mLinesDistance.setCurrentValue(linesDistance);
    mLinesDistance.setName("distance");
    mLinesDistance.blockPointer();
    addChildAnimator(&mLinesDistance);
}

void LinesEffect::apply(BoundingBox *target,
                        QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale,
                        bool highQuality)
{
    qreal linesWidth = mLinesWidth.getCurrentValue()*scale;
    qreal linesDistance = mLinesDistance.getCurrentValue()*scale;
    if((linesWidth < 0.1 && linesDistance < linesWidth) ||
            (linesDistance <= linesWidth*0.5)) return;

    QImage linesImg = QImage(imgPtr->size(),
                             QImage::Format_ARGB32);
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
    setName("circles");

    mCirclesRadius.setValueRange(0., 1000.);
    mCirclesRadius.setCurrentValue(circlesRadius);
    mCirclesRadius.setName("radius");
    mCirclesRadius.blockPointer();
    addChildAnimator(&mCirclesRadius);

    mCirclesDistance.setValueRange(-1000., 1000.);
    mCirclesDistance.setCurrentValue(circlesDistance);
    mCirclesDistance.setName("distance");
    mCirclesDistance.blockPointer();
    addChildAnimator(&mCirclesDistance);
}
#include "Boxes/boundingbox.h"
void CirclesEffect::apply(BoundingBox *target,
                          QImage *imgPtr,
                          const fmt_filters::image &img,
                          qreal scale,
                          bool highQuality)
{
    qreal radius = mCirclesRadius.getCurrentValue()*scale;
    qreal distance = mCirclesDistance.getCurrentValue()*scale;
    if((radius < 0.1 && distance < radius) || (distance <= -0.6*radius)) return;

    QImage circlesImg = QImage(imgPtr->size(),
                               QImage::Format_ARGB32);
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
    setName("swirl");

    mDegreesAnimator.setValueRange(-3600., 3600.);
    mDegreesAnimator.setCurrentValue(degrees);
    mDegreesAnimator.setName("degrees");
    mDegreesAnimator.blockPointer();
    addChildAnimator(&mDegreesAnimator);
}

void SwirlEffect::apply(BoundingBox *target,
                        QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale,
                        bool highQuality) {
    Q_UNUSED(imgPtr);
    fmt_filters::swirl(img,
                       mDegreesAnimator.getCurrentValue(),
                       fmt_filters::rgba(0, 0, 0, 0));
}

OilEffect::OilEffect(qreal radius) {
    setName("oil");

    mRadiusAnimator.setValueRange(1., 5.);
    mRadiusAnimator.setCurrentValue(radius);
    mRadiusAnimator.setName("radius");
    mRadiusAnimator.blockPointer();
    addChildAnimator(&mRadiusAnimator);
}

void OilEffect::apply(BoundingBox *target,
                        QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale,
                        bool highQuality) {
    Q_UNUSED(imgPtr);
    fmt_filters::oil(img,
                     mRadiusAnimator.getCurrentValue());
}

ImplodeEffect::ImplodeEffect(qreal radius) {
    setName("implode");

    mFactorAnimator.setValueRange(0., 100.);
    mFactorAnimator.setCurrentValue(radius);
    mFactorAnimator.setName("factor");
    mFactorAnimator.blockPointer();
    addChildAnimator(&mFactorAnimator);
}

void ImplodeEffect::apply(BoundingBox *target,
                        QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale,
                        bool highQuality) {
    Q_UNUSED(imgPtr);
    fmt_filters::implode(img,
                         mFactorAnimator.getCurrentValue(),
                         fmt_filters::rgba(0, 0, 0, 0));
}


DesaturateEffect::DesaturateEffect(qreal radius) {
    setName("desaturate");

    mInfluenceAnimator.setValueRange(0., 1.);
    mInfluenceAnimator.setCurrentValue(radius);
    mInfluenceAnimator.setName("factor");
    mInfluenceAnimator.blockPointer();
    addChildAnimator(&mInfluenceAnimator);
}

void DesaturateEffect::apply(BoundingBox *target,
                        QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale,
                        bool highQuality) {
    Q_UNUSED(imgPtr);
    fmt_filters::desaturate(img,
                            mInfluenceAnimator.getCurrentValue());
}
