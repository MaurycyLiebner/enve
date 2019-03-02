#ifndef SMARTPATHCOLLECTION_H
#define SMARTPATHCOLLECTION_H
#include <QList>
#include "Animators/complexanimator.h"

class SmartPathAnimator;

class SmartPathCollection : public ComplexAnimator {
public:
    SmartPathCollection();

    SmartPathAnimator *createNewPath();

    SkPath getPathAtRelFrame(const qreal &relFrame);
protected:
    QList<qsptr<SmartPathAnimator>> mPaths;
};

#endif // SMARTPATHCOLLECTION_H
