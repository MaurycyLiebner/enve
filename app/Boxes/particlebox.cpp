#include "particlebox.h"
#include "GUI/mainwindow.h"
#include "durationrectangle.h"
#include "canvas.h"
#include "pointhelpers.h"
#include "MovablePoints/animatedpoint.h"
#include "PropertyUpdaters/displayedfillstrokesettingsupdater.h"
#include "PropertyUpdaters/particlesupdater.h"
#include "Animators/paintsettings.h"
#include "Animators/transformanimator.h"
#include "Animators/effectanimators.h"
#include "paintsettingsapplier.h"

ParticleBox::ParticleBox() : BoundingBox(TYPE_PARTICLES) {
    setName("Particle Box");
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

    ca_prependChildAnimator(mTopLeftAnimator.data(), mEffectsAnimators);
    ca_prependChildAnimator(mBottomRightAnimator.data(), mEffectsAnimators);

    qsptr<VaryingLenAnimationRect> durRect =
            SPtrCreate(VaryingLenAnimationRect)(this);
    setDurationRectangle(durRect);
    durRect->setMaxFrame(200);
    durRect->setMinFrame(-10);

    //addEmitter(new ParticleEmitter(this));
}

void ParticleBox::getAccelerationAt(const QPointF &pos,
                                    const int &frame,
                                    QPointF *acc) {
    Q_UNUSED(pos);
    Q_UNUSED(frame);
    *acc = QPointF(0., 9.8)/24.;
}

void ParticleBox::anim_setAbsFrame(const int &frame) {
    BoundingBox::anim_setAbsFrame(frame);
    scheduleUpdate(Animator::FRAME_CHANGE);
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
    ca_addChildAnimator(emitter);
    scheduleUpdate(Animator::USER_CHANGE);
}

void ParticleBox::removeEmitter(const qsptr<ParticleEmitter>& emitter) {
    mEmitters.removeOne(emitter);
    ca_removeChildAnimator(emitter);
    scheduleUpdate(Animator::USER_CHANGE);
}

FrameRange ParticleBox::prp_getIdenticalRelFrameRange(const int &relFrame) const {
    if(isRelFrameVisibleAndInVisibleDurationRect(relFrame)) {
        return {relFrame, relFrame};
    }
    return BoundingBox::prp_getIdenticalRelFrameRange(relFrame);
}

void ParticleBox::addEmitterAtAbsPos(const QPointF &absPos) {
    auto emitter = SPtrCreate(ParticleEmitter)(this);
    emitter->getPosPoint()->setRelativePos(mapAbsPosToRel(absPos));
    addEmitter(emitter);
}

bool ParticleBox::SWT_isParticleBox() const { return true; }

QRectF ParticleBox::getRelBoundingRect(const qreal &relFrame) {
    return QRectF(mTopLeftAnimator->getEffectiveValueAtRelFrame(relFrame),
                  mBottomRightAnimator->getEffectiveValueAtRelFrame(relFrame));
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

void ParticleBox::startAllPointsTransform() {
    mBottomRightPoint->startTransform();
    mTopLeftPoint->startTransform();
    startTransform();
}

MovablePoint *ParticleBox::getBottomRightPoint() {
    return mBottomRightPoint.get();
}

Particle::Particle(ParticleBox *parentBox) {
    mParentBox = parentBox;
}

void Particle::initializeParticle(const int &firstFrame,
                                  const int &nFrames,
                                  const SkPoint &iniPos,
                                  const SkPoint &iniVel,
                                  const SkScalar &partSize) {
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

void Particle::generatePathNextFrame(const int &frame,
                                     const SkScalar &velocityVar,
                                     const SkScalar &velocityVarPeriod,
                                     const SkPoint &acc,
                                     const SkScalar &finalScale,
                                     const SkScalar &finalOpacity,
                                     const SkScalar &decayFrames,
                                     const SkScalar &length) {
    if(mPrevVelocityDuration > velocityVarPeriod) {
        mPrevVelocityVar = mNextVelocityVar;
        mNextVelocityVar = SkPoint::Make(gRandF(-velocityVar, velocityVar),
                                         gRandF(-velocityVar, velocityVar));
        mPrevVelocityDuration = 0.;
    }

    int arrayId = frame - mFirstFrame;

    if(arrayId == 0) {
        SkScalar iniTime = gRandF(0., 1.);
        mLastPos += mLastVel*iniTime;
        mLastVel += acc*iniTime;
    }

    int remaining = mNumberFrames - arrayId;
    if(remaining <= decayFrames) {
        mLastScale += (finalScale - 1.)/decayFrames;
        mLastOpacity += (finalOpacity - 1.)/decayFrames;
    }

    SkPath linePath;
    SkScalar currLen = 0.;
    int currId = arrayId - 1;
    SkPoint lastPos = mLastPos;
    linePath.moveTo(lastPos);
    while(currId > -1) {
        SkPoint currPos = mParticleStates[currId].fPos;
        SkScalar lenInc = pointToLen(lastPos - currPos);
        SkScalar newLen = currLen + lenInc;
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

    SkScalar perPrevVelVar = (velocityVarPeriod - mPrevVelocityDuration)/
                            velocityVarPeriod;
    mLastPos += mLastVel + mPrevVelocityVar*perPrevVelVar +
                    mNextVelocityVar*(1.f - perPrevVelVar);
    mLastVel += acc;

    mPrevVelocityDuration += 1.;
}

bool Particle::isVisibleAtFrame(const int &frame) {
    int arrayId = frame - mFirstFrame;
    if(arrayId < 0 || arrayId >= mNumberFrames) return false;
    return true;
}

ParticleState Particle::getParticleStateAtFrame(const int &frame) {
    int arrayId = frame - mFirstFrame;
    return mParticleStates[arrayId];
}

bool Particle::getParticleStateAtFrameF(const qreal &frame,
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

ParticleEmitter::ParticleEmitter(ParticleBox *parentBox) :
    ComplexAnimator("particle emitter") {
    setParentBox(parentBox);

    mPosPoint = SPtrCreate(AnimatedPoint)(
                mPos.get(),
                mParentBox_k->getTransformAnimator(),
                TYPE_PATH_POINT);
    //mPos->setName("pos");
    //mPos.setCurrentValue(QPointF(0., 0.));

    mColorAnimator->qra_setCurrentValue(QColor(0, 0, 0));
    ca_addChildAnimator(mColorAnimator);

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

    ca_addChildAnimator(mPos);
    ca_addChildAnimator(mWidth);

    ca_addChildAnimator(mSrcVelInfl);

    ca_addChildAnimator(mIniVelocity);
    ca_addChildAnimator(mIniVelocityVar);

    ca_addChildAnimator(mIniVelocityAngle);
    ca_addChildAnimator(mIniVelocityAngleVar);

    ca_addChildAnimator(mAcceleration);

    ca_addChildAnimator(mParticlesPerSecond);
    ca_addChildAnimator(mParticlesFrameLifetime);

    ca_addChildAnimator(mVelocityRandomVar);
    ca_addChildAnimator(mVelocityRandomVarPeriod);

    ca_addChildAnimator(mParticleSize);
    ca_addChildAnimator(mParticleSizeVar);

    ca_addChildAnimator(mParticleLength);

    ca_addChildAnimator(mParticlesDecayFrames);
    ca_addChildAnimator(mParticlesSizeDecay);
    ca_addChildAnimator(mParticlesOpacityDecay);
    ca_addChildAnimator(mBoxTargetProperty);

    prp_setOwnUpdater(SPtrCreate(ParticlesUpdater)(this));

    scheduleGenerateParticles();
}

void ParticleEmitter::setParentBox(ParticleBox *parentBox) {
    mParentBox_k = parentBox;

    scheduleGenerateParticles();
    if(!parentBox) {
        mColorAnimator->prp_setInheritedUpdater(nullptr);
    } else {
        mColorAnimator->prp_setInheritedUpdater(
                    SPtrCreate(DisplayedFillStrokeSettingsUpdater)(parentBox));
    }
}

void ParticleEmitter::scheduleGenerateParticles() {
    mGenerateParticlesScheduled = true;
    mParentBox_k->prp_updateInfluenceRangeAfterChanged();
    mParentBox_k->scheduleUpdate(Animator::USER_CHANGE);
}

void ParticleEmitter::setMinFrame(const int &minFrame) {
    mMinFrame = minFrame;
    scheduleGenerateParticles();
}

void ParticleEmitter::setMaxFrame(const int &maxFrame) {
    mMaxFrame = maxFrame;
    scheduleGenerateParticles();
}

void ParticleEmitter::setFrameRange(const int &minFrame, const int &maxFrame) {
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
        const qreal &relFrame,
        const stdsptr<ParticleBoxRenderData> &particleData) {
    EmitterData data;
    auto qcol = mColorAnimator->getColorAtRelFrame(relFrame);
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
                stdsptr<BoundingBoxRenderData> renderData = targetT->createRenderData();
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
                        GetAsSPtr(renderData, BoundingBoxRenderData);
                renderData->fMaxBoundsEnabled = false;
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
    QPointF lastPos = mPos->getEffectiveValueAtRelFrame(mMinFrame);
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
                mPos->getEffectiveValueAtRelFrame(i);
        qreal width =
                mWidth->getEffectiveValue(i);
        qreal velocityVar =
                mVelocityRandomVar->getEffectiveValue(i);
        qreal velocityVarPeriod =
                mVelocityRandomVarPeriod->getEffectiveValue(i);
        QPointF acceleration =
                mAcceleration->getEffectiveValueAtRelFrame(i)/24.;
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
                newParticle = new Particle(mParentBox_k);
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
