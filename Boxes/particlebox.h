#ifndef PARTICLEBOX_H
#define PARTICLEBOX_H
#include "boundingbox.h"

class ParticleBox;

class Particle {
public:
    Particle(ParticleBox *parentBox);
    void initializeParticle(int firstFrame,
                            int nFrames,
                            const QPointF &iniPos,
                            const QPointF &iniVel);
    void generatePathNextFrame(int frame);

    bool isVisibleAtFrame(const int &frame);
    QPointF getPosAtFrame(const int &frame);
private:
    ParticleBox *mParentBox;
    int mFirstFrame;
    int mNumberFrames;
    QPointF *mPos = NULL;

    QPointF mLastPos;
    QPointF mLastVel;
};

class ParticleEmitter {
public:
    ParticleEmitter(ParticleBox *parentBox);

    void generateParticles();

    void drawParticles(QPainter *p);
    void updateParticlesForFrame(const int &frame);
    QRectF getParticlesBoundingRect();
private:
    int mMinFrame = 0;
    int mMaxFrame = 200;

    QList<QPointF> mParticlePos;
    QList<Particle*> mParticles;
    ParticleBox *mParentBox;

    QrealAnimator mIniVelocity;
    QrealAnimator mIniVelocityVar;
    QrealAnimator mIniVelocityAngle;
    QrealAnimator mIniVelocityAngleVar;

    QrealAnimator mParticlesPerSecond;
    QrealAnimator mParticlesFrameLifetime;

    QPointFAnimator mPos;
};

class ParticleBox : public BoundingBox
{
public:
    ParticleBox(BoxesGroup *parent);
    void getAccelerationAt(const QPointF &pos,
                           const int &frame,
                           QPointF *acc);
    void updateAfterFrameChanged(int currentFrame);
    void draw(QPainter *p);
    void updateBoundingRect();
private:
    QList<ParticleEmitter*> mEmitters;
};

#endif // PARTICLEBOX_H
