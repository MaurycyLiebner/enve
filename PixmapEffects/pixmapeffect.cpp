#include "pixmapeffect.h"
#include <QDrag>
#include <QMimeData>
#include <QDebug>
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

void applyBlur(const fmt_filters::image &img,
               const qreal &scale,
               const qreal &blurRadius,
               const bool &highQuality,
               const bool &hasKeys,
               const qreal &opacity) {
    qreal radius = blurRadius*scale;

    if(highQuality) {
        if(hasKeys) {
            fmt_filters::anim_fast_blur(img, radius*0.5, opacity);
            fmt_filters::anim_fast_blur(img, radius*0.5, opacity);
        } else {
            fmt_filters::fast_blur(img, radius*0.5, opacity);
            fmt_filters::fast_blur(img, radius*0.5, opacity);
        }
    } else {
        if(hasKeys) {
            fmt_filters::anim_fast_blur(img, radius*0.8, opacity);
        } else {
            fmt_filters::fast_blur(img, radius*0.8, opacity);
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
    return mBlurRadius->qra_getEffectiveValueAtRelFrame(relFrame);
}

PixmapEffectRenderData *BlurEffect::getPixmapEffectRenderDataForRelFrame(
                                    const int &relFrame) {
    BlurEffectRenderData *renderData = new BlurEffectRenderData();
    renderData->blurRadius = mBlurRadius->getCurrentEffectiveValueAtRelFrame(relFrame);
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

    mOpacity->prp_setName("opacity");
    mOpacity->qra_setValueRange(0., 1000.);
    mOpacity->qra_setCurrentValue(100.);
    ca_addChildAnimator(mOpacity.data());
//    mScale.setCurrentValue(1.);
//    mScale.setName("scale");
//    mScale.blockPointer();
//    mScale.setValueRange(0., 10.);
//    addChildAnimator(&mScale);
}

PixmapEffectRenderData *ShadowEffect::getPixmapEffectRenderDataForRelFrame(
                                    const int &relFrame) {
    ShadowEffectRenderData *renderData = new ShadowEffectRenderData();
    renderData->blurRadius = mBlurRadius->getCurrentEffectiveValueAtRelFrame(relFrame);
    renderData->hasKeys = mBlurRadius->prp_hasKeys();
    renderData->highQuality = mHighQuality->getValue();
    renderData->color = mColor->getColorAtRelFrame(relFrame);
    renderData->translation = mTranslation->
            getCurrentEffectivePointValueAtRelFrame(relFrame);
    renderData->opacity = mOpacity->getCurrentEffectiveValueAtRelFrame(relFrame)/100.;
    return renderData;
}

void applyShadow(const SkBitmap &imgPtr,
                 const qreal &scale,
                 const qreal &blurRadius,
                 const Color &currentColor,
                 const QPointF &trans,
                 const bool &hasKeys,
                 const bool &highQuality,
                 const qreal &opacity = 1.) {
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

    if(opacity > 1.) {
        applyBlur(shadowImg, scale,
                  blurRadius, highQuality,
                  hasKeys, opacity);
    } else {
        applyBlur(shadowImg, scale,
                  blurRadius, highQuality,
                  hasKeys);
        paint.setAlpha(qMin(255, qMax(0, (int)(opacity*255) )));
    }

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
                highQuality,
                opacity);
}

void ShadowEffect::applySk(const SkBitmap &imgPtr,
                           const fmt_filters::image &img,
                           qreal scale) {
    Q_UNUSED(img);
    applyShadow(imgPtr, scale,
                mBlurRadius->qra_getCurrentEffectiveValue(),
                mColor->getCurrentColor(),
                mTranslation->getCurrentEffectivePointValue(),
                mBlurRadius->prp_hasKeys(),
                mHighQuality->getValue(),
                mOpacity->qra_getCurrentEffectiveValue());
}

qreal ShadowEffect::getMargin() {
    return mBlurRadius->qra_getCurrentValue() +
            pointToLen(mTranslation->getCurrentEffectivePointValue());
}

qreal ShadowEffect::getMarginAtRelFrame(const int &relFrame) {
    return mBlurRadius->qra_getEffectiveValueAtRelFrame(relFrame) +
            pointToLen(mTranslation->getCurrentEffectivePointValueAtRelFrame(relFrame));
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

PixmapEffectRenderData *DesaturateEffect::getPixmapEffectRenderDataForRelFrame(
        const int &relFrame) {
    DesaturateEffectRenderData *renderData = new DesaturateEffectRenderData();
    renderData->influence =
            mInfluenceAnimator->getCurrentEffectiveValueAtRelFrame(relFrame);
    return renderData;
}

void DesaturateEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                                const fmt_filters::image &img,
                                                const qreal &scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    fmt_filters::desaturate(img,
                            influence);
}

PixmapEffectRenderData *ColorizeEffect::getPixmapEffectRenderDataForRelFrame(
        const int &relFrame) {
    ColorizeEffectRenderData *renderData = new ColorizeEffectRenderData();
    renderData->hue =
            mHueAnimator->getCurrentEffectiveValueAtRelFrame(relFrame);
    renderData->saturation =
            mSaturationAnimator->getCurrentEffectiveValueAtRelFrame(relFrame);
    renderData->lightness =
            mLightnessAnimator->getCurrentEffectiveValueAtRelFrame(relFrame);
    renderData->alpha =
            mAlphaAnimator->getCurrentEffectiveValueAtRelFrame(relFrame);
    return renderData;
}

ColorizeEffect::ColorizeEffect() :
    PixmapEffect(EFFECT_COLORIZE) {
    prp_setName("colorize");

    mHueAnimator = (new QrealAnimator())->ref<QrealAnimator>();
    mSaturationAnimator = (new QrealAnimator())->ref<QrealAnimator>();
    mLightnessAnimator = (new QrealAnimator())->ref<QrealAnimator>();
    mAlphaAnimator = (new QrealAnimator())->ref<QrealAnimator>();

    mHueAnimator->prp_setName("hue");
    mHueAnimator->qra_setValueRange(0., 1.);
    mHueAnimator->setPrefferedValueStep(0.01);

    mSaturationAnimator->prp_setName("saturation");
    mSaturationAnimator->qra_setValueRange(0., 1.);
    mSaturationAnimator->setPrefferedValueStep(0.01);

    mLightnessAnimator->prp_setName("lightness");
    mLightnessAnimator->qra_setValueRange(-1., 1.);
    mLightnessAnimator->setPrefferedValueStep(0.01);

    mAlphaAnimator->prp_setName("alpha");
    mAlphaAnimator->qra_setValueRange(0., 1.);
    mAlphaAnimator->setPrefferedValueStep(0.01);

    ca_addChildAnimator(mHueAnimator.data());
    ca_addChildAnimator(mSaturationAnimator.data());
    ca_addChildAnimator(mLightnessAnimator.data());
    ca_addChildAnimator(mAlphaAnimator.data());
}

void ColorizeEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                              const fmt_filters::image &img,
                                              const qreal &scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    fmt_filters::colorizeHSV(img,
                             hue, saturation, lightness, alpha);
}

ReplaceColorEffect::ReplaceColorEffect() :
    PixmapEffect(EFFECT_REPLACE_COLOR) {
    prp_setName("replace color");

    mFromColor = (new ColorAnimator())->ref<ColorAnimator>();
    mToColor = (new ColorAnimator())->ref<ColorAnimator>();
    mToleranceAnimator = (new QrealAnimator())->ref<QrealAnimator>();
    mSmoothnessAnimator = (new QrealAnimator())->ref<QrealAnimator>();

    mFromColor->prp_setName("from color");

    mToColor->prp_setName("to color");

    mToleranceAnimator->prp_setName("tolerance");
    mToleranceAnimator->qra_setValueRange(0., 1.);
    mToleranceAnimator->setPrefferedValueStep(0.01);

    mSmoothnessAnimator->prp_setName("smoothness");
    mSmoothnessAnimator->qra_setValueRange(0., 1.);
    mSmoothnessAnimator->setPrefferedValueStep(0.01);

    ca_addChildAnimator(mFromColor.data());
    ca_addChildAnimator(mToColor.data());
    ca_addChildAnimator(mToleranceAnimator.data());
    ca_addChildAnimator(mSmoothnessAnimator.data());
}

PixmapEffectRenderData *ReplaceColorEffect::getPixmapEffectRenderDataForRelFrame(
        const int &relFrame) {
    ReplaceColorEffectRenderData *renderData =
            new ReplaceColorEffectRenderData();
    QColor fromColor = mFromColor->getColorAtRelFrame(relFrame).qcol;
    QColor toColor = mToColor->getColorAtRelFrame(relFrame).qcol;

    renderData->redR = qRound(fromColor.red()*fromColor.alphaF());
    renderData->greenR = qRound(fromColor.green()*fromColor.alphaF());
    renderData->blueR = qRound(fromColor.blue()*fromColor.alphaF());
    renderData->alphaR = fromColor.alpha();

    renderData->redT = qRound(toColor.red()*toColor.alphaF());
    renderData->greenT = qRound(toColor.green()*toColor.alphaF());
    renderData->blueT = qRound(toColor.blue()*toColor.alphaF());
    renderData->alphaT = toColor.alpha();

    renderData->tolerance = qRound(mToleranceAnimator->
            getCurrentEffectiveValueAtRelFrame(relFrame)*255);
    renderData->smoothness = mSmoothnessAnimator->
            getCurrentEffectiveValueAtRelFrame(relFrame);
    return renderData;
}

void ReplaceColorEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                              const fmt_filters::image &img,
                                              const qreal &scale) {
    Q_UNUSED(imgPtr);
    fmt_filters::replaceColor(img, redR, greenR, blueR, alphaR,
                              redT, greenT, blueT, alphaT,
                              tolerance, smoothness*scale);
}

ContrastEffect::ContrastEffect(qreal contrast) :
    PixmapEffect(EFFECT_CONTRAST) {
    prp_setName("contrast");

    mContrastAnimator->qra_setValueRange(-255., 255.);
    mContrastAnimator->qra_setCurrentValue(contrast);
    mContrastAnimator->prp_setName("contrast");
    ca_addChildAnimator(mContrastAnimator.data());
}

PixmapEffectRenderData *ContrastEffect::getPixmapEffectRenderDataForRelFrame(
        const int &relFrame) {
    ContrastEffectRenderData *renderData = new ContrastEffectRenderData();
    renderData->contrast =
            mContrastAnimator->getCurrentEffectiveValueAtRelFrame(relFrame);
    renderData->hasKeys = mContrastAnimator->prp_hasKeys();
    return renderData;
}

void ContrastEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                                const fmt_filters::image &img,
                                                const qreal &scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    if(hasKeys) {
        fmt_filters::anim_contrast(img, contrast);
    } else {
        fmt_filters::contrast(img, contrast);
    }
}

BrightnessEffect::BrightnessEffect(qreal brightness) :
    PixmapEffect(EFFECT_BRIGHTNESS) {
    prp_setName("brightness");

    mBrightnessAnimator->qra_setValueRange(-255., 255.);
    mBrightnessAnimator->qra_setCurrentValue(brightness);
    mBrightnessAnimator->prp_setName("brightness");
    ca_addChildAnimator(mBrightnessAnimator.data());
}

PixmapEffectRenderData *BrightnessEffect::getPixmapEffectRenderDataForRelFrame(
        const int &relFrame) {
    BrightnessEffectRenderData *renderData = new BrightnessEffectRenderData();
    renderData->brightness =
            mBrightnessAnimator->getCurrentEffectiveValueAtRelFrame(relFrame);
    renderData->hasKeys = mBrightnessAnimator->prp_hasKeys();
    return renderData;
}

void BrightnessEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                                const fmt_filters::image &img,
                                                const qreal &scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    if(hasKeys) {
        fmt_filters::anim_brightness(img, brightness);
    } else {
        fmt_filters::brightness(img, brightness);
    }
}
