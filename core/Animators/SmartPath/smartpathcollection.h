#ifndef SMARTPATHCOLLECTION_H
#define SMARTPATHCOLLECTION_H
#include <QList>
#include "Animators/complexanimator.h"
#include "Animators/SmartPath/smartpathanimator.h"

class SmartPathAnimator;

class SmartPathCollection : public ComplexAnimator {
    friend class SelfRef;
    Q_OBJECT
protected:
    SmartPathCollection();
public:
    bool SWT_isSmartPathCollection() const { return true; }

    template<typename... Args>
    SmartPathAnimator *createNewPath(Args && ...arguments) {
        const auto newPath = SPtrCreate(SmartPathAnimator)(arguments...);
        addPath(newPath);
        return newPath.get();
    }

    void addPath(const qsptr<SmartPathAnimator>& path);
    void removePath(const qsptr<SmartPathAnimator>& path);

    SkPath getPathAtRelFrame(const qreal &relFrame) const;

    void applyTransform(const QMatrix &transform) const;

    void loadSkPath(const SkPath& path);
signals:
    void pathAdded(SmartPathAnimator*);
    void pathRemoved(SmartPathAnimator*);
};

#endif // SMARTPATHCOLLECTION_H
