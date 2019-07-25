#ifndef BOXRENDERDATA_H
#define BOXRENDERDATA_H
#include "skia/skiaincludes.h"

#include <QWeakPointer>
#include "updatable.h"
#include "Animators/animator.h"
#include <QMatrix>
struct PixmapEffectRenderData;
class BoundingBox;
class ShaderProgramCallerBase;
#include "smartPointers/sharedpointerdefs.h"
#include "effectsrenderer.h"

class RenderDataCustomizerFunctor;
struct BoxRenderData : public Task {
    friend class StdSelfRef;
protected:
    enum class Step { BOX_IMAGE, EFFECTS };

    BoxRenderData(BoundingBox *parentBoxT);

    virtual void drawSk(SkCanvas * const canvas) = 0;
    virtual void setupRenderData() {}
    virtual void transformRenderCanvas(SkCanvas& canvas) const;
    virtual void copyFrom(BoxRenderData *src);
    virtual void updateRelBoundingRect();

    HardwareSupport hardwareSupport() const;

    void scheduleTaskNow() final;
    void afterCanceled();
    void beforeProcessing() final;
    void afterProcessing() final;
    void afterQued() final;

    virtual SkColor eraseColor() const { return SK_ColorTRANSPARENT; }
public:
    virtual QPointF getCenterPosition() {
        return fRelBoundingRect.center();
    }

    bool nextStep() {
        const bool result = !mEffectsRenderer.isEmpty();
        if(result) mStep = Step::EFFECTS;
        return result;
    }

    void processGPU(QGL33 * const gl,
                    SwitchableContext &context);
    void process();


    stdsptr<BoxRenderData> makeCopy();

    bool fCopied = false;
    bool fRelBoundingRectSet = false;

    Animator::UpdateReason fReason;

    uint fBoxStateId = 0;

    QMatrix fResolutionScale;
    QMatrix fScaledTransform;
    QMatrix fTransform;
    QMatrix fRenderTransform;

    QRectF fRelBoundingRect;
    QRect fGlobalRect;
    QRect fMaxBoundsRect;

    QMargins fBaseMargin;

    qreal fOpacity = 1;
    qreal fResolution;
    int fRelFrame;

    // for motion blur
    bool fUseCustomRelFrame = false;
    qreal fCustomRelFrame;
    QList<QRectF> fOtherGlobalRects;
    stdptr<BoxRenderData> fMotionBlurTarget;
    // for motion blur

    SkBlendMode fBlendMode = SkBlendMode::kSrcOver;

    bool fParentIsTarget = true;
    bool fRefInParent = false;
    qptr<BoundingBox> fParentBox;
    sk_sp<SkImage> fRenderedImage;

    void drawRenderedImageForParent(SkCanvas * const canvas);

    void dataSet();

    void appendRenderCustomizerFunctor(
            const stdsptr<RenderDataCustomizerFunctor>& customizer) {
        mRenderDataCustomizerFunctors.append(customizer);
    }

    void prependRenderCustomizerFunctor(
            const stdsptr<RenderDataCustomizerFunctor>& customizer) {
        mRenderDataCustomizerFunctors.prepend(customizer);
    }
    bool nullifyBeforeProcessing();

    void addEffect(const stdsptr<RasterEffectCaller>& effect) {
        mEffectsRenderer.add(effect);
    }
protected:
    virtual void updateGlobalRect();
    bool hasEffects() const { return !mEffectsRenderer.isEmpty(); }

    void setBaseGlobalRect(const QRectF &baseRectF);

    QList<stdsptr<RenderDataCustomizerFunctor>> mRenderDataCustomizerFunctors;
    bool mDelayDataSet = false;
    bool mDataSet = false;
private:
    Step mStep = Step::BOX_IMAGE;
    EffectsRenderer mEffectsRenderer;
};

class RenderDataCustomizerFunctor : public StdSelfRef {
public:
    RenderDataCustomizerFunctor();
    virtual void customize(BoxRenderData * const data) = 0;
    void operator()(BoxRenderData * const data);
};

class ReplaceTransformDisplacementCustomizer : public RenderDataCustomizerFunctor {
public:
    ReplaceTransformDisplacementCustomizer(const qreal dx,
                                           const qreal dy);

    void customize(BoxRenderData * const data);
protected:
    qreal mDx, mDy;
};

class MultiplyTransformCustomizer : public RenderDataCustomizerFunctor {
public:
    MultiplyTransformCustomizer(const QMatrix &transform,
                                const qreal opacity = 1);

    void customize(BoxRenderData * const data);
protected:
    QMatrix mTransform;
    qreal mOpacity = 1;
};

class MultiplyOpacityCustomizer : public RenderDataCustomizerFunctor {
public:
    MultiplyOpacityCustomizer(const qreal opacity);

    void customize(BoxRenderData * const data);
protected:
    qreal mOpacity;
};

#endif // BOXRENDERDATA_H
