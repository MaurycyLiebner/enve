#ifndef BOUNDINGBOXRENDERDATA_H
#define BOUNDINGBOXRENDERDATA_H
#include "skiaincludes.h"

#include <QWeakPointer>
#include "updatable.h"
#include "Animators/animator.h"
#include <QMatrix>
struct PixmapEffectRenderData;
class BoundingBox;
#include "sharedpointerdefs.h"

class RenderDataCustomizerFunctor;
struct BoundingBoxRenderData : public _ScheduledExecutor {
    friend class StdSelfRef;
    BoundingBoxRenderData(BoundingBox *parentBoxT);

    virtual ~BoundingBoxRenderData();

    virtual void copyFrom(BoundingBoxRenderData *src);
    bool copied = false;

    bool relBoundingRectSet = false;

    Animator::UpdateReason reason;

    BoundingBoxRenderDataSPtr makeCopy();

    bool redo = false;

    bool renderedToImage = false;
    QMatrix transform;
    QMatrix parentTransform;
    QMatrix relTransform;
    QRectF relBoundingRect;
    QRectF globalBoundingRect;
    qreal opacity = 1.;
    qreal resolution;
    qreal effectsMargin;
    int relFrame;

    // for motion blur
    bool useCustomRelFrame = false;
    qreal customRelFrame;
    QList<QRectF> otherGlobalRects;
    BoundingBoxRenderDataPtr motionBlurTarget;
    // for motion blur

    QList<PixmapEffectRenderDataSPtr> pixmapEffects;
    SkPoint drawPos = SkPoint::Make(0.f, 0.f);
    SkBlendMode blendMode = SkBlendMode::kSrcOver;
    QRectF maxBoundsRect;
    bool maxBoundsEnabled = true;

    bool parentIsTarget = true;
    BoundingBoxQPtr parentBox;

    virtual void updateRelBoundingRect();
    void drawRenderedImageForParent(SkCanvas *canvas);
    virtual void renderToImage();
    sk_sp<SkImage> renderedImage;

    void _processUpdate();

    void beforeUpdate();

    void afterUpdate();

    void schedulerProccessed();

    virtual bool allDataReady() { return true; }

    void dataSet();

    void clearPixmapEffects() {
        pixmapEffects.clear();
        effectsMargin = 0.;
    }

    virtual QPointF getCenterPosition() {
        return relBoundingRect.center();
    }

    void appendRenderCustomizerFunctor(
            const RenderDataCustomizerFunctorSPtr& customizer) {
        mRenderDataCustomizerFunctors.append(customizer);
    }

    void prependRenderCustomizerFunctor(
            const RenderDataCustomizerFunctorSPtr& customizer) {
        mRenderDataCustomizerFunctors.prepend(customizer);
    }

    void parentBeingProcessed();
protected:
    void addSchedulerNow();
    QList<RenderDataCustomizerFunctorSPtr> mRenderDataCustomizerFunctors;
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
