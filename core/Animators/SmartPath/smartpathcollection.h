#ifndef SMARTPATHCOLLECTION_H
#define SMARTPATHCOLLECTION_H
#include <QList>
#include "Animators/complexanimator.h"

class SmartPathAnimator;

class SmartPathCollection : public ComplexAnimator {
    friend class SelfRef;
    Q_OBJECT
protected:
    SmartPathCollection();
public:
    bool SWT_isSmartPathCollection() const { return true; }

    SmartPathAnimator *createNewPath();
    void addPath(const qsptr<SmartPathAnimator>& path);
    void removePath(const qsptr<SmartPathAnimator>& path);

    SkPath getPathAtRelFrame(const qreal &relFrame) const;

    void applyTransform(const QMatrix &transform) const;
signals:
    void pathAdded(SmartPathAnimator*);
    void pathRemoved(SmartPathAnimator*);
};

#endif // SMARTPATHCOLLECTION_H
