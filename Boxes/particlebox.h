#ifndef PARTICLEBOX_H
#define PARTICLEBOX_H
#include "boundingbox.h"

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

    void drawSk(SkCanvas *canvas,
                const SkPaint paint) const {
        if(size < 0.) return;
        SkPaint paintT = paint;
        paintT.setAlpha(opacity);
        paintT.setStrokeWidth(size);
        canvas->drawPath(linePath, paintT);
    }

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

    SkColor color;
    QList<ParticleState> particleStates;
};

struct ParticleBoxRenderData : public BoundingBoxRenderData {
    QList<EmitterData> emittersData;
    SkRect clipRect;
private:
    void drawSk(SkCanvas *canvas) {
        canvas->save();

        canvas->clipRect(clipRect);
        Q_FOREACH(const EmitterData &emitterData, emittersData) {
            emitterData.drawParticles(canvas);
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
private:
    SkScalar mSize;
    SkPoint mPrevVelocityVar;
    SkPoint mNextVelocityVar;
    SkScalar mPrevVelocityDuration;

    ParticleBox *mParentBox;
    int mFirstFrame;
    int mNumberFrames;
    ParticleState *mParticleStates = NULL;

    SkScalar mLastScale;
    SkScalar mLastOpacity;
    SkPoint mLastPos;
    SkPoint mLastVel;
};

class ParticleEmitter : public ComplexAnimator {
public:
    ParticleEmitter(ParticleBox *parentBox);

    void setParentBox(ParticleBox *parentBox);

    void generateParticles();

    void drawParticlesSk(SkCanvas *canvas);
    void updateParticlesForFrame(const int &frame);
    void scheduleGenerateParticles();
    void scheduleUpdateParticlesForFrame();
    void generateParticlesIfNeeded();
    void updateParticlesForFrameIfNeeded(const int &frame);
    bool relPointInsidePath(const SkPoint &relPos);

    Property *makeDuplicate();

    void duplicateAnimatorsFrom(QPointFAnimator *pos,
            QrealAnimator *width,
            QrealAnimator *srcVelInfl,

            QrealAnimator *iniVelocity,
            QrealAnimator *iniVelocityVar,
            QrealAnimator *iniVelocityAngle,
            QrealAnimator *iniVelocityAngleVar,
            QPointFAnimator *acceleration,

            QrealAnimator *particlesPerSecond,
            QrealAnimator *particlesFrameLifetime,

            QrealAnimator *velocityRandomVar,
            QrealAnimator *velocityRandomVarPeriod,

            QrealAnimator *particleSize,
            QrealAnimator *particleSizeVar,

            QrealAnimator *particleLength,

            QrealAnimator *particlesDecayFrames,
            QrealAnimator *particlesSizeDecay,
            QrealAnimator *particlesOpacityDecay);

    void makeDuplicate(Property *target);

    void setMinFrame(const int &minFrame);

    void setMaxFrame(const int &maxFrame);

    void setFrameRange(const int &minFrame,
                       const int &maxFrame);

    ColorAnimator *getColorAnimator();
    MovablePoint *getPosPoint();

    EmitterData getEmitterDataAtRelFrame(const int &relFrame) {
        EmitterData data;
        data.color = mColorAnimator->getColorAtRelFrame(relFrame).getSkColor();

        Q_FOREACH(Particle *particle, mParticles) {
            if(particle->isVisibleAtFrame(relFrame)) {
                data.particleStates <<
                    particle->getParticleStateAtFrame(relFrame);
            }
        }

        return data;
    }

private:
    bool mGenerateParticlesScheduled = false;
    bool mUpdateParticlesForFrameScheduled = false;
    int mMinFrame = -10;
    int mMaxFrame = 200;

    QList<ParticleState> mParticleStates;
    QList<Particle*> mParticles;
    QList<Particle*> mNotFinishedParticles;
    ParticleBox *mParentBox = NULL;

    QSharedPointer<ColorAnimator> mColorAnimator =
            (new ColorAnimator())->ref<ColorAnimator>();

    QSharedPointer<MovablePoint> mPos;
    QSharedPointer<QrealAnimator> mWidth =
            (new QrealAnimator())->ref<QrealAnimator>();

    QSharedPointer<QrealAnimator> mSrcVelInfl =
            (new QrealAnimator())->ref<QrealAnimator>();

    QSharedPointer<QrealAnimator> mIniVelocity =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mIniVelocityVar =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mIniVelocityAngle =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mIniVelocityAngleVar =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QPointFAnimator> mAcceleration =
            (new QPointFAnimator())->ref<QPointFAnimator>();

    QSharedPointer<QrealAnimator> mParticlesPerSecond =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mParticlesFrameLifetime =
            (new QrealAnimator())->ref<QrealAnimator>();

    QSharedPointer<QrealAnimator> mVelocityRandomVar =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mVelocityRandomVarPeriod =
            (new QrealAnimator())->ref<QrealAnimator>();

    QSharedPointer<QrealAnimator> mParticleSize =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mParticleSizeVar =
            (new QrealAnimator())->ref<QrealAnimator>();

    QSharedPointer<QrealAnimator> mParticleLength =
            (new QrealAnimator())->ref<QrealAnimator>();

    QSharedPointer<QrealAnimator> mParticlesDecayFrames =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mParticlesSizeDecay =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mParticlesOpacityDecay =
            (new QrealAnimator())->ref<QrealAnimator>();
};

class ParticleBox : public BoundingBox {
    Q_OBJECT
public:
    ParticleBox(BoxesGroup *parent);
    void getAccelerationAt(const QPointF &pos,
                           const int &frame,
                           QPointF *acc);
    void prp_setAbsFrame(const int &frame);
    void updateRelBoundingRect();
    bool relPointInsidePath(const QPointF &relPos);

    void addEmitter(ParticleEmitter *emitter);
    void drawSk(SkCanvas *canvas);
    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);

    BoundingBox *createNewDuplicate(BoxesGroup *parent);

    void makeDuplicate(Property *targetBox);

    void startAllPointsTransform();

    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode,
                             const qreal &canvasScaleInv);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<MovablePoint *> *list);
    void applyPaintSetting(const PaintSetting &setting);
    MovablePoint *getBottomRightPoint();
    void addEmitterAtAbsPos(const QPointF &absPos);

    void setUpdateVars();

    bool SWT_isParticleBox();

    BoundingBoxRenderData *createRenderData() {
        return new ParticleBoxRenderData();
    }

    void setupBoundingBoxRenderDataForRelFrame(const int &relFrame,
                                               BoundingBoxRenderData *data) {
        BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame, data);
        ParticleBoxRenderData *particleData = (ParticleBoxRenderData*)data;
        particleData->emittersData.clear();
        foreach(ParticleEmitter *emitter, mEmitters) {
            particleData->emittersData << emitter->getEmitterDataAtRelFrame(
                                              relFrame);
        }
        particleData->relBoundingRect = getBoundingRectAtRelFrame(relFrame);

        particleData->clipRect = QRectFToSkRect(particleData->relBoundingRect);
    }

    QRectF getBoundingRectAtRelFrame(const int &relFrame) {
        return QRectF(mTopLeftPoint->getRelativePosAtRelFrame(relFrame),
                      mBottomRightPoint->getRelativePosAtRelFrame(relFrame));
    }

    void removeEmitter(ParticleEmitter *emitter);
public slots:
    void updateAfterDurationRectangleRangeChanged();
private:
    bool mFrameChangedUpdateScheduled = false;
    MovablePoint *mTopLeftPoint;
    MovablePoint *mBottomRightPoint;
    QList<ParticleEmitter*> mEmitters;
};

#endif // PARTICLEBOX_H
