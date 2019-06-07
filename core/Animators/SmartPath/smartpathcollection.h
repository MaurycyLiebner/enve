#ifndef SMARTPATHCOLLECTION_H
#define SMARTPATHCOLLECTION_H
#include <QList>
#include "Animators/complexanimator.h"
#include "Animators/SmartPath/smartpathanimator.h"
#include "MovablePoints/segment.h"

class SmartPathAnimator;
class SmartNodePoint;

class SmartPathCollection : public ComplexAnimator {
    friend class SelfRef;
    Q_OBJECT
protected:
    SmartPathCollection();
public:
    bool SWT_isSmartPathCollection() const { return true; }

    void readProperty(QIODevice * const src) {
       int nPaths;
       src->read(rcChar(&nPaths), sizeof(int));
       for(int i = 0; i < nPaths; i++)
           createNewPath()->readProperty(src);
    }

    void writeProperty(QIODevice * const dst) const {
        const int nPaths = ca_getNumberOfChildren();
        dst->write(rcConstChar(&nPaths), sizeof(int));
        for(int i = 0; i < nPaths; i++) {
            const auto path = ca_getChildAt<SmartPathAnimator>(i);
            path->writeProperty(dst);
        }
    }

    template<typename... Args>
    SmartPathAnimator *createNewPath(Args && ...arguments) {
        const auto newPath = SPtrCreate(SmartPathAnimator)(arguments...);
        addPath(newPath);
        return newPath.get();
    }

    SmartNodePoint * createNewSubPathAtPos(const QPointF &absPos);

    void addPath(const qsptr<SmartPathAnimator>& path);
    void removePath(const qsptr<SmartPathAnimator>& path);
    void moveAllFrom(SmartPathCollection * const from) {
        const int iMax = from->ca_getNumberOfChildren() - 1;
        for(int i = iMax; i >= 0; i--)
            addPath(from->ca_takeChildAt<SmartPathAnimator>(i));
    }

    SkPath getPathAtRelFrame(const qreal relFrame) const;

    void applyTransform(const QMatrix &transform) const;

    void loadSkPath(const SkPath& path);
signals:
    void pathAdded(SmartPathAnimator*);
    void pathRemoved(SmartPathAnimator*);
};

#endif // SMARTPATHCOLLECTION_H
