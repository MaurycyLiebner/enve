#ifndef ANIMATORUPDATER_H
#define ANIMATORUPDATER_H
#include "smartpointertarget.h"

class BoundingBox;

class PathBox;

class Gradient;

class Circle;

class Rectangle;

class AnimatorUpdater : public SmartPointerTarget
{
public:
    AnimatorUpdater() : SmartPointerTarget() {}
    virtual ~AnimatorUpdater() {}

    virtual void update() {}
};

class TransUpdater : public AnimatorUpdater
{
public:
    TransUpdater(BoundingBox *boundingBox);

    void update();

private:
    BoundingBox *mTarget;
};

class PathPointUpdater : public AnimatorUpdater
{
public:
    PathPointUpdater(PathBox *vectorPath);

    void update();

private:
    PathBox *mTarget;
};

class GradientUpdater : public AnimatorUpdater
{
public:
    GradientUpdater(Gradient *gradient);

    void update();

private:
    Gradient *mTarget;
};

class StrokeWidthUpdater : public AnimatorUpdater
{
public:
    StrokeWidthUpdater(PathBox *path);

    void update();

private:
    PathBox *mTarget;
};

class DisplayedFillStrokeSettingsUpdater : public AnimatorUpdater
{
public:
    DisplayedFillStrokeSettingsUpdater(PathBox *path);

    void update();
private:
    PathBox *mTarget;
};

class RectangleBottomRightUpdater : public AnimatorUpdater
{
public:
    RectangleBottomRightUpdater(Rectangle *target);

    void update();

private:
    Rectangle *mTarget;
};

class PixmapEffectUpdater : public AnimatorUpdater
{
public:
    PixmapEffectUpdater(BoundingBox *target);

    void update();
private:
    BoundingBox *mTarget;
};

class AnimationBox;
class AnimationBoxFrameUpdater : public AnimatorUpdater
{
public:
    AnimationBoxFrameUpdater(AnimationBox *target);

    void update();
private:
    AnimationBox *mTarget;
};

class ParticleEmitter;
class ParticlesUpdater : public AnimatorUpdater {
public:
    ParticlesUpdater(ParticleEmitter *target);

    void update();
private:
    ParticleEmitter *mTarget;
};

#endif // ANIMATORUPDATER_H
