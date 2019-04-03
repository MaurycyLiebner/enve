#include "smartctrlpoint.h"
#include "smartnodepoint.h"
#include "Boxes/boxesgroup.h"
#include "pointhelpers.h"
#include "Animators/SmartPath/smartpathanimator.h"
#include "Animators/transformanimator.h"

SmartCtrlPoint::SmartCtrlPoint(SmartNodePoint * const parentPoint,
                               const Type& type) :
    NonAnimatedMovablePoint(parentPoint->getParentTransform(),
                            MovablePointType::TYPE_CTRL_POINT, 5),
    mCtrlType(type), mParentPoint_k(parentPoint) {
    disableSelection();
}

void SmartCtrlPoint::setRelativePos(const QPointF &relPos) {
    NonAnimatedMovablePoint::setRelativePos(relPos);
    if(mCtrlType == C0) mParentPoint_k->c0Moved(getRelativePos());
    else mParentPoint_k->c2Moved(getRelativePos());
}

void SmartCtrlPoint::rotateRelativeToSavedPivot(const qreal &rotate) {
    const QPointF savedValue = getSavedRelPos() - mParentPoint_k->getSavedRelPos();
    QMatrix mat;
    mat.rotate(rotate);
    setRelativePos(mat.map(savedValue) + mParentPoint_k->getRelativePos());
}

void SmartCtrlPoint::scale(const qreal &sx, const qreal &sy) {
    const QPointF savedValue = getSavedRelPos() - mParentPoint_k->getSavedRelPos();
    QMatrix mat;
    mat.scale(sx, sy);
    setRelativePos(mat.map(savedValue) + mParentPoint_k->getRelativePos());
}

void SmartCtrlPoint::startTransform() {
    NonAnimatedMovablePoint::startTransform();
    if(mParentPoint_k->getCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt_cv->NonAnimatedMovablePoint::startTransform();
    }
    mParentPoint_k->getTargetAnimator()->startPathChange();
}

void SmartCtrlPoint::finishTransform() {
    if(mParentPoint_k->getCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt_cv->NonAnimatedMovablePoint::finishTransform();
    }
    mParentPoint_k->getTargetAnimator()->finishPathChange();
}

void SmartCtrlPoint::cancelTransform() {
    NonAnimatedMovablePoint::cancelTransform();
    if(mParentPoint_k->getCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt_cv->NonAnimatedMovablePoint::cancelTransform();
    }
    mParentPoint_k->getTargetAnimator()->cancelPathChange();
}

void SmartCtrlPoint::setOtherCtrlPt(SmartCtrlPoint * const ctrlPt) {
    mOtherCtrlPt_cv = ctrlPt;
}

void SmartCtrlPoint::remove() {
    if(mCtrlType == C0) mParentPoint_k->setC0Enabled(false);
    else mParentPoint_k->setC2Enabled(false);
}

bool SmartCtrlPoint::isHidden() const {
    if(mCtrlType == C0) return NonAnimatedMovablePoint::isHidden() ||
                               !mParentPoint_k->isPrevNormalSelected();
    if(mCtrlType == C2) return NonAnimatedMovablePoint::isHidden() ||
                               !mParentPoint_k->isNextNormalSelected();
    return true;
}
