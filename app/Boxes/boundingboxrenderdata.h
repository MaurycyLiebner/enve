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
protected:
    BoundingBoxRenderData(BoundingBox *parentBoxT);

    virtual void drawSk(SkCanvas * const canvas) = 0;
    virtual void transformRenderCanvas(SkCanvas& canvas) const {
        canvas.concat(toSkMatrix(fScaledTransform));
    }
    virtual void copyFrom(BoundingBoxRenderData *src);
    virtual void updateRelBoundingRect();

    void scheduleTaskNow();
public:
    virtual void renderToImage();
    virtual QPointF getCenterPosition() {
        return fRelBoundingRect.center();
    }

    void _processUpdate();
    void beforeProcessingStarted();
    void afterProcessingFinished();
    void taskQued();

    // gpu
    bool needsGpuProcessing() const {
        return !fGPUEffects.isEmpty();
    }
    QList<stdsptr<GPURasterEffectCaller>> fGPUEffects;
    // gpu

    stdsptr<BoundingBoxRenderData> makeCopy();
    bool fCopied = false;
    bool fRelBoundingRectSet = false;

    Animator::UpdateReason fReason;

    bool fRedo = false;
    uint fBoxStateId = 0;

    bool fRenderedToImage = false;

    QMatrix fResolutionScale;
    QMatrix fScaledTransform;
    QMatrix fTransform;
    QMatrix fParentTransform;
    QMatrix fRelTransform;
    QMatrix fRenderTransform;

    QRectF fRelBoundingRect;
    QRectF fGlobalBoundingRect;
    qreal fOpacity = 1;
    qreal fResolution;
    qreal fEffectsMargin;
    int fRelFrame;

    // for motion blur
    bool fUseCustomRelFrame = false;
    qreal fCustomRelFrame;
    QList<QRectF> fOtherGlobalRects;
    stdptr<BoundingBoxRenderData> fMotionBlurTarget;
    // for motion blur

    QList<stdsptr<PixmapEffectRenderData>> fRasterEffects;
    SkPoint fDrawPos = SkPoint::Make(0, 0);
    SkBlendMode fBlendMode = SkBlendMode::kSrcOver;
    QRectF fMaxBoundsRect;
    bool fMaxBoundsEnabled = true;

    bool fParentIsTarget = true;
    qptr<BoundingBox> fParentBox;
    sk_sp<SkImage> fRenderedImage;
    SkBitmap fBitmapTMP;

    void drawRenderedImageForParent(SkCanvas * const canvas);

    void dataSet();

    void clearPixmapEffects() {
        fRasterEffects.clear();
        fEffectsMargin = 0;
    }

    void setupWhenNoRenderingNeeded(const sk_sp<SkImage> image,
                                    const QMatrix& renderTransform) {
        fRenderedImage = image;
        fRenderTransform = renderTransform;
        updateRelBoundingRect();
        updateGlobalFromRelBoundingRect();
        dataSet();
        fResolutionScale.reset();
        fResolutionScale.scale(fResolution, fResolution);
        fScaledTransform = fTransform*fResolutionScale;
        fRenderedToImage = true;
        finishedProcessing();
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
    void updateGlobalFromRelBoundingRect() {
        fGlobalBoundingRect = fScaledTransform.mapRect(fRelBoundingRect);
        fixupGlobalBoundingRect();
        fDrawPos = SkPoint::Make(qRound(fGlobalBoundingRect.left()),
                                 qRound(fGlobalBoundingRect.top()));
    }

    void fixupGlobalBoundingRect() {
        for(const QRectF &rectT : fOtherGlobalRects) {
            fGlobalBoundingRect = fGlobalBoundingRect.united(rectT);
        }
        fGlobalBoundingRect.adjust(-fEffectsMargin, -fEffectsMargin,
                                   fEffectsMargin, fEffectsMargin);
        if(fMaxBoundsEnabled) {
            const auto maxBounds = fResolutionScale.mapRect(fMaxBoundsRect);
            fGlobalBoundingRect = fGlobalBoundingRect.intersected(maxBounds);
        }

        const QPointF roundTL(qRound(fGlobalBoundingRect.left()),
                              qRound(fGlobalBoundingRect.top()));
        const QPointF transF = fGlobalBoundingRect.topLeft() - roundTL;
        fGlobalBoundingRect.translate(-transF);
    }

    QList<stdsptr<RenderDataCustomizerFunctor>> mRenderDataCustomizerFunctors;
    bool mDelayDataSet = false;
    bool mDataSet = false;
};

class RenderDataCustomizerFunctor : public StdSelfRef {
public:
    RenderDataCustomizerFunctor();
    virtual void customize(BoundingBoxRenderData * const data) = 0;
    void operator()(BoundingBoxRenderData * const data);
};

class ReplaceTransformDisplacementCustomizer : public RenderDataCustomizerFunctor {
public:
    ReplaceTransformDisplacementCustomizer(const qreal &dx,
                                           const qreal &dy);

    void customize(BoundingBoxRenderData * const data);
protected:
    qreal mDx, mDy;
};

class MultiplyTransformCustomizer : public RenderDataCustomizerFunctor {
public:
    MultiplyTransformCustomizer(const QMatrix &transform,
                                const qreal &opacity = 1);

    void customize(BoundingBoxRenderData * const data);
protected:
    QMatrix mTransform;
    qreal mOpacity = 1;
};

class MultiplyOpacityCustomizer : public RenderDataCustomizerFunctor {
public:
    MultiplyOpacityCustomizer(const qreal &opacity);

    void customize(BoundingBoxRenderData * const data);
protected:
    qreal mOpacity;
};

#endif // BOUNDINGBOXRENDERDATA_H
