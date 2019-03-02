#include "smartpathcollection.h"
#include "Animators/SmartPath/smartpathanimator.h"

SmartPathCollection::SmartPathCollection() :
    ComplexAnimator("paths") {}

SmartPathAnimator *SmartPathCollection::createNewPath() {
    const auto newPath = SPtrCreate(SmartPathAnimator)();
    mPaths.append(newPath);
    return newPath.get();
}
