#ifndef BOUNDINGBOXRENDERDATA_H
#define BOUNDINGBOXRENDERDATA_H
#include "skiaincludes.h"

#include <QWeakPointer>
#include "updatable.h"
#include <QMatrix>
class PixmapEffectRenderData;
class BoundingBox;

class RenderDataCustomizerFunctor;
struct BoundingBoxRenderData : public _ScheduledExecutor {
    BoundingBoxRenderData(BoundingBox *parentBoxT);

    virtual ~BoundingBoxRenderData();

    virtual void copyFrom(BoundingBoxRenderData* src);
    bool copied = false;


    std::shared_ptr<BoundingBoxRenderData> makeCopy();

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
    std::shared_ptr<BoundingBoxRenderData> motionBlurTarget;
    // for motion blur

    QList<PixmapEffectRenderData*> pixmapEffects;
    SkPoint drawPos = SkPoint::Make(0.f, 0.f);
    SkBlendMode blendMode = SkBlendMode::kSrcOver;
    QRectF maxBoundsRect;
    bool maxBoundsEnabled = true;

    bool parentIsTarget = true;
    QWeakPointer<BoundingBox> parentBox;

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
        foreach(PixmapEffectRenderData *effect, pixmapEffects) {
            delete effect;
        }
        pixmapEffects.clear();
        effectsMargin = 0.;
    }

    virtual QPointF getCenterPosition() {
        return relBoundingRect.center();
    }

    void appendRenderCustomizerFunctor(RenderDataCustomizerFunctor *customizer) {
        mRenderDataCustomizerFunctors.append(customizer);
    }

    void prependRenderCustomizerFunctor(RenderDataCustomizerFunctor *customizer) {
        mRenderDataCustomizerFunctors.prepend(customizer);
    }

    void parentBeingProcessed();
protected:
    void addSchedulerNow();
    QList<RenderDataCustomizerFunctor*> mRenderDataCustomizerFunctors;
    bool mDelayDataSet = false;
    bool mDataSet = false;
    virtual void drawSk(SkCanvas *canvas) = 0;
};

typedef std::shared_ptr<BoundingBoxRenderData> BoundingBoxRenderDataSPtr;

class RenderDataCustomizerFunctor {
public:
    RenderDataCustomizerFunctor() {}
    virtual ~RenderDataCustomizerFunctor() {}
    virtual void customize(const std::shared_ptr<BoundingBoxRenderData>& data) = 0;
    void operator()(const std::shared_ptr<BoundingBoxRenderData>& data) {
        customize(data);
    }
};

class ReplaceTransformDisplacementCustomizer :
        public RenderDataCustomizerFunctor {
public:
    ReplaceTransformDisplacementCustomizer(const qreal &dx,
                                           const qreal &dy) {
        mDx = dx;
        mDy = dy;
    }

    void customize(const std::shared_ptr<BoundingBoxRenderData>& data) {
        QMatrix transformT = data->transform;
        data->transform.setMatrix(transformT.m11(), transformT.m12(),
                                  transformT.m21(), transformT.m22(),
                                  mDx, mDy);
    }
protected:
    qreal mDx, mDy;
};

class MultiplyTransformCustomizer :
        public RenderDataCustomizerFunctor {
public:
    MultiplyTransformCustomizer(const QMatrix &transform,
                                const qreal &opacity = 1.) {
        mTransform = transform;
        mOpacity = opacity;
    }

    void customize(const std::shared_ptr<BoundingBoxRenderData>& data) {
        data->transform = mTransform*data->transform;
        data->opacity *= mOpacity;
    }
protected:
    QMatrix mTransform;
    qreal mOpacity = 1.;
};

class MultiplyOpacityCustomizer :
        public RenderDataCustomizerFunctor {
public:
    MultiplyOpacityCustomizer(const qreal &opacity) {
        mOpacity = opacity;
    }

    void customize(const std::shared_ptr<BoundingBoxRenderData>& data) {
        data->opacity *= mOpacity;
    }
protected:
    qreal mOpacity;
};

#endif // BOUNDINGBOXRENDERDATA_H
