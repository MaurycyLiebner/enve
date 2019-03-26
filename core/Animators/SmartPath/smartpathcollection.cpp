#include "smartpathcollection.h"
#include "Animators/SmartPath/smartpathanimator.h"

SmartPathCollection::SmartPathCollection() :
    ComplexAnimator("paths") {}

SmartPathAnimator *SmartPathCollection::createNewPath() {
    const auto newPath = SPtrCreate(SmartPathAnimator)();
    ca_addChildAnimator(newPath);
    return newPath.get();
}

SkPath SmartPathCollection::getPathAtRelFrame(const qreal &relFrame) {
    SkPath result;
    for(const auto& child : ca_mChildAnimators) {
        const auto path = GetAsPtr(child, SmartPathAnimator);
        result.addPath(path->getPathAtRelFrame(relFrame));
    }
    return result;
}
