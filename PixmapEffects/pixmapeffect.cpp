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
    mHighQuality = (new BoolProperty())->ref<BoolProperty>();
    mBlurRadius = (new QrealAnimator())->ref<QrealAnimator>();
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

qreal BlurEffect::getMargin() {
    return mBlurRadius->qra_getCurrentValue();
}

qreal BlurEffect::getMarginAtRelFrame(const int &relFrame) {
    return mBlurRadius->qra_getEffectiveValueAtRelFrame(relFrame);
}

PixmapEffectRenderData *BlurEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, const std::shared_ptr<BoundingBoxRenderData>& ) {
    BlurEffectRenderData *renderData = new BlurEffectRenderData();
    renderData->blurRadius = mBlurRadius->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->hasKeys = mBlurRadius->prp_hasKeys();
    renderData->highQuality = mHighQuality->getValue();
    return renderData;
}

ShadowEffect::ShadowEffect(qreal radius) : PixmapEffect(EFFECT_SHADOW) {
    mHighQuality = (new BoolProperty())->ref<BoolProperty>();
    mBlurRadius = (new QrealAnimator())->ref<QrealAnimator>();
    mOpacity = (new QrealAnimator())->ref<QrealAnimator>();
    mColor = (new ColorAnimator())->ref<ColorAnimator>();
    mTranslation = (new QPointFAnimator())->ref<QPointFAnimator>();

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

PixmapEffectRenderData *ShadowEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, const std::shared_ptr<BoundingBoxRenderData>& ) {
    ShadowEffectRenderData *renderData = new ShadowEffectRenderData();
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
    mLinesDistance = (new QrealAnimator())->ref<QrealAnimator>();
    mLinesWidth = (new QrealAnimator())->ref<QrealAnimator>();

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


PixmapEffectRenderData *LinesEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, const std::shared_ptr<BoundingBoxRenderData> &) {
    LinesEffectRenderData *renderData = new LinesEffectRenderData();
    renderData->linesDistance = mLinesDistance->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->linesWidth = mLinesWidth->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->vertical = mVertical;

    return renderData;
}

CirclesEffect::CirclesEffect(qreal circlesRadius,
                             qreal circlesDistance) :
    PixmapEffect(EFFECT_CIRCLES) {
    mCirclesDistance = (new QrealAnimator())->ref<QrealAnimator>();
    mCirclesRadius = (new QrealAnimator())->ref<QrealAnimator>();

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

PixmapEffectRenderData *CirclesEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, const std::shared_ptr<BoundingBoxRenderData> &) {
    CirclesEffectRenderData *renderData = new CirclesEffectRenderData();
    renderData->circlesDistance = mCirclesDistance->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->circlesRadius = mCirclesRadius->getCurrentEffectiveValueAtRelFrameF(relFrame);

    return renderData;
}

SwirlEffect::SwirlEffect(qreal degrees) :
    PixmapEffect(EFFECT_SWIRL) {
    mDegreesAnimator = (new QrealAnimator())->ref<QrealAnimator>();
    prp_setName("swirl");

    mDegreesAnimator->qra_setValueRange(-3600., 3600.);
    mDegreesAnimator->qra_setCurrentValue(degrees);
    mDegreesAnimator->prp_setName("degrees");
    ca_addChildAnimator(mDegreesAnimator.data());
}

PixmapEffectRenderData *SwirlEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, const std::shared_ptr<BoundingBoxRenderData> &) {
    SwirlEffectRenderData *renderData = new SwirlEffectRenderData();
    renderData->degrees = mDegreesAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return renderData;
}

OilEffect::OilEffect(qreal radius) : PixmapEffect(EFFECT_OIL) {
    mRadiusAnimator = (new QrealAnimator())->ref<QrealAnimator>();
    prp_setName("oil");

    mRadiusAnimator->qra_setValueRange(1., 5.);
    mRadiusAnimator->qra_setCurrentValue(radius);
    mRadiusAnimator->prp_setName("radius");
    ca_addChildAnimator(mRadiusAnimator.data());
}

PixmapEffectRenderData *OilEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, const std::shared_ptr<BoundingBoxRenderData> &) {
    OilEffectRenderData *renderData = new OilEffectRenderData();
    renderData->radius = mRadiusAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return renderData;
}

ImplodeEffect::ImplodeEffect(qreal radius) :
    PixmapEffect(EFFECT_IMPLODE) {
    mFactorAnimator = (new QrealAnimator())->ref<QrealAnimator>();
    prp_setName("implode");

    mFactorAnimator->qra_setValueRange(0., 100.);
    mFactorAnimator->qra_setCurrentValue(radius);
    mFactorAnimator->prp_setName("factor");
    ca_addChildAnimator(mFactorAnimator.data());
}

PixmapEffectRenderData *ImplodeEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, const std::shared_ptr<BoundingBoxRenderData> &) {
    ImplodeEffectRenderData *renderData = new ImplodeEffectRenderData();
    renderData->factor = mFactorAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return renderData;
}


DesaturateEffect::DesaturateEffect(qreal radius) :
    PixmapEffect(EFFECT_DESATURATE) {
    mInfluenceAnimator = (new QrealAnimator())->ref<QrealAnimator>();
    prp_setName("desaturate");

    mInfluenceAnimator->qra_setValueRange(0., 1.);
    mInfluenceAnimator->qra_setCurrentValue(radius);
    mInfluenceAnimator->prp_setName("factor");
    ca_addChildAnimator(mInfluenceAnimator.data());
}

PixmapEffectRenderData *DesaturateEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, const std::shared_ptr<BoundingBoxRenderData>& ) {
    DesaturateEffectRenderData *renderData = new DesaturateEffectRenderData();
    renderData->influence =
            mInfluenceAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return renderData;
}

void DesaturateEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                                const fmt_filters::image &img,
                                                const qreal &scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    fmt_filters::desaturate(img, influence);
}

PixmapEffectRenderData *ColorizeEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, const std::shared_ptr<BoundingBoxRenderData>& ) {
    ColorizeEffectRenderData *renderData = new ColorizeEffectRenderData();
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

PixmapEffectRenderData *ReplaceColorEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, const std::shared_ptr<BoundingBoxRenderData>& ) {
    ReplaceColorEffectRenderData *renderData =
            new ReplaceColorEffectRenderData();
    QColor fromColor = mFromColor->getColorAtRelFrameF(relFrame).qcol;
    QColor toColor = mToColor->getColorAtRelFrameF(relFrame).qcol;

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
    mContrastAnimator = (new QrealAnimator())->ref<QrealAnimator>();
    prp_setName("contrast");

    mContrastAnimator->qra_setValueRange(-255., 255.);
    mContrastAnimator->qra_setCurrentValue(contrast);
    mContrastAnimator->prp_setName("contrast");
    ca_addChildAnimator(mContrastAnimator.data());
}

PixmapEffectRenderData *ContrastEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, const std::shared_ptr<BoundingBoxRenderData>& ) {
    ContrastEffectRenderData *renderData = new ContrastEffectRenderData();
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
        fmt_filters::contrast(img, contrast);
    }
}

BrightnessEffect::BrightnessEffect(qreal brightness) :
    PixmapEffect(EFFECT_BRIGHTNESS) {
    mBrightnessAnimator = (new QrealAnimator())->ref<QrealAnimator>();
    prp_setName("brightness");

    mBrightnessAnimator->qra_setValueRange(-255., 255.);
    mBrightnessAnimator->qra_setCurrentValue(brightness);
    mBrightnessAnimator->prp_setName("brightness");
    ca_addChildAnimator(mBrightnessAnimator.data());
}

PixmapEffectRenderData *BrightnessEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, const std::shared_ptr<BoundingBoxRenderData>& ) {
    BrightnessEffectRenderData *renderData = new BrightnessEffectRenderData();
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
    mOpacity = (new QrealAnimator())->ref<QrealAnimator>();
    mNumberSamples = (new QrealAnimator())->ref<QrealAnimator>();
    mFrameStep = (new QrealAnimator())->ref<QrealAnimator>();
    prp_setName("motion blur");

    mOpacity->qra_setValueRange(0., 100.);
    mOpacity->qra_setCurrentValue(100.);
    mOpacity->prp_setName("opacity");
    ca_addChildAnimator(mOpacity.data());
    mNumberSamples->qra_setValueRange(0.1, 99.);
    mNumberSamples->qra_setCurrentValue(1.);
    mNumberSamples->prp_setName("number of samples");
    ca_addChildAnimator(mNumberSamples.data());
    mFrameStep->qra_setValueRange(.1, 9.9);
    mFrameStep->qra_setCurrentValue(1.);
    mFrameStep->prp_setName("frame step");
    ca_addChildAnimator(mFrameStep.data());
}

PixmapEffectRenderData *SampledMotionBlurEffect::
getPixmapEffectRenderDataForRelFrameF(const qreal &relFrame,
                                     const std::shared_ptr<BoundingBoxRenderData>& data) {
    if(!data->parentIsTarget) return nullptr;
    SampledMotionBlurEffectRenderData *renderData =
            new SampledMotionBlurEffectRenderData();
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
        std::shared_ptr<BoundingBoxRenderData> sampleRenderData = mParentBox->createRenderData();
        //mParentBox->setupBoundingBoxRenderDataForRelFrameF(i, sampleRenderData);
        sampleRenderData->parentIsTarget = false;
        sampleRenderData->useCustomRelFrame = true;
        sampleRenderData->customRelFrame = relFrameT;
        sampleRenderData->motionBlurTarget = data;
        renderData->samples << sampleRenderData;
        sampleRenderData->addScheduler();
        sampleRenderData->addDependent(data);
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
