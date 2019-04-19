#include "brushpolypoint.h"
#include "Animators/brushpolylineanimator.h"

void BrushPolyPoint::setRelativePos(const QPointF &pos) {
    getPosRef() = pos;
    mPolyAnim->pathChanged();
}

void BrushPolyPoint::startTransform() {
    mPolyAnim->startPathChange();
}

void BrushPolyPoint::cancelTransform() {
    mPolyAnim->cancelPathChange();
}

void BrushPolyPoint::finishTransform() {
    mPolyAnim->finishPathChange();
}
