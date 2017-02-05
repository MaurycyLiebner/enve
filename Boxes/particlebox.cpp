#include "particlebox.h"

ParticleBox::ParticleBox(BoxesGroup *parent) :
    BoundingBox(parent, TYPE_PARTICLES) {

}

void ParticleBox::getAccelerationAt(const QPointF &pos,
                                    const int &frame,
                                    QPointF *acc) {
    *acc = QPointF(0., 9.8)/24.;
}

void ParticleBox::updateAfterFrameChanged(int currentFrame) {
    foreach(ParticleEmitter *emitter, mEmitters) {
        emitter->updateParticlesForFrame(currentFrame);
    }
}

void ParticleBox::updateBoundingRect() {
    mRelBoundingRect = QRectF();
    foreach(ParticleEmitter *emitter, mEmitters) {
        mRelBoundingRect = mRelBoundingRect.united(
                    emitter->getParticlesBoundingRect());
    }

    qreal effectsMargin = mEffectsMargin*mUpdateCanvasTransform.m11();
    mPixBoundingRect = mUpdateTransform.mapRect(mRelBoundingRect).
                        adjusted(-effectsMargin, -effectsMargin,
                                 effectsMargin, effectsMargin);
    BoundingBox::updateBoundingRect();
}

void ParticleBox::draw(QPainter *p)
{
    if(mVisible) {
        p->save();

        foreach(ParticleEmitter *emitter, mEmitters) {
            emitter->drawParticles(p);
        }

        p->restore();
    }
}

Particle::Particle(ParticleBox *parentBox) {
    mParentBox = parentBox;
}

void Particle::initializeParticle(int firstFrame, int nFrames,
                                  const QPointF &iniPos,
                                  const QPointF &iniVel) {
    mFirstFrame = firstFrame;
    if(mPos != NULL) {
        if(nFrames == mNumberFrames) return;
        delete[] mPos;
    }
    mNumberFrames = nFrames;
    mPos = new QPointF[nFrames];
    mLastPos = iniPos;
    mLastVel = iniVel;
}

void Particle::generatePathNextFrame(int frame) {
    QPointF acc;
    mParentBox->getAccelerationAt(mLastPos,
                                  frame,
                                  &acc);
    mLastVel += acc;

    mLastPos += mLastVel;

    mPos[frame] = mLastPos;
}

bool Particle::isVisibleAtFrame(const int &frame) {
    int arrayId = frame - mFirstFrame;
    if(arrayId < 0 || arrayId >= mNumberFrames) return false;
    return true;
}

QPointF Particle::getPosAtFrame(const int &frame) {
    int arrayId = frame - mFirstFrame;
    return mPos[arrayId];
}

ParticleEmitter::ParticleEmitter(ParticleBox *parentBox) {
    mParentBox = parentBox;

    generateParticles();
}

double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

void ParticleEmitter::generateParticles() {
    qreal remainingPartFromFrame = 0.;
    QList<Particle*> notFinishedParticles;
    int nReuseParticles = mParticles.count();
    int currentReuseParticle = 0;
    bool reuseParticle = nReuseParticles > 0;

    int totalNeededParticles = 0;
    for(int i = mMinFrame; i < mMaxFrame; i++) {
        qreal iniVelocity =
                mIniVelocity.getValueAtFrame(i);
        qreal iniVelocityVar =
                mIniVelocityVar.getValueAtFrame(i);
        qreal iniVelocityAngle =
                mIniVelocityAngle.getValueAtFrame(i);
        qreal iniVelocityAngleVar =
                mIniVelocityAngleVar.getValueAtFrame(i);
        qreal particlesPerFrame =
                mParticlesPerSecond.getValueAtFrame(i)/24.;
        qreal particlesFrameLifetime =
                mParticlesFrameLifetime.getValueAtFrame(i);

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

            newParticle->initializeParticle(i, particlesFrameLifetime,
                                            partVel, mPos);
            notFinishedParticles << newParticle;
        }
        int nNotFinished = notFinishedParticles.count();
        int currPart = 0;
        while(currPart < nNotFinished) {
            Particle *particle = notFinishedParticles.at(currPart);

            if(particle->isVisibleAtFrame(i)) {
                particle->generatePathNextFrame(i);
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
}

void ParticleEmitter::drawParticles(QPainter *p) {
    p->setBrush(Qt::black);
    foreach(const QPointF &pos, mParticlePos) {
        p->drawEllipse(pos,
                       5., 5.);
    }
}

void ParticleEmitter::updateParticlesForFrame(const int &frame) {
    foreach(Particle *particle, mParticles) {
        if(particle->isVisibleAtFrame(frame)) {
            mParticlePos << particle->getPosAtFrame(frame);
        }
    }
}

QRectF ParticleEmitter::getParticlesBoundingRect() {
    QRectF rect;
    foreach(const QPointF &pos, mParticlePos) {
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
        p->drawEllipse(pos,
                       5., 5.);
    }
    if(rect.isNull()) return rect;
    rect.adjust(-5., -5, 5, 5);
    return rect;
}
