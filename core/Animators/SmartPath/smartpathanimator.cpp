#include "smartpathanimator.h"
#include "../qrealpoint.h"

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

SmartPathAnimator::SmartPathAnimator(PathAnimator * const pathAnimator) :
    GraphAnimator("smart path") {
}
