#ifndef PARTICLEBOX_H
#define PARTICLEBOX_H
#include "boundingbox.h"

class ParticleBox;

struct ParticleState {
    ParticleState() {}
    ParticleState(const QPointF &posT,
                  const qreal &scaleT,
                  const qreal &sizeT,
                  const qreal &opacityT) {
        pos = posT;
        size = scaleT*sizeT*0.5;
        opacity = opacityT;
        isLine = false;
    }
    ParticleState(const QPointF &posT,
              const qreal &scaleT,
              const qreal &sizeT,
              const qreal &opacityT,
              const QPainterPath &path) {
//        QPainterPathStroker stroker;
//        stroker.setWidth(radius);
//        stroker.setCapStyle(Qt::RoundCap);
//        linePath = stroker.createStroke(path);
        pos = posT;
        size = scaleT*sizeT;
        opacity = opacityT;
        isLine = true;

        linePath = path;
    }

    void draw(QPainter *p) const {
        if(size < 0.) return;
        p->setOpacity(opacity);

        if(isLine) {
            QPen pen = p->pen();
            pen.setWidthF(size);
            p->setPen(pen);
            p->drawPath(linePath);
        } else {
            p->drawEllipse(pos, size, size);
        }
    }

    bool isLine;
    QPainterPath linePath;
    QPointF pos;
    qreal size;
    qreal opacity;
};

class Particle {
public:
    Particle(ParticleBox *parentBox);
    void initializeParticle(const int &firstFrame,
                            const int &nFrames,
                            const QPointF &iniPos,
                            const QPointF &iniVel,
                            const qreal &partSize);
    void generatePathNextFrame(const int &frame,
                               const qreal &velocityVar,
                               const qreal &velocityVarPeriod,
                               const QPointF &acc,
                               const qreal &finalScale,
                               const qreal &finalOpacity,
                               const qreal &decayFrames,
                               const qreal &length);

    bool isVisibleAtFrame(const int &frame);
    ParticleState getParticleStateAtFrame(const int &frame);
private:
    qreal mSize;
    QPointF mPrevVelocityVar;
    QPointF mNextVelocityVar;
    qreal mPrevVelocityDuration;

    ParticleBox *mParentBox;
    int mFirstFrame;
    int mNumberFrames;
    ParticleState *mParticleStates = NULL;

    qreal mLastScale;
    qreal mLastOpacity;
    QPointF mLastPos;
    QPointF mLastVel;
};

class ParticleEmitter : public ComplexAnimator {
public:
    ParticleEmitter(ParticleBox *parentBox);

    void setParentBox(ParticleBox *parentBox);

    void generateParticles();

    void drawParticles(QPainter *p);
    void updateParticlesForFrame(const int &frame);
    QRectF getParticlesBoundingRect();
    void scheduleGenerateParticles();
    void scheduleUpdateParticlesForFrame();
    void generateParticlesIfNeeded();
    void updateParticlesForFrameIfNeeded(const int &frame);
    bool relPointInsidePath(const QPointF &relPos) {
        foreach(const ParticleState &state, mParticleStates) {
            if(pointToLen(state.pos - relPos) < 5.) {
                return true;
            }
        }

        return false;
    }

    Property *makeDuplicate() {
        ParticleEmitter *emitterDupli = new ParticleEmitter(mParentBox);
        makeDuplicate(emitterDupli);
        return emitterDupli;
    }

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

    void setMinFrame(const int &minFrame) {
        mMinFrame = minFrame;
        scheduleGenerateParticles();
    }

    void setMaxFrame(const int &maxFrame) {
        mMaxFrame = maxFrame;
        scheduleGenerateParticles();
    }

    void setFrameRange(const int &minFrame,
                       const int &maxFrame);

    ColorAnimator *getColorAnimator();
    MovablePoint *getPosPoint() {
        return mPos.data();
    }
private:
    QRectF mParticlesBoundingRect;
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
    void updateAfterFrameChanged(const int &currentFrame);
    void draw(QPainter *p);
    void updateRelBoundingRect();
    void preUpdatePixmapsUpdates();
    bool relPointInsidePath(const QPointF &relPos);

    void addEmitter(ParticleEmitter *emitter);

    BoundingBox *createNewDuplicate(BoxesGroup *parent) {
        return new ParticleBox(parent);
    }

    void makeDuplicate(Property *targetBox) {
        BoundingBox::makeDuplicate(targetBox);
        ParticleBox *pbTarget = (ParticleBox*)targetBox;
        foreach(ParticleEmitter *emitter, mEmitters) {
            pbTarget->addEmitter((ParticleEmitter*)
                                 emitter->makeDuplicate());
        }
    }

    void startAllPointsTransform();
    void drawSelected(QPainter *p,
                      const CanvasMode &currentCanvasMode);
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode,
                             const qreal &canvasScaleInv);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<MovablePoint *> *list);
    void applyPaintSetting(const PaintSetting &setting);
    MovablePoint *getBottomRightPoint();
    void addEmitterAtAbsPos(const QPointF &absPos);

    void prp_setAbsFrame(const int &frame) {
        BoundingBox::prp_setAbsFrame(frame);
        mFrameChangedUpdateScheduled = true;
    }

    void setUpdateVars() {
        BoundingBox::setUpdateVars();
        if(mFrameChangedUpdateScheduled) {
            mFrameChangedUpdateScheduled = false;
            foreach(ParticleEmitter *emitter, mEmitters) {
                emitter->scheduleUpdateParticlesForFrame();
            }
        }
    }

    bool SWT_isParticleBox() { return true; }
public slots:
    void updateAfterDurationRectangleRangeChanged();
private:
    bool mFrameChangedUpdateScheduled = false;
    MovablePoint *mTopLeftPoint;
    MovablePoint *mBottomRightPoint;
    QList<ParticleEmitter*> mEmitters;
};

#endif // PARTICLEBOX_H
