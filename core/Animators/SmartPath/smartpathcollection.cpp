#include "smartpathcollection.h"
#include "Animators/SmartPath/smartpathanimator.h"

SmartPathCollection::SmartPathCollection() :
    ComplexAnimator("paths") {}

SmartPathAnimator *SmartPathCollection::createNewPath() {
    const auto newPath = SPtrCreate(SmartPathAnimator)();
    mPaths.append(newPath);
    return newPath.get();
}

SkPath SmartPathCollection::getPathAtRelFrame(const qreal &relFrame) {
    SkPath result;
    for(const auto& path : mPaths) {
        result.addPath(path->getPathAtRelFrame(relFrame));
    }
    return result;
}
