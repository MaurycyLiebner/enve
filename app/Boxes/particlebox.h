#ifndef PARTICLEBOX_H
#define PARTICLEBOX_H
#include "boundingbox.h"
#include "Properties/boxtargetproperty.h"
#include "Animators/coloranimator.h"
#include "Animators/qpointfanimator.h"
#include "Animators/dynamiccomplexanimator.h"
class QPointFAnimator;
class AnimatedPoint;
class ParticleBox;

struct ParticleState {
    ParticleState() {}

    ParticleState(const SkPoint &posT,
                  const SkScalar scaleT,
                  const SkScalar sizeT,
                  const unsigned char &opacityT,
                  const SkPath &path) {
        fPos = posT;
        fSize = scaleT*sizeT;
        fOpacity = opacityT;

        fLinePath = path;
    }

    static ParticleState interpolate(const ParticleState &state1,
                                     const ParticleState &state2,
                                     const SkScalar weight2) {
        SkScalar weight1 = 1.f - weight2;
        SkPath pathT;
        if(state2.fLinePath.isEmpty()) {
            pathT = state1.fLinePath;
        } else if(state1.fLinePath.isEmpty()) {
            pathT = state2.fLinePath;
        } else {
            state1.fLinePath.interpolate(state2.fLinePath,
                                        1.f - weight2, &pathT);
        }
        uchar opacity1 = state1.fOpacity;
        uchar opacity2 = state2.fOpacity;
        int iOpacity = qRound(opacity1*weight1 + opacity2*weight2);
        uchar opacityT = static_cast<uchar>(qMax(0, qMin(255, iOpacity)));
        return ParticleState(state1.fPos*weight1 + state2.fPos*weight2, 1.,
                             state1.fSize*weight1 + state2.fSize*weight2,
                             opacityT, pathT);
    }

    void drawSk(SkCanvas * const canvas,
                const SkPaint paint) const {
        if(fSize < 0.f) return;
        SkPaint paintT = paint;
        if(fTargetRenderData.get() == nullptr) {
            paintT.setAlpha(fOpacity);
            paintT.setStrokeWidth(fSize);
            canvas->drawPath(fLinePath, paintT);
        } else {
            int iAlpha = qRound(fTargetRenderData->fOpacity*2.55);
            paintT.setAlpha(static_cast<U8CPU>(iAlpha));
            sk_sp<SkImage> imageT = fTargetRenderData->fRenderedImage;
            if(imageT.get() == nullptr) return;
            //paintT.setAntiAlias(true);
            //paintT.setFilterQuality(kHigh_SkFilterQuality);
            SkScalar drawX = fPos.x() - imageT->width()*0.5f;
            SkScalar drawY = fPos.y() - imageT->height()*0.5f;
            canvas->drawImage(imageT, drawX, drawY, &paintT);
        }
    }

    stdsptr<BoundingBoxRenderData> fTargetRenderData;
    SkPath fLinePath;
    SkPoint fPos;
    SkScalar fSize;
    unsigned char fOpacity;
};

struct EmitterData {
    void drawParticles(SkCanvas *canvas) const {
        canvas->save();
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(color);
        paint.setStrokeCap(SkPaint::kRound_Cap);
        paint.setStyle(SkPaint::kStroke_Style);
        for(const ParticleState &state : particleStates) {
            state.drawSk(canvas, paint);
        }
        canvas->restore();
    }

    bool boxDraw = false;
    SkColor color;
    QList<ParticleState> particleStates;
};

struct ParticleBoxRenderData : public BoundingBoxRenderData {
    ParticleBoxRenderData(BoundingBox *parentBoxT) :
        BoundingBoxRenderData(parentBoxT) {

    }

    void updateRelBoundingRect() {
        BoundingBoxRenderData::updateRelBoundingRect();
        fClipRect = toSkRect(fRelBoundingRect);
    }

    QList<EmitterData> fEmittersData;
    SkRect fClipRect;
private:
    void drawSk(SkCanvas * const canvas) {
        canvas->save();
        canvas->clipRect(fClipRect);
        for(const EmitterData &emitterData : fEmittersData) {
            if(emitterData.boxDraw) {
                canvas->save();
                canvas->resetMatrix();
                canvas->translate(toSkScalar(-fGlobalBoundingRect.left()),
                                  toSkScalar(-fGlobalBoundingRect.top()));
                QMatrix scale;
                scale.scale(fResolution, fResolution);
                canvas->concat(toSkMatrix(scale));
                emitterData.drawParticles(canvas);
                canvas->restore();
            } else {
                emitterData.drawParticles(canvas);
            }
        }

        canvas->restore();
    }
};

class Particle {
public:
    Particle();
    void initializeParticle(const int firstFrame,
                            const int nFrames,
                            const SkPoint &iniPos,
                            const SkPoint &iniVel,
                            const SkScalar partSize);
    void generatePathNextFrame(const int frame,
                               const SkScalar velocityVar,
                               const SkScalar velocityVarPeriod,
                               const SkPoint &acc,
                               const SkScalar finalScale,
                               const SkScalar finalOpacity,
                               const SkScalar decayFrames,
                               const SkScalar length);

    bool isVisibleAtFrame(const int frame);
    ParticleState getParticleStateAtFrame(const int frame);
    bool getParticleStateAtFrameF(const qreal frame, ParticleState &state);
private:
    SkScalar mSize;
    SkPoint mPrevVelocityVar;
    SkPoint mNextVelocityVar;
    SkScalar mPrevVelocityDuration;

    int mFirstFrame;
    int mNumberFrames;
    ParticleState *mParticleStates = nullptr;

    SkScalar mLastScale;
    SkScalar mLastOpacity;
    SkPoint mLastPos;
    SkPoint mLastVel;
};

class ParticleEmitter : public StaticComplexAnimator {
public:
    ParticleEmitter();

    void scheduleGenerateParticles();
    void generateParticlesIfNeeded();
    void generateParticles();

    void setMinFrame(const int minFrame);

    void setMaxFrame(const int maxFrame);

    void setFrameRange(const int minFrame,
                       const int maxFrame);

    ColorAnimator *getColorAnimator();
    MovablePoint *getPosPoint();

    EmitterData getEmitterDataAtRelFrameF(
            const qreal relFrame,
            const stdsptr<ParticleBoxRenderData>& particleData);
private:
    bool mGenerateParticlesScheduled = false;
    bool mUpdateParticlesForFrameScheduled = false;
    int mMinFrame = -10;
    int mMaxFrame = 200;

    QList<Particle*> mParticles;
    QList<Particle*> mNotFinishedParticles;

    qsptr<ColorAnimator> mColorAnimator =
            SPtrCreate(ColorAnimator)();

    qsptr<QPointFAnimator> mPos =
            SPtrCreate(QPointFAnimator)("position");

    stdsptr<AnimatedPoint> mPosPoint;

    qsptr<QrealAnimator> mWidth =
            SPtrCreate(QrealAnimator)("width");

    qsptr<QrealAnimator> mSrcVelInfl =
            SPtrCreate(QrealAnimator)("source velocity influence");

    qsptr<QrealAnimator> mIniVelocity =
            SPtrCreate(QrealAnimator)("initial velocity");
    qsptr<QrealAnimator> mIniVelocityVar =
            SPtrCreate(QrealAnimator)("initial velocity variation");
    qsptr<QrealAnimator> mIniVelocityAngle =
            SPtrCreate(QrealAnimator)("initial velocity angle");
    qsptr<QrealAnimator> mIniVelocityAngleVar =
            SPtrCreate(QrealAnimator)("initial velocity angle variation");
    qsptr<QPointFAnimator> mAcceleration =
            SPtrCreate(QPointFAnimator)("acceleration");

    qsptr<QrealAnimator> mParticlesPerSecond =
            SPtrCreate(QrealAnimator)("particles per second");
    qsptr<QrealAnimator> mParticlesFrameLifetime =
            SPtrCreate(QrealAnimator)("particle lifetime");

    qsptr<QrealAnimator> mVelocityRandomVar =
            SPtrCreate(QrealAnimator)("velocity random variation");
    qsptr<QrealAnimator> mVelocityRandomVarPeriod =
            SPtrCreate(QrealAnimator)("velocity variation period");

    qsptr<QrealAnimator> mParticleSize =
            SPtrCreate(QrealAnimator)("particle size");
    qsptr<QrealAnimator> mParticleSizeVar =
            SPtrCreate(QrealAnimator)("particle size variation");

    qsptr<QrealAnimator> mParticleLength =
            SPtrCreate(QrealAnimator)("particle length");

    qsptr<QrealAnimator> mParticlesDecayFrames =
            SPtrCreate(QrealAnimator)("decay frames");
    qsptr<QrealAnimator> mParticlesSizeDecay =
            SPtrCreate(QrealAnimator)("final scale");
    qsptr<QrealAnimator> mParticlesOpacityDecay =
            SPtrCreate(QrealAnimator)("final opacity");

    qsptr<BoxTargetProperty> mBoxTargetProperty =
            SPtrCreate(BoxTargetProperty)("source");
};

class ParticleBox : public BoundingBox {
public:
    ParticleBox();
    void getAccelerationAt(const QPointF &pos,
                           const int frame,
                           QPointF *acc);
    void anim_setAbsFrame(const int frame);
    bool relPointInsidePath(const QPointF &relPos) const;

    void addEmitter(const qsptr<ParticleEmitter> &emitter);
    void startAllPointsTransform();

    void applyPaintSetting(const PaintSettingsApplier &setting);
    MovablePoint *getBottomRightPoint();
    void addEmitterAtAbsPos(const QPointF &absPos);

    bool SWT_isParticleBox() const;

    stdsptr<BoundingBoxRenderData> createRenderData() {
        return SPtrCreate(ParticleBoxRenderData)(this);
    }

    void setupRenderData(const qreal relFrame,
                         BoundingBoxRenderData * const data) {
        BoundingBox::setupRenderData(relFrame, data);
        auto particleData = GetAsSPtr(data, ParticleBoxRenderData);
        particleData->fEmittersData.clear();
        for(const auto& emitter : mEmitters) {
            emitter->generateParticlesIfNeeded();
            particleData->fEmittersData << emitter->getEmitterDataAtRelFrameF(
                                              relFrame, particleData);
        }
    }

    QRectF getRelBoundingRect(const qreal relFrame);

    void removeEmitter(const qsptr<ParticleEmitter> &emitter);

    FrameRange prp_getIdenticalRelRange(const int relFrame) const;
    void updateAfterDurationRectangleRangeChanged();
private:
    qsptr<QPointFAnimator> mTopLeftAnimator;
    qsptr<QPointFAnimator> mBottomRightAnimator;

    stdsptr<AnimatedPoint> mTopLeftPoint;
    stdsptr<AnimatedPoint> mBottomRightPoint;
    stdsptr<DynamicComplexAnimator<ParticleEmitter>> mEmitterCol;
    QList<qsptr<ParticleEmitter>> mEmitters;
};

#endif // PARTICLEBOX_H
