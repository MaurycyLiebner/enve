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
    mBlurRadius->qra_setCurrentValue(radius);
    prp_setName("blur");
    mBlurRadius->prp_setName("radius");
    mBlurRadius->qra_setValueRange(0., 1000.);
    ca_addChildAnimator(mBlurRadius.data());
}

void BlurEffect::apply(BoundingBox *target,
                       QImage *imgPtr,
                       const fmt_filters::image &img,
                       qreal scale) {
    Q_UNUSED(imgPtr);
    qreal radius = mBlurRadius->qra_getCurrentValue()*scale;
    fmt_filters::blur(img, radius, radius*0.3333);
    return;
    //fmt_filters::fast_blur(img, radius*0.5);
    fmt_filters::fast_blur(img, radius*0.25);
    fmt_filters::fast_blur(img, radius*0.25);
    //fmt_filters::blur(img, radius, radius*0.3333);
}

qreal BlurEffect::getMargin()
{
    return mBlurRadius->qra_getCurrentValue();
}

#include <QSqlError>
int BlurEffect::prp_saveToSql(QSqlQuery *query, const int &boundingBoxSqlId) {
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

void BlurEffect::prp_loadFromSql(const int &identifyingId) {
    loadBlurEffectFromSql(identifyingId);
}

void BlurEffect::loadBlurEffectFromSql(const int &pixmapEffectId) {
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

Property *BlurEffect::prp_makeDuplicate() {
    BlurEffect *newBlur = new BlurEffect();
    prp_makeDuplicate(newBlur);
    return newBlur;
}

void BlurEffect::prp_makeDuplicate(Property *target) {
    BlurEffect *blurTarget = (BlurEffect*)target;

    blurTarget->duplicateBlurRadiusAnimatorFrom(mBlurRadius.data());
}

void BlurEffect::duplicateBlurRadiusAnimatorFrom(
        QrealAnimator *source) {
    source->prp_makeDuplicate(mBlurRadius.data());
}

ShadowEffect::ShadowEffect(qreal radius) : PixmapEffect(EFFECT_SHADOW) {
    mBlurRadius->qra_setCurrentValue(radius);
    prp_setName("shadow");
    mBlurRadius->prp_setName("blur radius");
    mBlurRadius->qra_setValueRange(0., 1000.);
    ca_addChildAnimator(mBlurRadius.data());

    mTranslation->qra_setCurrentValue(QPointF(0., 0.));
    mTranslation->prp_setName("translation");
    mTranslation->qra_setValueRange(-1000., 1000.);
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

void ShadowEffect::apply(BoundingBox *target,
                         QImage *imgPtr,
                         const fmt_filters::image &img,
                         qreal scale) {
    Q_UNUSED(imgPtr);

    QImage shadowQImg = imgPtr->copy();
    fmt_filters::image shadowImg(shadowQImg.bits(),
                                 shadowQImg.width(),
                                 shadowQImg.height());

    QColor currentColor = mColor->qra_getCurrentValue().qcol;

    fmt_filters::replaceColor(shadowImg,
                              currentColor.red(),
                              currentColor.green(),
                              currentColor.blue());

//    QPainter p0(&shadowQImg);
//    p0.setCompositionMode(QPainter::CompositionMode_Xor);
//    p0.fillRect(shadowQImg.rect(), Qt::black);
//    p0.end();


    qreal radius = mBlurRadius->qra_getCurrentValue()*scale;
    //fmt_filters::blur(shadowImg, radius, radius*0.3333);
    fmt_filters::fast_blur(shadowImg, radius*0.25);
    fmt_filters::fast_blur(shadowImg, radius*0.25);
    //fmt_filters::fast_blur(shadowImg, radius*0.5);

    QPainter p(imgPtr);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    p.setOpacity(mOpacity->qra_getCurrentValue()*0.01);
    p.drawImage(mTranslation->qra_getCurrentValue()*scale, shadowQImg);
    p.end();
}

qreal ShadowEffect::getMargin() {
    return mBlurRadius->qra_getCurrentValue() +
            pointToLen(mTranslation->qra_getCurrentValue());
}

Property *ShadowEffect::prp_makeDuplicate() {
    ShadowEffect *newShadow = new ShadowEffect();
    prp_makeDuplicate(newShadow);
    return newShadow;
}

void ShadowEffect::prp_makeDuplicate(Property *target) {
    ShadowEffect *shadowTarget = (ShadowEffect*)target;

    shadowTarget->duplicateTranslationAnimatorFrom(mTranslation.data());
    shadowTarget->duplicateBlurRadiusAnimatorFrom(mBlurRadius.data());
    shadowTarget->duplicateColorAnimatorFrom(mColor.data());
    shadowTarget->duplicateOpacityAnimatorFrom(mOpacity.data());
}

void ShadowEffect::duplicateTranslationAnimatorFrom(QPointFAnimator *source) {
    source->prp_makeDuplicate(mTranslation.data());
}

void ShadowEffect::duplicateBlurRadiusAnimatorFrom(
        QrealAnimator *source) {
    source->prp_makeDuplicate(mBlurRadius.data());
}

void ShadowEffect::duplicateColorAnimatorFrom(ColorAnimator *source) {
    source->prp_makeDuplicate(mColor.data());
}

void ShadowEffect::duplicateOpacityAnimatorFrom(QrealAnimator *source) {
    source->prp_makeDuplicate(mOpacity.data());
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

void LinesEffect::apply(BoundingBox *target,
                        QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale)
{
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

#include "Boxes/boundingbox.h"
void CirclesEffect::apply(BoundingBox *target,
                          QImage *imgPtr,
                          const fmt_filters::image &img,
                          qreal scale)
{
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

void SwirlEffect::apply(BoundingBox *target,
                        QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale) {
    Q_UNUSED(imgPtr);
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

void OilEffect::apply(BoundingBox *target,
                        QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale) {
    Q_UNUSED(imgPtr);
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

int ImplodeEffect::prp_saveToSql(QSqlQuery *query, const int &boundingBoxSqlId) {
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

void ImplodeEffect::apply(BoundingBox *target,
                        QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale) {
    Q_UNUSED(imgPtr);
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

void DesaturateEffect::apply(BoundingBox *target,
                        QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale) {
    Q_UNUSED(imgPtr);
    fmt_filters::desaturate(img,
                            mInfluenceAnimator->qra_getCurrentValue());
}

AlphaMatteEffect::AlphaMatteEffect(BoundingBox *parentBox) :
    PixmapEffect(EFFECT_ALPHA_MATTE) {
    prp_setName("alpha matte");

    mInfluenceAnimator->qra_setValueRange(0., 1.);
    mInfluenceAnimator->qra_setCurrentValue(1.);
    mInfluenceAnimator->setPrefferedValueStep(0.05);
    mInfluenceAnimator->prp_setName("influence");
    ca_addChildAnimator(mInfluenceAnimator.data());

    mBoxTarget->prp_setName("target");
    mBoxTarget->setParentBox(parentBox);
    ca_addChildAnimator(mBoxTarget.data());

    mInvertedProperty->prp_setName("invert");
    ca_addChildAnimator(mInvertedProperty.data());
}

int AlphaMatteEffect::prp_saveToSql(QSqlQuery *query,
                                    const int &boundingBoxSqlId) {
    int pixmapEffectId = PixmapEffect::prp_saveToSql(query,
                                                     boundingBoxSqlId);
    int infId = mInfluenceAnimator->prp_saveToSql(query);
    BoundingBox *target = mBoxTarget->getTarget();
    int boundingBoxId;
    if(target == NULL) {
         boundingBoxId = -1;
    } else {
        boundingBoxId = target->getSqlId();
    }

    if(!query->exec(
        QString("INSERT INTO alphamatteeffect (pixmapeffectid, "
                "influenceid, boundingboxid, inverted) "
                "VALUES (%1, %2, %3, %4)").
                arg(pixmapEffectId).
                arg(infId).
                arg(boundingBoxId).
                arg(mInvertedProperty->getValue()) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }
    return pixmapEffectId;
}

void AlphaMatteEffect::prp_loadFromSql(const int &identifyingId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM alphamatteeffect WHERE pixmapeffectid = " +
            QString::number(identifyingId);
    if(query.exec(queryStr)) {
        query.next();
        mInfluenceAnimator->prp_loadFromSql(query.value("influenceid").toInt() );
        mInvertedProperty->setValue(query.value("inverted").toBool() );
    } else {
        qDebug() << "Could not load alphamatteeffect with id " << identifyingId;
    }
}

void AlphaMatteEffect::apply(BoundingBox *target,
                             QImage *imgPtr,
                             const fmt_filters::image &img,
                             qreal scale) {
    BoundingBox *boxTarget = mBoxTarget->getTarget();
    if(boxTarget) {
        qreal influence = mInfluenceAnimator->qra_getCurrentValue();
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
        if(mInvertedProperty->getValue()) {
            p2.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        } else {
            p2.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        }

        p2.drawImage(0, 0, imgTmp);

        p2.end();
    }
}
