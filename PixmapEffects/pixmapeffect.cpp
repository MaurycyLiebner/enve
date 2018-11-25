#include "pixmapeffect.h"
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include "Boxes/boxesgroup.h"
#include "fmt_filters.h"
#include "pointhelpers.h"
#include "Animators/coloranimator.h"
#include "Animators/qpointfanimator.h"
#include "Properties/boolproperty.h"

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

PixmapEffectRenderData::~PixmapEffectRenderData() {}

PixmapEffect::PixmapEffect(const QString &name,
                           const PixmapEffectType &type) :
    ComplexAnimator(name) {
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

BlurEffect::BlurEffect(qreal radius) : PixmapEffect("blur", EFFECT_BLUR) {
    mHighQuality = SPtrCreate(BoolProperty)();
    mBlurRadius = SPtrCreate(QrealAnimator)();
    mBlurRadius->qra_setCurrentValue(radius);
    mBlurRadius->prp_setName("radius");
    mBlurRadius->qra_setValueRange(0., 1000.);

    mHighQuality->setValue(false);
    mHighQuality->prp_setName("high quality");

    ca_addChildAnimator(mHighQuality);
    ca_addChildAnimator(mBlurRadius);
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
            fmt_filters::fast_blur(img, qRound(radius*0.5));
            fmt_filters::fast_blur(img, qRound(radius*0.5));
        }
    } else {
        if(hasKeys) {
            fmt_filters::anim_fast_blur(img, qRound(radius*0.8));
        } else {
            fmt_filters::fast_blur(img, qRound(radius*0.8));
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
            fmt_filters::fast_blur(img, qRound(radius*0.5));
            fmt_filters::fast_blur(img, qRound(radius*0.5));
        }
    } else {
        if(hasKeys) {
            fmt_filters::anim_fast_blur(img, radius*0.8);
        } else {
            fmt_filters::fast_blur(img, qRound(radius*0.8));
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
            fmt_filters::fast_blur(img, qRound(radius*0.5), opacity);
            fmt_filters::fast_blur(img, qRound(radius*0.5), opacity);
        }
    } else {
        if(hasKeys) {
            fmt_filters::anim_fast_blur(img, radius*0.8, opacity);
        } else {
            fmt_filters::fast_blur(img, qRound(radius*0.8), opacity);
        }
    }
}

qreal BlurEffect::getMargin() {
    return mBlurRadius->qra_getCurrentValue();
}

qreal BlurEffect::getMarginAtRelFrame(const int &relFrame) {
    return mBlurRadius->qra_getEffectiveValueAtRelFrame(relFrame);
}

PixmapEffectRenderDataSPtr BlurEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = BlurEffectRenderData::createBlurEffectRenderData();
    renderData->blurRadius = mBlurRadius->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->hasKeys = mBlurRadius->prp_hasKeys();
    renderData->highQuality = mHighQuality->getValue();
    return renderData;
}

ShadowEffect::ShadowEffect(qreal radius) : PixmapEffect("shadow", EFFECT_SHADOW) {
    mHighQuality = SPtrCreate(BoolProperty)("high quality");
    mBlurRadius = SPtrCreate(QrealAnimator)("blur radius");
    mOpacity = SPtrCreate(QrealAnimator)("opacity");
    mColor = SPtrCreate(ColorAnimator)();
    mTranslation = SPtrCreate(QPointFAnimator)("translation");

    mBlurRadius->qra_setCurrentValue(radius);

    mHighQuality->setValue(false);
    ca_addChildAnimator(mHighQuality);

    mBlurRadius->qra_setValueRange(0., 1000.);
    ca_addChildAnimator(mBlurRadius);

    mTranslation->setCurrentPointValue(QPointF(0., 0.));
    mTranslation->setValuesRange(-1000., 1000.);
    ca_addChildAnimator(mTranslation);

    mColor->qra_setCurrentValue(Qt::black);
    ca_addChildAnimator(mColor);

    mOpacity->qra_setValueRange(0., 1000.);
    mOpacity->qra_setCurrentValue(100.);
    ca_addChildAnimator(mOpacity);
//    mScale.setCurrentValue(1.);
//    mScale.setName("scale");
//    mScale.blockPointer();
//    mScale.setValueRange(0., 10.);
//    addChildAnimator(&mScale);
}

PixmapEffectRenderDataSPtr ShadowEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = ShadowEffectRenderData::createShadowEffectRenderData();
    renderData->blurRadius = mBlurRadius->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->hasKeys = mBlurRadius->prp_hasKeys();
    renderData->highQuality = mHighQuality->getValue();
    renderData->color = mColor->getColorAtRelFrameF(relFrame);
    renderData->translation = mTranslation->
            getCurrentEffectivePointValueAtRelFrameF(relFrame);
    renderData->opacity = mOpacity->getCurrentEffectiveValueAtRelFrameF(relFrame)/100.;
    return renderData;
}

void applyShadow(const SkBitmap &imgPtr,
                 const qreal &scale,
                 const qreal &blurRadius,
                 const QColor &currentColor,
                 const QPointF &trans,
                 const bool &hasKeys,
                 const bool &highQuality,
                 const qreal &opacity = 1.) {
    SkBitmap shadowBitmap;
    shadowBitmap.allocPixels(imgPtr.info());

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kDstIn);
    SkCanvas *shadowCanvas = new SkCanvas(shadowBitmap);
    shadowCanvas->clear(QColorToSkColor(currentColor));
    shadowCanvas->drawBitmap(imgPtr, 0, 0, &paint);
    shadowCanvas->flush();
    delete shadowCanvas;

    SkPixmap shadowPixmap;
    shadowBitmap.peekPixels(&shadowPixmap);
    fmt_filters::image shadowImg(static_cast<uint8_t*>(shadowPixmap.writable_addr()),
                                 shadowBitmap.width(), shadowBitmap.height());

    if(opacity > 1.) {
        applyBlur(shadowImg, scale,
                  blurRadius, highQuality,
                  hasKeys, opacity);
    } else {
        applyBlur(shadowImg, scale,
                  blurRadius, highQuality,
                  hasKeys);
        int alphaT = qMin(255, qMax(0, qRound(opacity*255) ));
        paint.setAlpha(static_cast<U8CPU>(alphaT));
    }

    SkCanvas *dstCanvas = new SkCanvas(imgPtr);
    paint.setBlendMode(SkBlendMode::kDstOver);
    dstCanvas->drawBitmap(shadowBitmap,
                          qrealToSkScalar(trans.x()*scale),
                          qrealToSkScalar(trans.y()*scale),
                          &paint);
    dstCanvas->flush();
    delete dstCanvas;
}

void ShadowEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                            const fmt_filters::image &img,
                                            const qreal &scale) {
    Q_UNUSED(img);
    applyShadow(imgPtr, scale,
                blurRadius,
                color,
                translation,
                hasKeys,
                highQuality,
                opacity);
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
    PixmapEffect("lines", EFFECT_LINES) {
    mLinesDistance = SPtrCreate(QrealAnimator)();
    mLinesWidth = SPtrCreate(QrealAnimator)();

    mLinesWidth->qra_setValueRange(0., 100000.);
    mLinesWidth->qra_setCurrentValue(linesWidth);
    mLinesWidth->prp_setName("width");
    ca_addChildAnimator(mLinesWidth);

    mLinesDistance->qra_setValueRange(0., 100000.);
    mLinesDistance->qra_setCurrentValue(linesDistance);
    mLinesDistance->prp_setName("distance");
    ca_addChildAnimator(mLinesDistance);
}


PixmapEffectRenderDataSPtr LinesEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = LinesEffectRenderData::createLinesEffectRenderData();
    renderData->linesDistance = mLinesDistance->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->linesWidth = mLinesWidth->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->vertical = mVertical;

    return renderData;
}

CirclesEffect::CirclesEffect(qreal circlesRadius,
                             qreal circlesDistance) :
    PixmapEffect(EFFECT_CIRCLES) {
    mCirclesDistance = SPtrCreate(QrealAnimator)();
    mCirclesRadius = SPtrCreate(QrealAnimator)();

    prp_setName("circles");

    mCirclesRadius->qra_setValueRange(0., 1000.);
    mCirclesRadius->qra_setCurrentValue(circlesRadius);
    mCirclesRadius->prp_setName("radius");
    ca_addChildAnimator(mCirclesRadius);

    mCirclesDistance->qra_setValueRange(-1000., 1000.);
    mCirclesDistance->qra_setCurrentValue(circlesDistance);
    mCirclesDistance->prp_setName("distance");
    ca_addChildAnimator(mCirclesDistance);
}

PixmapEffectRenderDataSPtr CirclesEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = CirclesEffectRenderData::createCirclesEffectRenderData();
    renderData->circlesDistance = mCirclesDistance->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->circlesRadius = mCirclesRadius->getCurrentEffectiveValueAtRelFrameF(relFrame);

    return renderData;
}

SwirlEffect::SwirlEffect(qreal degrees) :
    PixmapEffect(EFFECT_SWIRL) {
    mDegreesAnimator = SPtrCreate(QrealAnimator)();
    prp_setName("swirl");

    mDegreesAnimator->qra_setValueRange(-3600., 3600.);
    mDegreesAnimator->qra_setCurrentValue(degrees);
    mDegreesAnimator->prp_setName("degrees");
    ca_addChildAnimator(mDegreesAnimator);
}

PixmapEffectRenderDataSPtr SwirlEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SwirlEffectRenderData::createSwirlEffectRenderData();
    renderData->degrees = mDegreesAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return renderData;
}

OilEffect::OilEffect(qreal radius) : PixmapEffect(EFFECT_OIL) {
    mRadiusAnimator = SPtrCreate(QrealAnimator)();
    prp_setName("oil");

    mRadiusAnimator->qra_setValueRange(1., 5.);
    mRadiusAnimator->qra_setCurrentValue(radius);
    mRadiusAnimator->prp_setName("radius");
    ca_addChildAnimator(mRadiusAnimator);
}

PixmapEffectRenderDataSPtr OilEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = OilEffectRenderData::createOilEffectRenderData();
    renderData->radius = mRadiusAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return renderData;
}

ImplodeEffect::ImplodeEffect(qreal radius) :
    PixmapEffect(EFFECT_IMPLODE) {
    mFactorAnimator = SPtrCreate(QrealAnimator)();
    prp_setName("implode");

    mFactorAnimator->qra_setValueRange(0., 100.);
    mFactorAnimator->qra_setCurrentValue(radius);
    mFactorAnimator->prp_setName("factor");
    ca_addChildAnimator(mFactorAnimator);
}

PixmapEffectRenderDataSPtr ImplodeEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = ImplodeEffectRenderData::createImplodeEffectRenderData();
    renderData->factor = mFactorAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return renderData;
}


DesaturateEffect::DesaturateEffect(qreal radius) :
    PixmapEffect(EFFECT_DESATURATE) {
    mInfluenceAnimator = SPtrCreate(QrealAnimator)();
    prp_setName("desaturate");

    mInfluenceAnimator->qra_setValueRange(0., 1.);
    mInfluenceAnimator->qra_setCurrentValue(radius);
    mInfluenceAnimator->prp_setName("factor");
    ca_addChildAnimator(mInfluenceAnimator);
}

PixmapEffectRenderDataSPtr DesaturateEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = DesaturateEffectRenderData::createDesaturateEffectRenderData();
    renderData->influence =
            mInfluenceAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return renderData;
}

void DesaturateEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                                const fmt_filters::image &img,
                                                const qreal &scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    fmt_filters::desaturate(img, qrealToSkScalar(influence));
}

PixmapEffectRenderDataSPtr ColorizeEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = ColorizeEffectRenderData::createColorizeEffectRenderData();
    renderData->hue =
            mHueAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->saturation =
            mSaturationAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->lightness =
            mLightnessAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->alpha =
            mAlphaAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return renderData;
}

ColorizeEffect::ColorizeEffect() :
    PixmapEffect(EFFECT_COLORIZE) {
    prp_setName("colorize");

    mHueAnimator = SPtrCreate(QrealAnimator)();
    mSaturationAnimator = SPtrCreate(QrealAnimator)();
    mLightnessAnimator = SPtrCreate(QrealAnimator)();
    mAlphaAnimator = SPtrCreate(QrealAnimator)();

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

    ca_addChildAnimator(mHueAnimator);
    ca_addChildAnimator(mSaturationAnimator);
    ca_addChildAnimator(mLightnessAnimator);
    ca_addChildAnimator(mAlphaAnimator);
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

    mFromColor = SPtrCreate(ColorAnimator)();
    mToColor = SPtrCreate(ColorAnimator)();
    mToleranceAnimator = SPtrCreate(QrealAnimator)();
    mSmoothnessAnimator = SPtrCreate(QrealAnimator)();

    mFromColor->prp_setName("from color");

    mToColor->prp_setName("to color");

    mToleranceAnimator->prp_setName("tolerance");
    mToleranceAnimator->qra_setValueRange(0., 1.);
    mToleranceAnimator->setPrefferedValueStep(0.01);

    mSmoothnessAnimator->prp_setName("smoothness");
    mSmoothnessAnimator->qra_setValueRange(0., 1.);
    mSmoothnessAnimator->setPrefferedValueStep(0.01);

    ca_addChildAnimator(mFromColor);
    ca_addChildAnimator(mToColor);
    ca_addChildAnimator(mToleranceAnimator);
    ca_addChildAnimator(mSmoothnessAnimator);
}

PixmapEffectRenderDataSPtr ReplaceColorEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = ReplaceColorEffectRenderData::
            createReplaceColorEffectRenderData();
    QColor fromColor = mFromColor->getColorAtRelFrameF(relFrame);
    QColor toColor = mToColor->getColorAtRelFrameF(relFrame);

    renderData->redR = qRound(fromColor.red()*fromColor.alphaF());
    renderData->greenR = qRound(fromColor.green()*fromColor.alphaF());
    renderData->blueR = qRound(fromColor.blue()*fromColor.alphaF());
    renderData->alphaR = fromColor.alpha();

    renderData->redT = qRound(toColor.red()*toColor.alphaF());
    renderData->greenT = qRound(toColor.green()*toColor.alphaF());
    renderData->blueT = qRound(toColor.blue()*toColor.alphaF());
    renderData->alphaT = toColor.alpha();

    renderData->tolerance = qRound(mToleranceAnimator->
            getCurrentEffectiveValueAtRelFrameF(relFrame)*255);
    renderData->smoothness = mSmoothnessAnimator->
            getCurrentEffectiveValueAtRelFrameF(relFrame);
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
    mContrastAnimator = SPtrCreate(QrealAnimator)();
    prp_setName("contrast");

    mContrastAnimator->qra_setValueRange(-255., 255.);
    mContrastAnimator->qra_setCurrentValue(contrast);
    mContrastAnimator->prp_setName("contrast");
    ca_addChildAnimator(mContrastAnimator);
}

PixmapEffectRenderDataSPtr ContrastEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = ContrastEffectRenderData::createContrastEffectRenderData();
    renderData->contrast =
            mContrastAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
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
        fmt_filters::contrast(img, qRound(contrast));
    }
}

BrightnessEffect::BrightnessEffect(qreal brightness) :
    PixmapEffect(EFFECT_BRIGHTNESS) {
    mBrightnessAnimator = SPtrCreate(QrealAnimator)();
    prp_setName("brightness");

    mBrightnessAnimator->qra_setValueRange(-255., 255.);
    mBrightnessAnimator->qra_setCurrentValue(brightness);
    mBrightnessAnimator->prp_setName("brightness");
    ca_addChildAnimator(mBrightnessAnimator);
}

PixmapEffectRenderDataSPtr BrightnessEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData =
            BrightnessEffectRenderData::
            createBrightnessEffectRenderData();
    renderData->brightness =
            mBrightnessAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
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

SampledMotionBlurEffect::SampledMotionBlurEffect(BoundingBox *box) :
    PixmapEffect(EFFECT_MOTION_BLUR) {
    mParentBox = box;
    mOpacity = SPtrCreate(QrealAnimator)();
    mNumberSamples = SPtrCreate(QrealAnimator)();
    mFrameStep = SPtrCreate(QrealAnimator)();
    prp_setName("motion blur");

    mOpacity->qra_setValueRange(0., 100.);
    mOpacity->qra_setCurrentValue(100.);
    mOpacity->prp_setName("opacity");
    ca_addChildAnimator(mOpacity);
    mNumberSamples->qra_setValueRange(0.1, 99.);
    mNumberSamples->qra_setCurrentValue(1.);
    mNumberSamples->prp_setName("number of samples");
    ca_addChildAnimator(mNumberSamples);
    mFrameStep->qra_setValueRange(.1, 9.9);
    mFrameStep->qra_setCurrentValue(1.);
    mFrameStep->prp_setName("frame step");
    ca_addChildAnimator(mFrameStep);
}

PixmapEffectRenderDataSPtr SampledMotionBlurEffect::
getPixmapEffectRenderDataForRelFrameF(const qreal &relFrame,
                                     BoundingBoxRenderData* data) {
    if(!data->parentIsTarget) return nullptr;
    auto renderData =
            SampledMotionBlurEffectRenderData::
            createSampledMotionBlurEffectRenderData();
    renderData->opacity =
            mOpacity->getCurrentEffectiveValueAtRelFrameF(relFrame)*0.01;
    renderData->numberSamples =
            mNumberSamples->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->boxData = data;

    int numberFrames = qCeil(renderData->numberSamples);
    qreal frameStep = mFrameStep->getCurrentEffectiveValueAtRelFrameF(relFrame);
    qreal relFrameT = relFrame - numberFrames*frameStep;
    for(int i = 0; i < numberFrames; i++) {
        if(!mParentBox->isRelFrameFVisibleAndInVisibleDurationRect(relFrameT)) {
            if(i == numberFrames - 1) {
                renderData->numberSamples = qRound(renderData->numberSamples - 0.500001);
            } else {
                renderData->numberSamples -= 1;
            }
            relFrameT += frameStep;
            continue;
        }
        BoundingBoxRenderDataSPtr sampleRenderData =
                mParentBox->createRenderData();
        //mParentBox->setupBoundingBoxRenderDataForRelFrameF(i, sampleRenderData);
        sampleRenderData->parentIsTarget = false;
        sampleRenderData->useCustomRelFrame = true;
        sampleRenderData->customRelFrame = relFrameT;
        sampleRenderData->motionBlurTarget = data;
        sampleRenderData->addScheduler();
        sampleRenderData->addDependent(data);
        renderData->samples << sampleRenderData;

        relFrameT += frameStep;
    }
    return renderData;
}

void SampledMotionBlurEffect::prp_setAbsFrame(const int &frame) {
    PixmapEffect::prp_setAbsFrame(frame);
    int margin = qCeil(mNumberSamples->getCurrentEffectiveValueAtRelFrame(anim_mCurrentRelFrame)*
                       mFrameStep->getCurrentEffectiveValueAtRelFrame(anim_mCurrentRelFrame));
    int first, last;
    getParentBoxFirstLastMarginAjusted(&first, &last, anim_mCurrentRelFrame);
    if((frame >= first && frame < first + margin) ||
       (frame <= last && frame > last - margin)) {
        prp_callUpdater();
    }
}

void SampledMotionBlurEffect::getParentBoxFirstLastMarginAjusted(int *firstT,
                                                                 int *lastT,
                                                                 const int &relFrame) {
    int boxFirst;
    int boxLast;
    mParentBox->getFirstAndLastIdenticalForMotionBlur(&boxFirst,
                                                      &boxLast,
                                                      relFrame);
    int margin = qCeil(mNumberSamples->getCurrentEffectiveValueAtRelFrame(relFrame)*
                       mFrameStep->getCurrentEffectiveValueAtRelFrame(relFrame));
    if(boxFirst == INT_MIN) {
        if(boxLast != INT_MAX) {
            if(boxLast - margin < relFrame) {
                boxFirst = relFrame;
            }
        }
    } else {
        boxFirst += margin;
    }
    if(boxLast == INT_MAX) {
        if(boxFirst != INT_MIN) {
            if(boxFirst > relFrame) {
                boxLast = relFrame;
            }
        }
    } else {
        boxLast -= margin;
    }
    *firstT = boxFirst;
    *lastT = boxLast;
}

void SampledMotionBlurEffect::prp_getFirstAndLastIdenticalRelFrame(
        int *firstIdentical,
        int *lastIdentical,
        const int &relFrame) {
    int boxFirst;
    int boxLast;
    getParentBoxFirstLastMarginAjusted(&boxFirst, &boxLast, relFrame);
    int fId;
    int lId;
    PixmapEffect::prp_getFirstAndLastIdenticalRelFrame(&fId,
                                                       &lId,
                                                       relFrame);
    fId = qMax(fId, boxFirst);
    lId = qMin(lId, boxLast);
    if(lId > fId) {
        *firstIdentical = fId;
        *lastIdentical = lId;
    } else {
        *firstIdentical = relFrame;
        *lastIdentical = relFrame;
    }
}

void replaceIfHigherAlpha(const int &x0, const int &y0,
                          const SkBitmap &dst,
                          const sk_sp<SkImage> &src,
                          const qreal &alphaT) {
    SkPixmap dstP;
    SkPixmap srcP;
    dst.peekPixels(&dstP);
    src->peekPixels(&srcP);
    uint8_t *dstD = (uint8_t*)dstP.writable_addr();
    uint8_t *srcD = (uint8_t*)srcP.writable_addr();
    for(int y = 0; y < src->height() && y + y0 < dst.height(); y++) {
        for(int x = 0; x < src->width() && x + x0 < dst.width(); x++) {
            int dstRId = ((y + y0) * dst.width() + (x + x0))*4;
            int srcRId = (y * src->width() + x)*4;
            uchar dstAlpha = dstD[dstRId + 3];
            uchar srcAlpha = qRound(srcD[srcRId + 3]*alphaT);
            if(dstAlpha >= srcAlpha) continue;
            dstD[dstRId] = qRound(srcD[srcRId]*alphaT);
            dstD[dstRId + 1] = qRound(srcD[srcRId + 1]*alphaT);
            dstD[dstRId + 2] = qRound(srcD[srcRId + 2]*alphaT);
            dstD[dstRId + 3] = srcAlpha;
        }
    }
}

void replaceIfHigherAlpha(const int &x0, const int &y0,
                          const SkBitmap &dst,
                          const SkBitmap &src) {
    SkPixmap dstP;
    SkPixmap srcP;
    dst.peekPixels(&dstP);
    src.peekPixels(&srcP);
    uint8_t *dstD = (uint8_t*)dstP.writable_addr();
    uint8_t *srcD = (uint8_t*)srcP.writable_addr();
    for(int y = 0; y < src.height() && y + y0 < dst.height(); y++) {
        for(int x = 0; x < src.width() && x + x0 < dst.width(); x++) {
            int dstRId = ((y + y0) * dst.width() + (x + x0))*4;
            int srcRId = (y * src.width() + x)*4;
            uchar dstAlpha = dstD[dstRId + 3];
            uchar srcAlpha = srcD[srcRId + 3];
            if(dstAlpha >= srcAlpha) continue;
            dstD[dstRId] = srcD[srcRId];
            dstD[dstRId + 1] = srcD[srcRId + 1];
            dstD[dstRId + 2] = srcD[srcRId + 2];
            dstD[dstRId + 3] = srcAlpha;
        }
    }
}

void SampledMotionBlurEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                                       const fmt_filters::image &img,
                                                       const qreal &scale) {
    SkBitmap motionBlur;
    motionBlur.allocPixels(imgPtr.info());
    motionBlur.eraseColor(SK_ColorTRANSPARENT);
    //SkCanvas canvasSk(motionBlur);
    qreal opacityStepT = 1./(numberSamples + 1);
    qreal opacityT = opacityStepT*(1. - qCeil(numberSamples) + numberSamples);
    foreach(const BoundingBoxRenderDataSPtr &sample, samples) {
        qreal sampleAlpha = opacityT*opacityT*opacity;
        QPointF drawPos = sample->globalBoundingRect.topLeft() -
                boxData->globalBoundingRect.topLeft();//QPointF(0., 0.);
        replaceIfHigherAlpha(drawPos.x(), drawPos.y(),
                             motionBlur, sample->renderedImage,
                             sampleAlpha);
        opacityT += opacityStepT;
    }

    replaceIfHigherAlpha(0.f, 0.f,
                         imgPtr, motionBlur);
//    SkCanvas canvasSk2(imgPtr);
//    SkPaint paintT;
//    paintT.setBlendMode(SkBlendMode::kDstOver);
//    canvasSk2.drawBitmap(motionBlur, 0.f, 0.f, &paintT);
}

void LinesEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                           const fmt_filters::image &img,
                                           const qreal &scale) {
}

void SwirlEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                           const fmt_filters::image &img,
                                           const qreal &scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    fmt_filters::swirl(img, degrees, fmt_filters::rgba(0, 0, 0, 0));
}

void OilEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                         const fmt_filters::image &img,
                                         const qreal &scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    fmt_filters::oil(img, radius);
}

void ImplodeEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                             const fmt_filters::image &img,
                                             const qreal &scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    fmt_filters::implode(img, factor, fmt_filters::rgba(0, 0, 0, 0));
}

void CirclesEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                             const fmt_filters::image &img,
                                             const qreal &scale) {

}
