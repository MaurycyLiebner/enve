#include "nodepointvalues.h"


NodePointValues operator-(const NodePointValues &ppv1,
                          const NodePointValues &ppv2) {
    return NodePointValues(ppv1.fC0 - ppv2.fC0,
                           ppv1.fP1 - ppv2.fP1,
                           ppv1.fC2 - ppv2.fC2);
}

NodePointValues operator+(const NodePointValues &ppv1,
                          const NodePointValues &ppv2) {
    return NodePointValues(ppv1.fC0 + ppv2.fC0,
                           ppv1.fP1 + ppv2.fP1,
                           ppv1.fC2 + ppv2.fC2);
}

NodePointValues operator/(const NodePointValues &ppv,
                          const qreal &val) {
    qreal invVal = 1./val;
    return NodePointValues(ppv.fC0 * invVal,
                           ppv.fP1 * invVal,
                           ppv.fC2 * invVal);
}

NodePointValues operator*(const qreal &val,
                          const NodePointValues &ppv) {
    return ppv*val;
}

NodePointValues operator*(const NodePointValues &ppv,
                          const qreal &val) {
    return NodePointValues(ppv.fC0 * val,
                           ppv.fP1 * val,
                           ppv.fC2 * val);
}
