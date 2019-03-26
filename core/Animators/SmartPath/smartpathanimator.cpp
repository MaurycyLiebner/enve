#include "smartpathanimator.h"
#include "../qrealpoint.h"
#include "smartpathcollection.h"

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
        spKey->getValue().actionDisconnectNodes(node1Id, node2Id);
    }
    mBaseValue.actionDisconnectNodes(node1Id, node2Id);
    prp_updateInfluenceRangeAfterChanged();
}

SmartPathAnimator::SmartPathAnimator() :
    GraphAnimator("path") {}

SmartPathAnimator::SmartPathAnimator(const SmartPath &baseValue) :
    GraphAnimator("path"), mBaseValue(baseValue) {}
