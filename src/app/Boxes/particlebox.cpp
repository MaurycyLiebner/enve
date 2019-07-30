#include "particlebox.h"
#include "GUI/mainwindow.h"
#include "GUI/Timeline/durationrectangle.h"
#include "canvas.h"
#include "pointhelpers.h"
#include "MovablePoints/animatedpoint.h"
#include "PropertyUpdaters/displayedfillstrokesettingsupdater.h"
#include "PropertyUpdaters/particlesupdater.h"
#include "Animators/paintsettingsanimator.h"
#include "Animators/transformanimator.h"
#include "Animators/gpueffectanimators.h"
#include "paintsettingsapplier.h"

ParticleBox::ParticleBox() : BoundingBox(TYPE_PARTICLES) {
    prp_setName("Particle Box");
    mTopLeftAnimator = SPtrCreate(QPointFAnimator)("top left");
    mTopLeftPoint = SPtrCreate(AnimatedPoint)(
                mTopLeftAnimator.data(),
                mTransformAnimator.data(),
                TYPE_PATH_POINT);
    mTopLeftAnimator->prp_setInheritedUpdater(
                SPtrCreate(DisplayedFillStrokeSettingsUpdater)(this));

    mBottomRightAnimator = SPtrCreate(QPointFAnimator)("bottom right");
    mBottomRightPoint = SPtrCreate(AnimatedPoint)(
                mBottomRightAnimator.data(),
                mTransformAnimator.data(),
                TYPE_PATH_POINT);
    mBottomRightAnimator->prp_setInheritedUpdater(
                SPtrCreate(DisplayedFillStrokeSettingsUpdater)(this));

    ca_prependChildAnimator(mTopLeftAnimator.data(), mGPUEffectsAnimators);
    ca_prependChildAnimator(mBottomRightAnimator.data(), mGPUEffectsAnimators);

    const auto durRect = SPtrCreate(DurationRectangle)(this);
    setDurationRectangle(durRect);
    mDurationRectangleLocked = true;
    durRect->setMaxFrame(200);
    durRect->setMinFrame(-10);

    //addEmitter(new ParticleEmitter(this));
}

void ParticleBox::getAccelerationAt(const QPointF &pos,
                                    const int frame,
                                    QPointF *acc) {
    Q_UNUSED(pos);
    Q_UNUSED(frame);
    *acc = QPointF(0., 9.8)/24.;
}

void ParticleBox::anim_setAbsFrame(const int frame) {
    BoundingBox::anim_setAbsFrame(frame);
    planScheduleUpdate(Animator::FRAME_CHANGE);
}

bool ParticleBox::relPointInsidePath(const QPointF &relPos) const {
    if(mRelBoundingRect.contains(relPos.x(), relPos.y())) {
        /*if(mEmitters.isEmpty()) */return true;
//        for(const auto& emitter : mEmitters) {
//            if(emitter->relPointInsidePath(relPos)) {
//                return true;
//            }
//        }
//        return false;
    } else {
        return false;
    }
}

void ParticleBox::addEmitter(const qsptr<ParticleEmitter>& emitter) {
    mEmitters << emitter;
    mEmitterCol->addChild(emitter);
}

void ParticleBox::removeEmitter(const qsptr<ParticleEmitter>& emitter) {
    mEmitters.removeOne(emitter);
    mEmitterCol->removeChild(emitter);
}

FrameRange ParticleBox::prp_getIdenticalRelRange(const int relFrame) const {
    if(isVisibleAndInDurationRect(relFrame)) {
        return {relFrame, relFrame};
    }
    return BoundingBox::prp_getIdenticalRelRange(relFrame);
}

void ParticleBox::addEmitterAtAbsPos(const QPointF &absPos) {
    const auto emitter = SPtrCreate(ParticleEmitter)();
    emitter->getPosPoint()->setRelativePos(mapAbsPosToRel(absPos));
    addEmitter(emitter);
}

bool ParticleBox::SWT_isParticleBox() const { return true; }

QRectF ParticleBox::getRelBoundingRect(const qreal relFrame) {
    return QRectF(mTopLeftAnimator->getEffectiveValue(relFrame),
                  mBottomRightAnimator->getEffectiveValue(relFrame));
}

void ParticleBox::updateAfterDurationRectangleRangeChanged() {
    const int minFrame = mDurationRectangle->getMinFrameAsRelFrame();
    const int maxFrame = mDurationRectangle->getMaxFrameAsRelFrame();
    for(const auto& emitter : mEmitters) {
        emitter->setFrameRange(minFrame, maxFrame);
    }
}

void ParticleBox::applyPaintSetting(const PaintSettingsApplier &setting) {
//    if(setting.targetsFill()) {
//        for(const auto& emitter : mEmitters) {
//            setting.applyColorSetting(emitter->getColorAnimator());
//        }
//    }
}

MovablePoint *ParticleBox::getBottomRightPoint() {
    return mBottomRightPoint.get();
}

Particle::Particle() {}

void Particle::initializeParticle(const int firstFrame,
                                  const int nFrames,
                                  const SkPoint &iniPos,
                                  const SkPoint &iniVel,
                                  const float partSize) {
    mSize = partSize;
    mPrevVelocityVar = SkPoint::Make(0., 0.);
    mNextVelocityVar = SkPoint::Make(0., 0.);
    mPrevVelocityDuration = 10000000.;
    mLastScale = 1.;
    mLastOpacity = 1.;

    mFirstFrame = firstFrame;
    mLastPos = iniPos;
    mLastVel = iniVel;
    if(mParticleStates) {
        if(nFrames == mNumberFrames) return;
        delete[] mParticleStates;
    }
    mNumberFrames = nFrames;
    mParticleStates = new ParticleState[nFrames];
}

void Particle::generatePathNextFrame(const int frame,
                                     const float velocityVar,
                                     const float velocityVarPeriod,
                                     const SkPoint &acc,
                                     const float finalScale,
                                     const float finalOpacity,
                                     const float decayFrames,
                                     const float length) {
    if(mPrevVelocityDuration > velocityVarPeriod) {
        mPrevVelocityVar = mNextVelocityVar;
        mNextVelocityVar = SkPoint::Make(gRandF(-velocityVar, velocityVar),
                                         gRandF(-velocityVar, velocityVar));
        mPrevVelocityDuration = 0.;
    }

    int arrayId = frame - mFirstFrame;

    if(arrayId == 0) {
        float iniTime = gRandF(0., 1.);
        mLastPos += mLastVel*iniTime;
        mLastVel += acc*iniTime;
    }

    int remaining = mNumberFrames - arrayId;
    if(remaining <= decayFrames) {
        mLastScale += (finalScale - 1.)/decayFrames;
        mLastOpacity += (finalOpacity - 1.)/decayFrames;
    }

    SkPath linePath;
    float currLen = 0.;
    int currId = arrayId - 1;
    SkPoint lastPos = mLastPos;
    linePath.moveTo(lastPos);
    while(currId > -1) {
        SkPoint currPos = mParticleStates[currId].fPos;
        float lenInc = pointToLen(lastPos - currPos);
        float newLen = currLen + lenInc;
        if(newLen > length) {
            linePath.lineTo(lastPos + (currPos - lastPos)*
                            (length - currLen)*(1./lenInc));
            break;
        } else {
            linePath.lineTo(currPos);
        }
        currLen = newLen;
        lastPos = currPos;
        currId--;
    }

    mParticleStates[arrayId] =
            ParticleState(mLastPos,
                          mLastScale,
                          mSize,
                          qMax(0, qMin(255, qRound(mLastOpacity*255))),
                          linePath);

    float perPrevVelVar = (velocityVarPeriod - mPrevVelocityDuration)/
                            velocityVarPeriod;
    mLastPos += mLastVel + mPrevVelocityVar*perPrevVelVar +
                    mNextVelocityVar*(1.f - perPrevVelVar);
    mLastVel += acc;

    mPrevVelocityDuration += 1.;
}

bool Particle::isVisibleAtFrame(const int frame) {
    int arrayId = frame - mFirstFrame;
    if(arrayId < 0 || arrayId >= mNumberFrames) return false;
    return true;
}

ParticleState Particle::getParticleStateAtFrame(const int frame) {
    int arrayId = frame - mFirstFrame;
    return mParticleStates[arrayId];
}

bool Particle::getParticleStateAtFrameF(const qreal frame,
                                        ParticleState &state) {
    int arrayId = qFloor(frame) - mFirstFrame;
    int arrayId2 = qFloor(frame) + 1 - mFirstFrame;
    if(arrayId < 0) return false;
    if(arrayId2 < 0) return false;
    if(arrayId >= mNumberFrames) return false;
    if(arrayId2 >= mNumberFrames) return false;
    state = ParticleState::interpolate(mParticleStates[arrayId],
                                       mParticleStates[arrayId2],
                                       frame - qFloor(frame));
    return true;
}

ParticleEmitter::ParticleEmitter() :
    StaticComplexAnimator("particle emitter") {
    mPosPoint = SPtrCreate(AnimatedPoint)(
                mPos.get(),
                getTransformAnimator(),
                TYPE_PATH_POINT);
    //mPos->prp_setName("pos");
    //mPos.setCurrentValue(QPointF(0., 0.));

    mColorAnimator->qra_setCurrentValue(QColor(0, 0, 0));
    ca_addChild(mColorAnimator);

    mWidth->setValueRange(0., 6000.);
    mWidth->setCurrentBaseValue(0.);

    mSrcVelInfl->setValueRange(-1., 1.);
    mSrcVelInfl->setCurrentBaseValue(0.);

    mIniVelocity->setValueRange(-1000., 1000.);
    mIniVelocity->setCurrentBaseValue(10.);

    mIniVelocityVar->setValueRange(0., 1000.);
    mIniVelocityVar->setCurrentBaseValue(5.);

    mIniVelocityAngle->setValueRange(-3600., 3600.);
    mIniVelocityAngle->setCurrentBaseValue(-90.);

    mIniVelocityAngleVar->setValueRange(0., 3600.);
    mIniVelocityAngleVar->setCurrentBaseValue(15.);

    mAcceleration->setValuesRange(-100., 100.);
    mAcceleration->setBaseValue(QPointF(0., 9.8));

    mParticlesPerSecond->setValueRange(0., 10000.);
    mParticlesPerSecond->setCurrentBaseValue(120);

    mParticlesFrameLifetime->setValueRange(1., 1000.);
    mParticlesFrameLifetime->setCurrentBaseValue(50.);

    mVelocityRandomVar->setValueRange(0., 1000.);
    mVelocityRandomVar->setCurrentBaseValue(5.);

    mVelocityRandomVarPeriod->setValueRange(1., 100.);
    mVelocityRandomVarPeriod->setCurrentBaseValue(10.);

    mParticleSize->setValueRange(0., 100.);
    mParticleSize->setCurrentBaseValue(5.);

    mParticleSizeVar->setValueRange(0., 100.);
    mParticleSizeVar->setCurrentBaseValue(1.);

    mParticleLength->setValueRange(0., 2000.);
    mParticleLength->setCurrentBaseValue(0.);

    mParticlesDecayFrames->setValueRange(0., 1000.);
    mParticlesDecayFrames->setCurrentBaseValue(10.);

    mParticlesSizeDecay->setValueRange(0., 10.);
    mParticlesSizeDecay->setCurrentBaseValue(0.);

    mParticlesOpacityDecay->setValueRange(0., 1.);
    mParticlesOpacityDecay->setCurrentBaseValue(0.);

    ca_addChild(mPos);
    ca_addChild(mWidth);

    ca_addChild(mSrcVelInfl);

    ca_addChild(mIniVelocity);
    ca_addChild(mIniVelocityVar);

    ca_addChild(mIniVelocityAngle);
    ca_addChild(mIniVelocityAngleVar);

    ca_addChild(mAcceleration);

    ca_addChild(mParticlesPerSecond);
    ca_addChild(mParticlesFrameLifetime);

    ca_addChild(mVelocityRandomVar);
    ca_addChild(mVelocityRandomVarPeriod);

    ca_addChild(mParticleSize);
    ca_addChild(mParticleSizeVar);

    ca_addChild(mParticleLength);

    ca_addChild(mParticlesDecayFrames);
    ca_addChild(mParticlesSizeDecay);
    ca_addChild(mParticlesOpacityDecay);
    ca_addChild(mBoxTargetProperty);

    prp_setOwnUpdater(SPtrCreate(ParticlesUpdater)(this));

    scheduleGenerateParticles();
}

void ParticleEmitter::scheduleGenerateParticles() {
    mGenerateParticlesScheduled = true;
    prp_afterWholeInfluenceRangeChanged();
}

void ParticleEmitter::setMinFrame(const int minFrame) {
    mMinFrame = minFrame;
    scheduleGenerateParticles();
}

void ParticleEmitter::setMaxFrame(const int maxFrame) {
    mMaxFrame = maxFrame;
    scheduleGenerateParticles();
}

void ParticleEmitter::setFrameRange(const int minFrame, const int maxFrame) {
    if(minFrame == mMinFrame && mMaxFrame == maxFrame) return;
    if(mMaxFrame > maxFrame) {
        int currId = mParticles.count() - 1;
        while(currId > 0) {
            Particle *part = mParticles.at(currId);
            if(part->isVisibleAtFrame(maxFrame)) break;
            mParticles.removeOne(part);
            currId--;
            delete part;
        }
    } else {
        scheduleGenerateParticles();
    }

    mMinFrame = minFrame;
    mMaxFrame = maxFrame;
}

ColorAnimator *ParticleEmitter::getColorAnimator() {
    return mColorAnimator.data();
}

MovablePoint *ParticleEmitter::getPosPoint() {
    return mPosPoint.get();
}

EmitterData ParticleEmitter::getEmitterDataAtRelFrameF(
        const qreal relFrame,
        const stdsptr<ParticleBoxRenderData> &particleData) {
    EmitterData data;
    auto qcol = mColorAnimator->getColor(relFrame);
    data.color = toSkColor(qcol);

    BoundingBox *targetT = mBoxTargetProperty->getTarget();
    if(!targetT) {
        data.boxDraw = false;
        for(Particle *particle : mParticles) {
            if(particle->isVisibleAtFrame(relFrame)) {
                ParticleState stateT;
                if(particle->getParticleStateAtFrameF(relFrame, stateT)) {
                    data.particleStates << stateT;
                }
            }
        }
    } else {
        data.boxDraw = true;
        for(Particle *particle : mParticles) {
            if(particle->isVisibleAtFrame(relFrame)) {
                ParticleState stateT;
                if(!particle->getParticleStateAtFrameF(relFrame, stateT)) continue;
                stdsptr<BoxRenderData> renderData = targetT->createRenderData();
                QMatrix multMatr = QMatrix(stateT.fSize, 0.,
                                           0., stateT.fSize,
                                           0., 0.)*particleData->fTransform;
                renderData->appendRenderCustomizerFunctor(
                            SPtrCreate(MultiplyTransformCustomizer)(
                                multMatr, stateT.fOpacity/255.));
                renderData->appendRenderCustomizerFunctor(
                            SPtrCreate(ReplaceTransformDisplacementCustomizer)(
                                stateT.fPos.x(), stateT.fPos.y()));

                stateT.fTargetRenderData =
                        GetAsSPtr(renderData, BoxRenderData);
                renderData->fParentIsTarget = false;
                data.particleStates << stateT;
                renderData->addDependent(particleData.get());
                renderData->scheduleTask();
            }
        }
    }

    return data;
}

void ParticleEmitter::generateParticlesIfNeeded() {
    if(mGenerateParticlesScheduled) {
        mGenerateParticlesScheduled = false;
        generateParticles();
    }
}

void ParticleEmitter::generateParticles() {
    srand(0);
    qreal remainingPartFromFrame = 0.;
    QList<Particle*> notFinishedParticles;
    int nReuseParticles = mParticles.count();
    int currentReuseParticle = 0;
    bool reuseParticle = nReuseParticles > 0;

    int totalNeededParticles = 0;
    QPointF lastPos = mPos->getEffectiveValue(mMinFrame);
    for(int i = mMinFrame; i < mMaxFrame; i++) {
        qreal srcVelInfl =
                mSrcVelInfl->getEffectiveValue(i);
        qreal iniVelocity =
                mIniVelocity->getEffectiveValue(i);
        qreal iniVelocityVar =
                mIniVelocityVar->getEffectiveValue(i);
        qreal iniVelocityAngle =
                mIniVelocityAngle->getEffectiveValue(i);
        qreal iniVelocityAngleVar =
                mIniVelocityAngleVar->getEffectiveValue(i);
        qreal particlesPerFrame =
                mParticlesPerSecond->getEffectiveValue(i)/24.;
        int particlesFrameLifetime = qRound(
                    mParticlesFrameLifetime->getEffectiveValue(i));
        QPointF pos =
                mPos->getEffectiveValue(i);
        qreal width =
                mWidth->getEffectiveValue(i);
        qreal velocityVar =
                mVelocityRandomVar->getEffectiveValue(i);
        qreal velocityVarPeriod =
                mVelocityRandomVarPeriod->getEffectiveValue(i);
        QPointF acceleration =
                mAcceleration->getEffectiveValue(i)/24.;
        qreal finalScale =
                mParticlesSizeDecay->getEffectiveValue(i);
        qreal finalOpacity =
                mParticlesOpacityDecay->getEffectiveValue(i);
        qreal decayFrames =
                mParticlesDecayFrames->getEffectiveValue(i);
        qreal particleSize =
                mParticleSize->getEffectiveValue(i);
        qreal particleSizeVar =
                mParticleSizeVar->getEffectiveValue(i);
        qreal length = mParticleLength->getEffectiveValue(i);

        QPointF srcVel = (pos - lastPos)*srcVelInfl;

        int particlesToCreate = qRound(remainingPartFromFrame + particlesPerFrame);
        remainingPartFromFrame += particlesPerFrame - particlesToCreate;
        if(remainingPartFromFrame < 0.) remainingPartFromFrame = 0.;

        for(int j = 0; j < particlesToCreate; j++) {
            Particle *newParticle;
            if(reuseParticle) {
                newParticle = mParticles.at(currentReuseParticle);
                currentReuseParticle++;
                reuseParticle = currentReuseParticle < nReuseParticles;
            } else {
                newParticle = new Particle();
                mParticles << newParticle;
            }
            qreal partVelAmp = gRandF(iniVelocity - iniVelocityVar,
                                      iniVelocity + iniVelocityVar);


            QMatrix rotVelM;
            qreal velDeg = gRandF(iniVelocityAngle - iniVelocityAngleVar,
                                  iniVelocityAngle + iniVelocityAngleVar);
            rotVelM.rotate(velDeg);
            QPointF partVel = rotVelM.map(QPointF(partVelAmp, 0.)) + srcVel;

            qreal partSize = gRandF(particleSize - particleSizeVar,
                                    particleSize + particleSizeVar);

            qreal xTrans = gRandF(-width, width);

            newParticle->initializeParticle(i, particlesFrameLifetime,
                                            SkPoint::Make(pos.x() + xTrans,
                                                          pos.y()),
                                            toSkPoint(partVel),
                                            partSize);
            notFinishedParticles << newParticle;
        }
        int nNotFinished = notFinishedParticles.count();
        int currPart = 0;
        while(currPart < nNotFinished) {
            Particle *particle = notFinishedParticles.at(currPart);

            if(particle->isVisibleAtFrame(i)) {
                particle->generatePathNextFrame(i,
                                                velocityVar,
                                                velocityVarPeriod,
                                                toSkPoint(acceleration),
                                                finalScale,
                                                finalOpacity,
                                                decayFrames,
                                                length);
                currPart++;
            } else {
                notFinishedParticles.removeAt(currPart);
                nNotFinished--;
            }
        }
        totalNeededParticles += particlesToCreate;
        lastPos = pos;
    }
    int nToRemove = mParticles.count() - totalNeededParticles;
    for(int i = 0; i < nToRemove; i++) {
        delete mParticles.takeLast();
    }

    mUpdateParticlesForFrameScheduled = true;
}
