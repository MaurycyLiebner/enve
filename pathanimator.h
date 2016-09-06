#ifndef PATHANIMATOR_H
#define PATHANIMATOR_H
#include "complexanimator.h"
#include "qpointfanimator.h"

class MovablePoint;

class PathPoint;

struct PathPointAnimators;

class PathAnimator : public ComplexAnimator
{
public:
    PathAnimator();

    void addPathPoint(PathPoint *pathPoint);
    void removePathPoint(PathPoint *pathPoint);
    void addPointPosAnimator(QrealAnimator *pointPosAnimator);
    void removePointPosAnimator(QrealAnimator *pointPosAnimator);
private:
    QList<PathPointAnimators*> mPathPointAnimators;
};

#endif // PATHANIMATOR_H
