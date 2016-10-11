#ifndef ANIMATORUPDATER_H
#define ANIMATORUPDATER_H
#include "smartpointertarget.h"

class BoundingBox;

class VectorPath;

class Gradient;

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
    PathPointUpdater(VectorPath *vectorPath);

    void update();

private:
    VectorPath *mTarget;
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
    StrokeWidthUpdater(VectorPath *path);

    void update();

private:
    VectorPath *mTarget;
};

class DisplayedFillStrokeSettingsUpdater : public AnimatorUpdater
{
public:
    DisplayedFillStrokeSettingsUpdater();

    void update();
};

#endif // ANIMATORUPDATER_H
