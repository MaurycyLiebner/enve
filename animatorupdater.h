#ifndef ANIMATORUPDATER_H
#define ANIMATORUPDATER_H

class BoundingBox;

class VectorPath;

class AnimatorUpdater
{
public:
    AnimatorUpdater() {}
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

#endif // ANIMATORUPDATER_H
