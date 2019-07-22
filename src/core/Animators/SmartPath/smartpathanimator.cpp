#include "smartpathanimator.h"
#include "Animators/qrealpoint.h"
#include "smartpathcollection.h"
#include "MovablePoints/pathpointshandler.h"

SmartPathAnimator::SmartPathAnimator() :
    GraphAnimator("path"),
    startPathChange([this]() { startPathChangeExec(); }),
    pathChanged([this]() { pathChangedExec(); }),
    cancelPathChange([this]() { cancelPathChangeExec(); }),
    finishPathChange([this]() { finishPathChangeExec(); }) {
    setPointsHandler(SPtrCreate(PathPointsHandler)(this));
}

SmartPathAnimator::SmartPathAnimator(const SkPath &path) :
    SmartPathAnimator() {
    mBaseValue.setPath(path);
    mCurrentPath = path;
    updateAllPoints();
}

SmartPathAnimator::SmartPathAnimator(const SmartPath &baseValue) :
    SmartPathAnimator() {
    mBaseValue = baseValue;
    mPathUpToDate = false;
    updateAllPoints();
}

void SmartPathAnimator::readProperty(QIODevice * const src) {
    readKeys(src);
    gRead(src, mBaseValue);
    prp_afterWholeInfluenceRangeChanged();
    updateAllPoints();
}

void SmartPathAnimator::graph_getValueConstraints(
        GraphKey *key, const QrealPointType &type,
        qreal &minValue, qreal &maxValue) const {
    if(type == QrealPointType::KEY_POINT) {
        minValue = key->getRelFrame();
        maxValue = minValue;
        //getFrameConstraints(key, type, minValue, maxValue);
    } else {
        minValue = -DBL_MAX;
        maxValue = DBL_MAX;
    }
}

void SmartPathAnimator::actionDisconnectNodes(const int node1Id,
                                              const int node2Id) {
    for(const auto &key : anim_mKeys) {
        const auto spKey = GetAsPtr(key, SmartPathKey);
        auto& keyPath = spKey->getValue();
        keyPath.actionDisconnectNodes(node1Id, node2Id);
    }
    mBaseValue.actionDisconnectNodes(node1Id, node2Id);
    prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::updateAllPoints() {
    const auto handler = getPointsHandler();
    const auto pathHandler = GetAsPtr(handler, PathPointsHandler);
    pathHandler->updateAllPoints();
}
