#include "pathkey.h"
#include "vectorpathanimator.h"
#include "GUI/mainwindow.h"
#include "skia/skqtconversions.h"

PathKey::PathKey(VectorPathAnimator* parentAnimator) :
    InterpolationKey(parentAnimator) {}

PathKey::PathKey(const int &relFrame,
                 const SkPath &path,
                 VectorPathAnimator *parentAnimator,
                 const bool& closed) :
    PathKey(parentAnimator) {
    mRelFrame = relFrame;
    setElementsFromSkPath(path);
    mPathClosed = closed;
}

PathKey::PathKey(const int &relFrame,
                 const SkPath &path,
                 const QList<SkPoint> &elementsPos,
                 VectorPathAnimator *parentAnimator,
                 const bool &closed) :
    PathKey(parentAnimator) {
    mRelFrame = relFrame;
    mPath = path;
    mElementsPos = elementsPos;
    mPathClosed = closed;
}

PathKey::PathKey(const int &relFrame,
                 const QList<SkPoint> &elementsPos,
                 VectorPathAnimator *parentAnimator,
                 const bool &closed) :
    PathKey(parentAnimator) {
    mRelFrame = relFrame;
    mElementsPos = elementsPos;
    mPathClosed = closed;
    updatePath();
}

stdsptr<PathKey> PathKey::createNewKeyFromSubsetForPath(
        VectorPathAnimator* parentAnimator,
        const int &firstId, int count) {
    QList<SkPoint> elementsPos =
            takeElementsPosSubset(firstId, count);
    stdsptr<PathKey> newKey =
            SPtrCreate(PathKey)(mRelFrame, elementsPos,
                                parentAnimator, false);
    parentAnimator->anim_appendKey(newKey);
    return newKey;
}

void PathKey::updateAfterChangedFromInside() {
    mParentAnimator->anim_updateAfterChangedKey(this);
}

NodeSettings *PathKey::getNodeSettingsForPtId(const int &ptId) {
    return GetAsPtr(mParentAnimator, VectorPathAnimator)->
            getNodeSettingsForPtId(ptId);
}
