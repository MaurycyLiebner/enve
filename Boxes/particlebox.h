#ifndef PARTICLEBOX_H
#define PARTICLEBOX_H
#include "boundingbox.h"
#include "Properties/boxtargetproperty.h"
#include "Animators/coloranimator.h"
class QPointFAnimator;

class ParticleBox;

struct ParticleState {
    ParticleState() {}

    ParticleState(const SkPoint &posT,
                  const SkScalar &scaleT,
                  const SkScalar &sizeT,
                  const unsigned char &opacityT,
                  const SkPath &path) {
        pos = posT;
        size = scaleT*sizeT;
        opacity = opacityT;

        linePath = path;
    }

    static ParticleState interpolate(const ParticleState &state1,
                                     const ParticleState &state2,
                                     const SkScalar &weight2) {
        SkScalar weight1 = 1.f - weight2;
        SkPath pathT;
        if(state2.linePath.isEmpty()) {
            pathT = state1.linePath;
        } else if(state1.linePath.isEmpty()) {
            pathT = state2.linePath;
        } else {
            state1.linePath.interpolate(state2.linePath,
                                        1.f - weight2, &pathT);
        }
        uchar opacity1 = state1.opacity;
        uchar opacity2 = state2.opacity;
        int iOpacity = qRound(opacity1*weight1 + opacity2*weight2);
        uchar opacityT = static_cast<uchar>(qMax(0, qMin(255, iOpacity)));
        return ParticleState(state1.pos*weight1 + state2.pos*weight2, 1.,
                             state1.size*weight1 + state2.size*weight2,
                             opacityT, pathT);
    }

    void drawSk(SkCanvas *canvas,
                const SkPaint paint) const {
        if(size < 0.f) return;
        SkPaint paintT = paint;
        if(targetRenderData.get() == nullptr) {
            paintT.setAlpha(opacity);
            paintT.setStrokeWidth(size);
            canvas->drawPath(linePath, paintT);
        } else {
            int iAlpha = qRound(targetRenderData->opacity*2.55);
            paintT.setAlpha(static_cast<U8CPU>(iAlpha));
            sk_sp<SkImage> imageT = targetRenderData->renderedImage;
            if(imageT.get() == nullptr) return;
            //paintT.setAntiAlias(true);
            //paintT.setFilterQuality(kHigh_SkFilterQuality);
            SkScalar drawX = pos.x() - imageT->width()*0.5f;
            SkScalar drawY = pos.y() - imageT->height()*0.5f;
            canvas->drawImage(imageT, drawX, drawY, &paintT);
        }
    }

    BoundingBoxRenderDataSPtr targetRenderData;
    SkPath linePath;
    SkPoint pos;
    SkScalar size;
    unsigned char opacity;
};

struct EmitterData {
    void drawParticles(SkCanvas *canvas) const {
        canvas->save();
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(color);
        paint.setStrokeCap(SkPaint::kRound_Cap);
        paint.setStyle(SkPaint::kStroke_Style);
        Q_FOREACH(const ParticleState &state, particleStates) {
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
        clipRect = QRectFToSkRect(relBoundingRect);
    }

    QList<EmitterData> emittersData;
    SkRect clipRect;
private:
    void drawSk(SkCanvas *canvas) {
        canvas->save();
        canvas->clipRect(clipRect);
        Q_FOREACH(const EmitterData &emitterData, emittersData) {
            if(emitterData.boxDraw) {
                canvas->save();
                canvas->resetMatrix();
                canvas->translate(qrealToSkScalar(-globalBoundingRect.left()),
                                  qrealToSkScalar(-globalBoundingRect.top()));
                QMatrix scale;
                scale.scale(resolution, resolution);
                canvas->concat(QMatrixToSkMatrix(scale));
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
    Particle(ParticleBox *parentBox);
    void initializeParticle(const int &firstFrame,
                            const int &nFrames,
                            const SkPoint &iniPos,
                            const SkPoint &iniVel,
                            const SkScalar &partSize);
    void generatePathNextFrame(const int &frame,
                               const SkScalar &velocityVar,
                               const SkScalar &velocityVarPeriod,
                               const SkPoint &acc,
                               const SkScalar &finalScale,
                               const SkScalar &finalOpacity,
                               const SkScalar &decayFrames,
                               const SkScalar &length);

    bool isVisibleAtFrame(const int &frame);
    ParticleState getParticleStateAtFrame(const int &frame);
    bool getParticleStateAtFrameF(const qreal &frame, ParticleState &state);
private:
    SkScalar mSize;
    SkPoint mPrevVelocityVar;
    SkPoint mNextVelocityVar;
    SkScalar mPrevVelocityDuration;

    ParticleBox *mParentBox;
    int mFirstFrame;
    int mNumberFrames;
    ParticleState *mParticleStates = nullptr;

    SkScalar mLastScale;
    SkScalar mLastOpacity;
    SkPoint mLastPos;
    SkPoint mLastVel;
};

class ParticleEmitter : public ComplexAnimator {
public:
    ParticleEmitter(ParticleBox *parentBox);

    void setParentBox(ParticleBox *parentBox);

    void scheduleGenerateParticles();
    void generateParticlesIfNeeded();
    void generateParticles();

    void setMinFrame(const int &minFrame);

    void setMaxFrame(const int &maxFrame);

    void setFrameRange(const int &minFrame,
                       const int &maxFrame);

    ColorAnimator *getColorAnimator();
    MovablePoint *getPosPoint();

    EmitterData getEmitterDataAtRelFrameF(
            const qreal &relFrame,
            const ParticleBoxRenderDataSPtr& particleData);

    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
private:
    bool mGenerateParticlesScheduled = false;
    bool mUpdateParticlesForFrameScheduled = false;
    int mMinFrame = -10;
    int mMaxFrame = 200;

    QList<Particle*> mParticles;
    QList<Particle*> mNotFinishedParticles;
    ParticleBox* mParentBox_k = nullptr;

    ColorAnimatorQSPtr mColorAnimator =
            SPtrCreate(ColorAnimator)();

    QPointFAnimatorQSPtr mPos =
            SPtrCreate(QPointFAnimator)("position");

    PointAnimatorMovablePointSPtr mPosPoint;

    QrealAnimatorQSPtr mWidth =
            SPtrCreate(QrealAnimator)("width");

    QrealAnimatorQSPtr mSrcVelInfl =
            SPtrCreate(QrealAnimator)("source velocity influence");

    QrealAnimatorQSPtr mIniVelocity =
            SPtrCreate(QrealAnimator)("initial velocity");
    QrealAnimatorQSPtr mIniVelocityVar =
            SPtrCreate(QrealAnimator)("initial velocity variation");
    QrealAnimatorQSPtr mIniVelocityAngle =
            SPtrCreate(QrealAnimator)("initial velocity angle");
    QrealAnimatorQSPtr mIniVelocityAngleVar =
            SPtrCreate(QrealAnimator)("initial velocity angle variation");
    QSharedPointer<QPointFAnimator> mAcceleration =
            SPtrCreate(QPointFAnimator)("acceleration");

    QrealAnimatorQSPtr mParticlesPerSecond =
            SPtrCreate(QrealAnimator)("particles per second");
    QrealAnimatorQSPtr mParticlesFrameLifetime =
            SPtrCreate(QrealAnimator)("particle lifetime");

    QrealAnimatorQSPtr mVelocityRandomVar =
            SPtrCreate(QrealAnimator)("velocity random variation");
    QrealAnimatorQSPtr mVelocityRandomVarPeriod =
            SPtrCreate(QrealAnimator)("velocity variation period");

    QrealAnimatorQSPtr mParticleSize =
            SPtrCreate(QrealAnimator)("particle size");
    QrealAnimatorQSPtr mParticleSizeVar =
            SPtrCreate(QrealAnimator)("particle size variation");

    QrealAnimatorQSPtr mParticleLength =
            SPtrCreate(QrealAnimator)("particle length");

    QrealAnimatorQSPtr mParticlesDecayFrames =
            SPtrCreate(QrealAnimator)("decay frames");
    QrealAnimatorQSPtr mParticlesSizeDecay =
            SPtrCreate(QrealAnimator)("final scale");
    QrealAnimatorQSPtr mParticlesOpacityDecay =
            SPtrCreate(QrealAnimator)("final opacity");

    BoxTargetPropertyQSPtr mBoxTargetProperty =
            SPtrCreate(BoxTargetProperty)("source");
};

class ParticleBox : public BoundingBox {
    Q_OBJECT
public:
    ParticleBox();
    void getAccelerationAt(const QPointF &pos,
                           const int &frame,
                           QPointF *acc);
    void prp_setAbsFrame(const int &frame);
    bool relPointInsidePath(const QPointF &relPos);

    void addEmitter(const ParticleEmitterQSPtr &emitter);
    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);
    void startAllPointsTransform();

    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode,
                             const qreal &canvasScaleInv);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<MovablePointPtr> &list);
    void applyPaintSetting(const PaintSetting &setting);
    MovablePoint *getBottomRightPoint();
    void addEmitterAtAbsPos(const QPointF &absPos);

    bool SWT_isParticleBox();

    BoundingBoxRenderDataSPtr createRenderData() {
        return SPtrCreate(ParticleBoxRenderData)(this);
    }

    void setupBoundingBoxRenderDataForRelFrameF(const qreal &relFrame,
                                               BoundingBoxRenderData* data) {
        BoundingBox::setupBoundingBoxRenderDataForRelFrameF(relFrame, data);
        auto particleData = GetAsSPtr(data, ParticleBoxRenderData);
        particleData->emittersData.clear();
        foreach(const ParticleEmitterQSPtr& emitter, mEmitters) {
            emitter->generateParticlesIfNeeded();
            particleData->emittersData << emitter->getEmitterDataAtRelFrameF(
                                              relFrame, particleData);
        }
    }

    QRectF getRelBoundingRectAtRelFrame(const int &relFrame);

    void removeEmitter(const ParticleEmitterQSPtr &emitter);

    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                               int *lastIdentical,
                                              const int &relFrame);
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
public slots:
    void updateAfterDurationRectangleRangeChanged();
private:
    QPointFAnimatorQSPtr mTopLeftAnimator;
    QPointFAnimatorQSPtr mBottomRightAnimator;

    PointAnimatorMovablePointSPtr mTopLeftPoint;
    PointAnimatorMovablePointSPtr mBottomRightPoint;
    QList<ParticleEmitterQSPtr> mEmitters;
};

#endif // PARTICLEBOX_H
