#ifndef PARTICLEBOX_H
#define PARTICLEBOX_H
#include "boundingbox.h"
#include "Properties/boxtargetproperty.h"
class PointAnimator;

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
        if(targetRenderData.get() == NULL) {
            SkPaint paintT = paint;
            paintT.setAlpha(opacity);
            paintT.setStrokeWidth(size);
            canvas->drawPath(linePath, paintT);
        } else {
            targetRenderData->renderToImage();
            targetRenderData->drawRenderedImageForParent(canvas);
        }
    }

    std::shared_ptr<BoundingBoxRenderData> targetRenderData;
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

    void scheduleGenerateParticles();
    void generateParticlesIfNeeded();
    void generateParticles();

    void setMinFrame(const int &minFrame);

    void setMaxFrame(const int &maxFrame);

    void setFrameRange(const int &minFrame,
                       const int &maxFrame);

    ColorAnimator *getColorAnimator();
    MovablePoint *getPosPoint();

    EmitterData getEmitterDataAtRelFrame(const int &relFrame,
                                         ParticleBoxRenderData *particleData) {
        EmitterData data;
        data.color = mColorAnimator->getColorAtRelFrame(relFrame).getSkColor();

        BoundingBox *targetT = mBoxTargetProperty->getTarget();
        if(targetT == NULL) {
            Q_FOREACH(Particle *particle, mParticles) {
                if(particle->isVisibleAtFrame(relFrame)) {
                    data.particleStates <<
                        particle->getParticleStateAtFrame(relFrame);
                }
            }
        } else {
            int targetRelFrame = targetT->prp_absFrameToRelFrame(
                        prp_relFrameToAbsFrame(relFrame));
            Q_FOREACH(Particle *particle, mParticles) {
                if(particle->isVisibleAtFrame(relFrame)) {
                    ParticleState stateT = particle->getParticleStateAtFrame(relFrame);
                    BoundingBoxRenderData *renderData = targetT->createRenderData();
                    targetT->setupBoundingBoxRenderDataForRelFrame(
                        targetRelFrame, renderData);
                    QMatrix transformT = renderData->transform;
                    transformT.setMatrix(transformT.m11()*stateT.size, transformT.m12(),
                                         transformT.m21(), transformT.m22()*stateT.size,
                                         stateT.pos.x(), stateT.pos.y());
                    renderData->transform = transformT*particleData->transform;
                    renderData->opacity = ((int)stateT.opacity)*
                            renderData->opacity/255;

                    stateT.targetRenderData =
                            renderData->ref<BoundingBoxRenderData>();
                    data.particleStates << stateT;
                    renderData->dataSet();
                }
            }
        }

        return data;
    }

    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
private:
    bool mGenerateParticlesScheduled = false;
    bool mUpdateParticlesForFrameScheduled = false;
    int mMinFrame = -10;
    int mMaxFrame = 200;

    QList<Particle*> mParticles;
    QList<Particle*> mNotFinishedParticles;
    ParticleBox *mParentBox = NULL;

    QSharedPointer<ColorAnimator> mColorAnimator =
            (new ColorAnimator())->ref<ColorAnimator>();

    QSharedPointer<PointAnimator> mPos;
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

    QSharedPointer<BoxTargetProperty> mBoxTargetProperty =
            (new BoxTargetProperty())->ref<BoxTargetProperty>();
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

    void addEmitter(ParticleEmitter *emitter);
    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);
    void startAllPointsTransform();

    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode,
                             const qreal &canvasScaleInv);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<MovablePoint *> *list);
    void applyPaintSetting(const PaintSetting &setting);
    MovablePoint *getBottomRightPoint();
    void addEmitterAtAbsPos(const QPointF &absPos);

    bool SWT_isParticleBox();

    BoundingBoxRenderData *createRenderData() {
        return new ParticleBoxRenderData(this);
    }

    void setupBoundingBoxRenderDataForRelFrame(const int &relFrame,
                                               BoundingBoxRenderData *data) {
        BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame, data);
        ParticleBoxRenderData *particleData = (ParticleBoxRenderData*)data;
        particleData->emittersData.clear();
        foreach(ParticleEmitter *emitter, mEmitters) {
            emitter->generateParticlesIfNeeded();
            particleData->emittersData << emitter->getEmitterDataAtRelFrame(
                                              relFrame, particleData);
        }
    }

    QRectF getRelBoundingRectAtRelFrame(const int &relFrame);

    void removeEmitter(ParticleEmitter *emitter);

    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                               int *lastIdentical,
                                              const int &relFrame);
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
public slots:
    void updateAfterDurationRectangleRangeChanged();
private:
    PointAnimator *mTopLeftPoint;
    PointAnimator *mBottomRightPoint;
    QList<ParticleEmitter*> mEmitters;
};

#endif // PARTICLEBOX_H
