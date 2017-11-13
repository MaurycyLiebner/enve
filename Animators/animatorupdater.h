#ifndef ANIMATORUPDATER_H
#define ANIMATORUPDATER_H
#include "selfref.h"

class BoundingBox;

class Gradient;
class PathBox;
class BasicTransformAnimator;

class AnimatorUpdater : public StdSelfRef
{
public:
    AnimatorUpdater() {}
    virtual ~AnimatorUpdater() {}

    virtual void update() {}
    virtual void frameChangeUpdate() {
        update();
    }
    virtual void updateFinal() {}
};

class TransformUpdater : public AnimatorUpdater {
public:
    TransformUpdater(BasicTransformAnimator *transformAnimator);
    void update();
    void updateFinal();

    void frameChangeUpdate();
private:
    BasicTransformAnimator *mTarget;
};

class NodePointUpdater : public AnimatorUpdater
{
public:
    NodePointUpdater(PathBox *vectorPath);

    void update();

    void frameChangeUpdate();
private:
    PathBox *mTarget;
};

class GradientUpdater : public AnimatorUpdater
{
public:
    GradientUpdater(Gradient *gradient);

    void update();
    void updateFinal();

    void frameChangeUpdate();
private:
    Gradient *mTarget;
};

class StrokeWidthUpdater : public AnimatorUpdater
{
public:
    StrokeWidthUpdater(PathBox *path);

    void update();

    void frameChangeUpdate();
private:
    PathBox *mTarget;
};

class DisplayedFillStrokeSettingsUpdater : public AnimatorUpdater
{
public:
    DisplayedFillStrokeSettingsUpdater(BoundingBox *path);

    void update();
    void frameChangeUpdate();
    void updateFinal();
private:
    BoundingBox *mTarget;
};

class PixmapEffectUpdater : public AnimatorUpdater
{
public:
    PixmapEffectUpdater(BoundingBox *target);

    void update();
    void frameChangeUpdate();
    void updateFinal();
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
    void frameChangeUpdate() {}
private:
    ParticleEmitter *mTarget;
};

class GradientPointsUpdater : public AnimatorUpdater {
public:
    GradientPointsUpdater(const bool &isFill, PathBox *target) :
        AnimatorUpdater() {
        mTarget = target;
        mIsFill = isFill;
    }

    void update();

    void frameChangeUpdate();
private:
    PathBox *mTarget;
    bool mIsFill;
};

class SingleSound;
class SingleSoundUpdater : public AnimatorUpdater {
public:
    SingleSoundUpdater(SingleSound *sound);
    ~SingleSoundUpdater();

    void update();

    void frameChangeUpdate() {}
private:
    SingleSound *mTarget;
};

class RandomQrealGenerator;
class RandomQrealGeneratorUpdater : public AnimatorUpdater {
public:
    RandomQrealGeneratorUpdater(RandomQrealGenerator *target);

    void update();

    void frameChangeUpdate() {}
private:
    RandomQrealGenerator *mTarget;
};

class PaintBox;
class PaintBoxSizeUpdaterBR : public AnimatorUpdater {
public:
    PaintBoxSizeUpdaterBR(PaintBox *paintBox);
    ~PaintBoxSizeUpdaterBR();

    void update();
    void frameChangeUpdate();
    void updateFinal();
private:
    PaintBox *mTarget;
};

class PaintBoxSizeUpdaterTL : public AnimatorUpdater {
public:
    PaintBoxSizeUpdaterTL(PaintBox *paintBox);
    ~PaintBoxSizeUpdaterTL();

    void update();
    void frameChangeUpdate();
    void updateFinal();
private:
    PaintBox *mTarget;
};

#endif // ANIMATORUPDATER_H
