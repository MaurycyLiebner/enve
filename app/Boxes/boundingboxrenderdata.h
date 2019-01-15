#ifndef BOUNDINGBOXRENDERDATA_H
#define BOUNDINGBOXRENDERDATA_H
#include "skia/skiaincludes.h"

#include <QWeakPointer>
#include "updatable.h"
#include "Animators/animator.h"
#include <QMatrix>
struct PixmapEffectRenderData;
class BoundingBox;
class ShaderProgramCallerBase;
class GPURasterEffectCaller;
#include "smartPointers/sharedpointerdefs.h"

class RenderDataCustomizerFunctor;
struct BoundingBoxRenderData : public _ScheduledTask {
    friend class StdSelfRef;
    BoundingBoxRenderData(BoundingBox *parentBoxT);
    virtual ~BoundingBoxRenderData();

    virtual void copyFrom(BoundingBoxRenderData *src);
    stdsptr<BoundingBoxRenderData> makeCopy();
    bool fCopied = false;

    // gpu
    bool needsGpuProcessing() const {
        return !fGPUEffects.isEmpty();
    }
    QList<stdsptr<GPURasterEffectCaller>> fGPUEffects;
    // gpu


    bool fRelBoundingRectSet = false;

    Animator::UpdateReason fReason;

    bool fRedo = false;

    bool fRenderedToImage = false;
    QMatrix fTransform;
    QMatrix fParentTransform;
    QMatrix fRelTransform;
    QRectF fRelBoundingRect;
    QRectF fGlobalBoundingRect;
    qreal fOpacity = 1.;
    qreal fResolution;
    qreal fEffectsMargin;
    int fRelFrame;

    // for motion blur
    bool fUseCustomRelFrame = false;
    qreal fCustomRelFrame;
    QList<QRectF> fOtherGlobalRects;
    stdptr<BoundingBoxRenderData> fMotionBlurTarget;
    // for motion blur

    QList<stdsptr<PixmapEffectRenderData>> fPixmapEffects;
    SkPoint fDrawPos = SkPoint::Make(0.f, 0.f);
    SkBlendMode fBlendMode = SkBlendMode::kSrcOver;
    QRectF fMaxBoundsRect;
    bool fMaxBoundsEnabled = true;

    bool fParentIsTarget = true;
    qptr<BoundingBox> fParentBox;
    sk_sp<SkImage> fRenderedImage;

    virtual void updateRelBoundingRect();
    void drawRenderedImageForParent(SkCanvas *canvas);
    virtual void renderToImage();

    void _processUpdate();

    void beforeProcessingStarted();

    void afterProcessingFinished();

    void taskQued();

    virtual bool allDataReady() { return true; }

    void dataSet();

    void clearPixmapEffects() {
        fPixmapEffects.clear();
        fEffectsMargin = 0.;
    }

    virtual QPointF getCenterPosition() {
        return fRelBoundingRect.center();
    }

    void appendRenderCustomizerFunctor(
            const stdsptr<RenderDataCustomizerFunctor>& customizer) {
        mRenderDataCustomizerFunctors.append(customizer);
    }

    void prependRenderCustomizerFunctor(
            const stdsptr<RenderDataCustomizerFunctor>& customizer) {
        mRenderDataCustomizerFunctors.prepend(customizer);
    }
    bool nullifyBeforeProcessing();
protected:
    void scheduleTaskNow();
    QList<stdsptr<RenderDataCustomizerFunctor>> mRenderDataCustomizerFunctors;
    bool mDelayDataSet = false;
    bool mDataSet = false;
    virtual void drawSk(SkCanvas *canvas) = 0;
};

class RenderDataCustomizerFunctor : public StdSelfRef {
public:
    RenderDataCustomizerFunctor();
    virtual ~RenderDataCustomizerFunctor();
    virtual void customize(BoundingBoxRenderData *data) = 0;
    void operator()(BoundingBoxRenderData* data);
};

class ReplaceTransformDisplacementCustomizer : public RenderDataCustomizerFunctor {
public:
    ReplaceTransformDisplacementCustomizer(const qreal &dx,
                                           const qreal &dy);

    void customize(BoundingBoxRenderData *data);
protected:
    qreal mDx, mDy;
};

class MultiplyTransformCustomizer : public RenderDataCustomizerFunctor {
public:
    MultiplyTransformCustomizer(const QMatrix &transform,
                                const qreal &opacity = 1.);

    void customize(BoundingBoxRenderData *data);
protected:
    QMatrix mTransform;
    qreal mOpacity = 1.;
};

class MultiplyOpacityCustomizer : public RenderDataCustomizerFunctor {
public:
    MultiplyOpacityCustomizer(const qreal &opacity);

    void customize(BoundingBoxRenderData *data);
protected:
    qreal mOpacity;
};

#endif // BOUNDINGBOXRENDERDATA_H
