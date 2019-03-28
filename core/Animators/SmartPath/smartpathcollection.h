#ifndef SMARTPATHCOLLECTION_H
#define SMARTPATHCOLLECTION_H
#include <QList>
#include "Animators/complexanimator.h"

class SmartPathAnimator;

class SmartPathCollection : public ComplexAnimator {
    friend class SelfRef;
protected:
    SmartPathCollection();
public:
    bool SWT_isSmartPathCollection() const { return true; }

    SmartPathAnimator *createNewPath();
    SkPath getPathAtRelFrame(const qreal &relFrame);
};

#endif // SMARTPATHCOLLECTION_H
