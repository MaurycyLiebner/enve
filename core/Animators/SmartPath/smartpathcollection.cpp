#include "smartpathcollection.h"

SmartPathCollection::SmartPathCollection() :
    ComplexAnimator("paths") {}

void SmartPathCollection::addPath(const qsptr<SmartPathAnimator> &path) {
    ca_addChildAnimator(path);
    emit pathAdded(path.get());
}

void SmartPathCollection::removePath(const qsptr<SmartPathAnimator> &path) {
    ca_removeChildAnimator(path);
    emit pathRemoved(path.get());
}

SkPath SmartPathCollection::getPathAtRelFrame(const qreal &relFrame) const {
    SkPath result;
    for(const auto& child : ca_mChildAnimators) {
        const auto path = GetAsPtr(child, SmartPathAnimator);
        result.addPath(path->getPathAtRelFrame(relFrame));
    }
    return result;
}

void SmartPathCollection::applyTransform(const QMatrix &transform) const {
    const int iMax = ca_getNumberOfChildren() - 1;
    for(int i = 0; i <= iMax; i++) {
        const auto path = ca_getChildAt<SmartPathAnimator>(i);
        path->applyTransform(transform);
    }
}

void SmartPathCollection::loadSkPath(const SkPath &path) {
    QList<SkPath> paths = gBreakApart(path);
    for(const auto& sPath : paths) createNewPath(sPath);
}
