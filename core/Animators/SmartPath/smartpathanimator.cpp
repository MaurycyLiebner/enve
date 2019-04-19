#include "smartpathanimator.h"
#include "../qrealpoint.h"
#include "smartpathcollection.h"
#include "MovablePoints/pathpointshandler.h"

SmartPathAnimator::SmartPathAnimator() :
    GraphAnimator("path") {
    setPointsHandler(SPtrCreate(PathPointsHandler)(this));
}

SmartPathAnimator::SmartPathAnimator(const SkPath &path) :
    SmartPathAnimator() {
    mBaseValue.setPath(path);
}

SmartPathAnimator::SmartPathAnimator(const SmartPath &baseValue) :
    SmartPathAnimator() {
    mBaseValue = baseValue;
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

void SmartPathAnimator::actionDisconnectNodes(const int &node1Id,
                                              const int &node2Id) {
    for(const auto &key : anim_mKeys) {
        const auto spKey = GetAsPtr(key, SmartPathKey);
        auto& keyPath = spKey->getValue();
        keyPath.actionDisconnectNodes(node1Id, node2Id);
    }
    mBaseValue.actionDisconnectNodes(node1Id, node2Id);
    prp_updateInfluenceRangeAfterChanged();
}
