#include "particlebox.h"
#include "mainwindow.h"


double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

ParticleBox::ParticleBox(BoxesGroup *parent) :
    BoundingBox(parent, TYPE_PARTICLES) {
    setName("Particle Box");
    mTopLeftPoint = new MovablePoint(this, TYPE_PATH_POINT);
    mBottomRightPoint = new MovablePoint(this, TYPE_PATH_POINT);
    Animator *topLeftAnimator = mTopLeftPoint->
                                        getRelativePosAnimatorPtr();
    Animator *bottomRightAnimator = mBottomRightPoint->
                                        getRelativePosAnimatorPtr();
    ca_addChildAnimator(topLeftAnimator);
    ca_addChildAnimator(bottomRightAnimator);

    topLeftAnimator->prp_setUpdater(
                new DisplayedFillStrokeSettingsUpdater(this));
    topLeftAnimator->prp_setName("top left");
    bottomRightAnimator->prp_setUpdater(
                new DisplayedFillStrokeSettingsUpdater(this));
    bottomRightAnimator->prp_setName("bottom right");

    //addEmitter(new ParticleEmitter(this));
}

void ParticleBox::getAccelerationAt(const QPointF &pos,
                                    const int &frame,
                                    QPointF *acc) {
    *acc = QPointF(0., 9.8)/24.;
}

void ParticleBox::updateAfterFrameChanged(int currentFrame) {
    BoundingBox::updateAfterFrameChanged(currentFrame);
    scheduleUpdate();
}

void ParticleBox::updateRelBoundingRect() {
    mRelBoundingRect = QRectF(mTopLeftPoint->getRelativePos(),
                              mBottomRightPoint->getRelativePos());
//    foreach(ParticleEmitter *emitter, mEmitters) {
//        mRelBoundingRect = mRelBoundingRect.united(
//                    emitter->getParticlesBoundingRect());
//    }

    BoundingBox::updateRelBoundingRect();
}

void ParticleBox::preUpdatePixmapsUpdates() {
    foreach(ParticleEmitter *emitter, mEmitters) {
        emitter->generateParticlesIfNeeded();
        emitter->updateParticlesForFrameIfNeeded();
    }

    BoundingBox::preUpdatePixmapsUpdates();
}

bool ParticleBox::relPointInsidePath(QPointF relPos) {
    if(mRelBoundingRectPath.contains(relPos) ) {
        /*if(mEmitters.isEmpty()) */return true;
        foreach(ParticleEmitter *emitter, mEmitters) {
            if(emitter->relPointInsidePath(relPos)) {
                return true;
            }
        }
        return false;
    } else {
        return false;
    }
}

void ParticleBox::addEmitter(ParticleEmitter *emitter) {
    mEmitters << emitter;
    ca_addChildAnimator(emitter);
}

void ParticleBox::addEmitterAtAbsPos(const QPointF &absPos) {
    ParticleEmitter *emitter = new ParticleEmitter(this);
    emitter->getPosPoint()->setAbsolutePos(absPos, false);
    addEmitter(emitter);
}

void ParticleBox::draw(QPainter *p)
{
    if(shouldUpdateAndDraw()) {
        p->save();

        p->setClipRect(mRelBoundingRect);
        foreach(ParticleEmitter *emitter, mEmitters) {
            emitter->drawParticles(p);
        }

        //p->setCompositionMode(QPainter::CompositionMode_DestinationIn);
        //p->fillRect(mRelBoundingRect, Qt::white);
        p->restore();
    }
}

void ParticleBox::applyPaintSetting(const PaintSetting &setting) {
    if(setting.targetsFill()) {
        foreach(ParticleEmitter *emitter, mEmitters) {
            setting.applyColorSetting(emitter->getColorAnimator());
        }
    }
}

void ParticleBox::startAllPointsTransform() {
    mBottomRightPoint->startTransform();
    mTopLeftPoint->startTransform();
    startTransform();
}

void ParticleBox::drawSelected(QPainter *p,
                             const CanvasMode &currentCanvasMode) {
    if(isVisibleAndInVisibleDurationRect()) {
        p->save();
        drawBoundingRect(p);
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            p->setPen(QPen(QColor(0, 0, 0, 255), 1.5));
            mTopLeftPoint->draw(p);
            mBottomRightPoint->draw(p);
            foreach(ParticleEmitter *emitter, mEmitters) {
                MovablePoint *pt = emitter->getPosPoint();
                pt->draw(p);
            }
        }
        p->restore();
    }
}


MovablePoint *ParticleBox::getPointAt(const QPointF &absPtPos,
                                      const CanvasMode &currentCanvasMode) {
    MovablePoint *pointToReturn = NULL;
    if(mTopLeftPoint->isPointAtAbsPos(absPtPos)) {
        return mTopLeftPoint;
    }
    if(mBottomRightPoint->isPointAtAbsPos(absPtPos) ) {
        return mBottomRightPoint;
    }
    foreach(ParticleEmitter *emitter, mEmitters) {
        MovablePoint *pt = emitter->getPosPoint();
        if(pt->isPointAtAbsPos(absPtPos)) {
            return pt;
        }
    }

    return pointToReturn;
}

void ParticleBox::selectAndAddContainedPointsToList(QRectF absRect,
                                                  QList<MovablePoint *> *list) {
    if(!mTopLeftPoint->isSelected()) {
        if(mTopLeftPoint->isContainedInRect(absRect)) {
            mTopLeftPoint->select();
            list->append(mTopLeftPoint);
        }
    }
    if(!mBottomRightPoint->isSelected()) {
        if(mBottomRightPoint->isContainedInRect(absRect)) {
            mBottomRightPoint->select();
            list->append(mBottomRightPoint);
        }
    }
    foreach(ParticleEmitter *emitter, mEmitters) {
        MovablePoint *pt = emitter->getPosPoint();
        if(pt->isContainedInRect(absRect)) {
            pt->select();
            list->append(pt);
        }
    }
}

MovablePoint *ParticleBox::getBottomRightPoint() {
    return mBottomRightPoint;
}

Particle::Particle(ParticleBox *parentBox) {
    mParentBox = parentBox;
}

void Particle::initializeParticle(int firstFrame, int nFrames,
                                  const QPointF &iniPos,
                                  const QPointF &iniVel,
                                  const qreal &partSize) {
    mSize = partSize;
    mPrevVelocityVar = QPointF(0., 0.);
    mNextVelocityVar = QPointF(0., 0.);
    mPrevVelocityDuration = 10000000.;
    mLastScale = 1.;
    mLastOpacity = 1.;

    mFirstFrame = firstFrame;
    mLastPos = iniPos;
    mLastVel = iniVel;
    if(mParticleStates != NULL) {
        if(nFrames == mNumberFrames) return;
        delete[] mParticleStates;
    }
    mNumberFrames = nFrames;
    mParticleStates = new ParticleState[nFrames];
}

void Particle::generatePathNextFrame(const int &frame,
                                     const qreal &velocityVar,
                                     const qreal &velocityVarPeriod,
                                     const QPointF &acc,
                                     const qreal &finalScale,
                                     const qreal &finalOpacity,
                                     const qreal &decayFrames,
                                     const qreal &length) {
    if(mPrevVelocityDuration > velocityVarPeriod) {
        mPrevVelocityVar = mNextVelocityVar;
        mNextVelocityVar = QPointF(fRand(-velocityVar, velocityVar),
                                   fRand(-velocityVar, velocityVar));
        mPrevVelocityDuration = 0.;
    }

    int arrayId = frame - mFirstFrame;

    if(arrayId == 0) {
        qreal iniTime = fRand(0., 1.);
        mLastPos += mLastVel*iniTime;
        mLastVel += acc*iniTime;
    }

    int remaining = mNumberFrames - arrayId;
    if(remaining <= decayFrames) {
        mLastScale += (finalScale - 1.)/decayFrames;
        mLastOpacity += (finalOpacity - 1.)/decayFrames;
    }

    if(length > 0.1) {
        QPainterPath linePath;
        qreal currLen = 0.;
        int currId = arrayId - 1;
        linePath.moveTo(mLastPos);
        QPointF lastPos = mLastPos;
        while(currId > -1) {
            QPointF currPos = mParticleStates[currId].pos;
            qreal lenInc = pointToLen(lastPos - currPos);
            qreal newLen = currLen + lenInc;
            if(newLen > length) {
                linePath.lineTo(lastPos + (currPos - lastPos)*
                                (length - currLen)/lenInc);
                break;
            } else {
                linePath.lineTo(currPos);
            }
            currLen = newLen;
            lastPos = currPos;
            currId--;
        }

        mParticleStates[arrayId] = ParticleState(mLastPos,
                                                 mLastScale,
                                                 mSize,
                                                 mLastOpacity,
                                                 linePath);
    } else {
        mParticleStates[arrayId] = ParticleState(mLastPos,
                                                 mLastScale,
                                                 mSize,
                                                 mLastOpacity);
    }

    qreal perPrevVelVar = (velocityVarPeriod - mPrevVelocityDuration)/
                            velocityVarPeriod;
    mLastPos += mLastVel +
            perPrevVelVar*mPrevVelocityVar +
            (1. - perPrevVelVar)*mNextVelocityVar;
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

ParticleEmitter::ParticleEmitter(ParticleBox *parentBox) :
    ComplexAnimator() {
    setParentBox(parentBox);

    prp_setName("particle emitter");

    mPos = new MovablePoint(mParentBox, TYPE_PATH_POINT);
    //mPos->setName("pos");
    //mPos.setCurrentValue(QPointF(0., 0.));

    mColorAnimator.prp_setName("color");
    mColorAnimator.qra_setCurrentValue(Color(0, 0, 0));
    ca_addChildAnimator(&mColorAnimator);

    mWidth.prp_setName("width");
    mWidth.qra_setValueRange(0., 6000.);
    mWidth.qra_setCurrentValue(0.);

    mIniVelocity.prp_setName("ini vel");
    mIniVelocity.qra_setValueRange(-1000., 1000.);
    mIniVelocity.qra_setCurrentValue(10.);

    mIniVelocityVar.prp_setName("ini vel var");
    mIniVelocityVar.qra_setValueRange(0., 1000.);
    mIniVelocityVar.qra_setCurrentValue(5.);

    mIniVelocityAngle.prp_setName("ini vel angle");
    mIniVelocityAngle.qra_setValueRange(-3600., 3600.);
    mIniVelocityAngle.qra_setCurrentValue(-90.);

    mIniVelocityAngleVar.prp_setName("ini vel angle var");
    mIniVelocityAngleVar.qra_setValueRange(0., 3600.);
    mIniVelocityAngleVar.qra_setCurrentValue(15.);

    mAcceleration.prp_setName("acceleration");
    mAcceleration.qra_setValueRange(-100., 100.);
    mAcceleration.qra_setCurrentValue(QPointF(0., 9.8));

    mParticlesPerSecond.prp_setName("particles per second");
    mParticlesPerSecond.qra_setValueRange(0., 10000.);
    mParticlesPerSecond.qra_setCurrentValue(120);

    mParticlesFrameLifetime.prp_setName("particles lifetime");
    mParticlesFrameLifetime.qra_setValueRange(1., 1000.);
    mParticlesFrameLifetime.qra_setCurrentValue(50.);

    mVelocityRandomVar.prp_setName("velocity random var");
    mVelocityRandomVar.qra_setValueRange(0., 1000.);
    mVelocityRandomVar.qra_setCurrentValue(5.);

    mVelocityRandomVarPeriod.prp_setName("velocity random var period");
    mVelocityRandomVarPeriod.qra_setValueRange(1., 100.);
    mVelocityRandomVarPeriod.qra_setCurrentValue(10.);

    mParticleSize.prp_setName("particle size");
    mParticleSize.qra_setValueRange(0., 100.);
    mParticleSize.qra_setCurrentValue(5.);

    mParticleSizeVar.prp_setName("particle size var");
    mParticleSizeVar.qra_setValueRange(0., 100.);
    mParticleSizeVar.qra_setCurrentValue(1.);

    mParticleLength.prp_setName("length");
    mParticleLength.qra_setValueRange(0., 2000.);
    mParticleLength.qra_setCurrentValue(0.);

    mParticlesDecayFrames.prp_setName("decay frames");
    mParticlesDecayFrames.qra_setValueRange(0., 1000.);
    mParticlesDecayFrames.qra_setCurrentValue(10.);

    mParticlesSizeDecay.prp_setName("final scale");
    mParticlesSizeDecay.qra_setValueRange(0., 10.);
    mParticlesSizeDecay.qra_setCurrentValue(0.);

    mParticlesOpacityDecay.prp_setName("final opacity");
    mParticlesOpacityDecay.qra_setValueRange(0., 1.);
    mParticlesOpacityDecay.qra_setCurrentValue(0.);

    //mPos.blockPointer();
    mWidth.blockPointer();

    mIniVelocity.blockPointer();
    mIniVelocityVar.blockPointer();

    mIniVelocityAngle.blockPointer();
    mIniVelocityAngleVar.blockPointer();

    mAcceleration.blockPointer();

    mParticlesPerSecond.blockPointer();
    mParticlesFrameLifetime.blockPointer();

    mVelocityRandomVar.blockPointer();
    mVelocityRandomVarPeriod.blockPointer();

    mParticleSize.blockPointer();
    mParticleSizeVar.blockPointer();

    mParticleLength.blockPointer();

    mParticlesDecayFrames.blockPointer();
    mParticlesSizeDecay.blockPointer();
    mParticlesOpacityDecay.blockPointer();

    mPos->getRelativePosAnimatorPtr()->prp_setName("pos");
    ca_addChildAnimator(mPos->getRelativePosAnimatorPtr());
    ca_addChildAnimator(&mWidth);

    ca_addChildAnimator(&mIniVelocity);
    ca_addChildAnimator(&mIniVelocityVar);

    ca_addChildAnimator(&mIniVelocityAngle);
    ca_addChildAnimator(&mIniVelocityAngleVar);

    ca_addChildAnimator(&mAcceleration);

    ca_addChildAnimator(&mParticlesPerSecond);
    ca_addChildAnimator(&mParticlesFrameLifetime);

    ca_addChildAnimator(&mVelocityRandomVar);
    ca_addChildAnimator(&mVelocityRandomVarPeriod);

    ca_addChildAnimator(&mParticleSize);
    ca_addChildAnimator(&mParticleSizeVar);

    ca_addChildAnimator(&mParticleLength);

    ca_addChildAnimator(&mParticlesDecayFrames);
    ca_addChildAnimator(&mParticlesSizeDecay);
    ca_addChildAnimator(&mParticlesOpacityDecay);

    prp_setUpdater(new ParticlesUpdater(this));
    prp_blockUpdater();
}

void ParticleEmitter::setParentBox(ParticleBox *parentBox) {
    mParentBox = parentBox;

    scheduleGenerateParticles();
    if(parentBox == NULL) {
        mColorAnimator.prp_setUpdater(NULL);
    } else {
        mColorAnimator.prp_setUpdater(
                    new DisplayedFillStrokeSettingsUpdater(parentBox));
    }
}

void ParticleEmitter::scheduleGenerateParticles() {
    mGenerateParticlesScheduled = true;
    mParentBox->clearCache();
    mParentBox->scheduleUpdate();
}

void ParticleEmitter::scheduleUpdateParticlesForFrame() {
    mUpdateParticlesForFrameScheduled = true;
    mParentBox->scheduleUpdate();
}

void ParticleEmitter::updateParticlesForFrameIfNeeded() {
    if(mUpdateParticlesForFrameScheduled) {
        mUpdateParticlesForFrameScheduled = false;
        updateParticlesForFrame(anim_mCurrentAbsFrame);
    }
}

void ParticleEmitter::duplicateAnimatorsFrom(QPointFAnimator *pos,
                                             QrealAnimator *width,
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
                                             QrealAnimator *particlesOpacityDecay) {
    pos->prp_makeDuplicate(mPos->getRelativePosAnimatorPtr());
    width->prp_makeDuplicate(&mWidth);

    iniVelocity->prp_makeDuplicate(&mIniVelocity);
    iniVelocityVar->prp_makeDuplicate(&mIniVelocityVar);
    iniVelocityAngle->prp_makeDuplicate(&mIniVelocityAngle);
    iniVelocityAngleVar->prp_makeDuplicate(&mIniVelocityAngleVar);
    acceleration->prp_makeDuplicate(&mAcceleration);

    particlesPerSecond->prp_makeDuplicate(&mParticlesPerSecond);
    particlesFrameLifetime->prp_makeDuplicate(&mParticlesFrameLifetime);

    velocityRandomVar->prp_makeDuplicate(&mVelocityRandomVar);
    velocityRandomVarPeriod->prp_makeDuplicate(&mVelocityRandomVarPeriod);

    particleSize->prp_makeDuplicate(&mParticleSize);
    particleSizeVar->prp_makeDuplicate(&mParticleSizeVar);

    particleLength->prp_makeDuplicate(&mParticleLength);

    particlesDecayFrames->prp_makeDuplicate(&mParticlesDecayFrames);
    particlesSizeDecay->prp_makeDuplicate(&mParticlesSizeDecay);
    particlesOpacityDecay->prp_makeDuplicate(&mParticlesOpacityDecay);
}

void ParticleEmitter::prp_makeDuplicate(Property *target) {
    ParticleEmitter *peTarget = ((ParticleEmitter*)target);
    peTarget->duplicateAnimatorsFrom(
                mPos->getRelativePosAnimatorPtr(),
                &mWidth,
                &mIniVelocity,
                &mIniVelocityVar,
                &mIniVelocityAngle,
                &mIniVelocityAngleVar,
                &mAcceleration,
                &mParticlesPerSecond,
                &mParticlesFrameLifetime,
                &mVelocityRandomVar,
                &mVelocityRandomVarPeriod,
                &mParticleSize,
                &mParticleSizeVar,
                &mParticleLength,
                &mParticlesDecayFrames,
                &mParticlesSizeDecay,
                &mParticlesOpacityDecay);
    peTarget->setMinFrame(mMinFrame);
    peTarget->setMaxFrame(mMaxFrame);
}

ColorAnimator *ParticleEmitter::getColorAnimator() {
    return &mColorAnimator;
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
    for(int i = mMinFrame; i < mMaxFrame; i++) {
        qreal iniVelocity =
                mIniVelocity.getCurrentValueAtAbsFrame(i);
        qreal iniVelocityVar =
                mIniVelocityVar.getCurrentValueAtAbsFrame(i);
        qreal iniVelocityAngle =
                mIniVelocityAngle.getCurrentValueAtAbsFrame(i);
        qreal iniVelocityAngleVar =
                mIniVelocityAngleVar.getCurrentValueAtAbsFrame(i);
        qreal particlesPerFrame =
                mParticlesPerSecond.getCurrentValueAtAbsFrame(i)/24.;
        qreal particlesFrameLifetime =
                mParticlesFrameLifetime.getCurrentValueAtAbsFrame(i);
        QPointF pos =
                mPos->getRelativePosAnimatorPtr()->getCurrentPointValueAtFrame(i);
        qreal width =
                mWidth.getCurrentValueAtAbsFrame(i);
        qreal velocityVar =
                mVelocityRandomVar.getCurrentValueAtAbsFrame(i);
        qreal velocityVarPeriod =
                mVelocityRandomVarPeriod.getCurrentValueAtAbsFrame(i);
        QPointF acceleration = mAcceleration.getCurrentPointValueAtFrame(i)/24.;
        qreal finalScale =
                mParticlesSizeDecay.getCurrentValueAtAbsFrame(i);
        qreal finalOpacity =
                mParticlesOpacityDecay.getCurrentValueAtAbsFrame(i);
        qreal decayFrames =
                mParticlesDecayFrames.getCurrentValueAtAbsFrame(i);
        qreal particleSize =
                mParticleSize.getCurrentValueAtAbsFrame(i);
        qreal particleSizeVar =
                mParticleSizeVar.getCurrentValueAtAbsFrame(i);
        qreal length = mParticleLength.getCurrentValueAtAbsFrame(i);

        int particlesToCreate = remainingPartFromFrame + particlesPerFrame;
        remainingPartFromFrame += particlesPerFrame - particlesToCreate;
        if(remainingPartFromFrame < 0.) remainingPartFromFrame = 0.;

        for(int j = 0; j < particlesToCreate; j++) {
            Particle *newParticle;
            if(reuseParticle) {
                newParticle = mParticles.at(currentReuseParticle);
                currentReuseParticle++;
                reuseParticle = currentReuseParticle < nReuseParticles;
            } else {
                newParticle = new Particle(mParentBox);
                mParticles << newParticle;
            }
            qreal partVelAmp = fRand(iniVelocity - iniVelocityVar,
                                     iniVelocity + iniVelocityVar);


            QMatrix rotVelM;
            qreal velDeg = fRand(iniVelocityAngle - iniVelocityAngleVar,
                                 iniVelocityAngle + iniVelocityAngleVar);
            rotVelM.rotate(velDeg);
            QPointF partVel = rotVelM.map(QPointF(partVelAmp, 0.));

            qreal partSize = fRand(particleSize - particleSizeVar,
                                   particleSize + particleSizeVar);

            qreal xTrans = fRand(-width,
                                 width);

            newParticle->initializeParticle(i, particlesFrameLifetime,
                                            QPointF(pos.x() + xTrans,
                                                    pos.y()),
                                            partVel, partSize);
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
                                                acceleration,
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
    }
    int nToRemove = mParticles.count() - totalNeededParticles;
    for(int i = 0; i < nToRemove; i++) {
        delete mParticles.takeLast();
    }

    mUpdateParticlesForFrameScheduled = true;
}

void ParticleEmitter::drawParticles(QPainter *p) {
    p->save();
    if(mParticleLength.qra_getCurrentValue() > 0.1) {
        QPen pen = QPen(mColorAnimator.qra_getCurrentValue().qcol);
        pen.setCapStyle(Qt::RoundCap);
        p->setPen(pen);
        p->setBrush(Qt::NoBrush);
    } else {
        p->setBrush(mColorAnimator.qra_getCurrentValue().qcol);
        p->setPen(Qt::NoPen);
    }
    foreach(const ParticleState &state, mParticleStates) {
        state.draw(p);
    }
    p->restore();
}

void ParticleEmitter::updateParticlesForFrame(const int &frame) {
    mParticleStates.clear();
    foreach(Particle *particle, mParticles) {
        if(particle->isVisibleAtFrame(frame)) {
            mParticleStates << particle->getParticleStateAtFrame(frame);
        }
    }
    QRectF rect;
    foreach(const ParticleState &state, mParticleStates) {
        const QPointF &pos = state.pos;
        if(rect.isNull()) {
            rect.setTopLeft(pos);
            rect.setTopRight(pos);
        } else {
            if(rect.left() > pos.x()) {
                rect.setLeft(pos.x());
            } else if(rect.right() < pos.x()) {
                rect.setRight(pos.x());
            }
            if(rect.top() > pos.y()) {
                rect.setTop(pos.y());
            } else if(rect.bottom() < pos.y()) {
                rect.setBottom(pos.y());
            }
        }
    }
    if(!rect.isNull()) {
        rect.adjust(-5., -5, 5, 5);
    }
    mParticlesBoundingRect = rect;
}

QRectF ParticleEmitter::getParticlesBoundingRect() {
    return mParticlesBoundingRect;
}

void ParticleEmitter::prp_setAbsFrame(int frame) {
    ComplexAnimator::prp_setAbsFrame(frame);
    scheduleUpdateParticlesForFrame();
}
