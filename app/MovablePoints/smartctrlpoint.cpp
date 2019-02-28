#include "smartctrlpoint.h"
#include "smartnodepoint.h"
#include "Boxes/vectorpath.h"
#include "Boxes/boxesgroup.h"
#include "pointhelpers.h"
#include "Animators/SmartPath/smartpathanimator.h"

SmartCtrlPoint::SmartCtrlPoint(SmartNodePoint * const parentPoint,
                               const Type& type) :
    NonAnimatedMovablePoint(parentPoint->getParentTransform(),
                 MovablePointType::TYPE_CTRL_POINT, 5),
    mCtrlType(type), mParentPoint_k(parentPoint) {
    mParentTransform_cv = mParentPoint_k->getParentTransform();
}

QPointF SmartCtrlPoint::getRelativePos() const {
    return mParentPoint_k->getRelativePos() + mCurrentPos;
}

void SmartCtrlPoint::setRelativePosVal(const QPointF &relPos) {
    NonAnimatedMovablePoint::setRelativePosVal(
                relPos - mParentPoint_k->getRelativePos());
}

void SmartCtrlPoint::setRelativePos(const QPointF &relPos) {
    setRelativePosVal(relPos);
    if(mCtrlType == C0) mParentPoint_k->c0Moved(mCurrentPos);
    else mParentPoint_k->c2Moved(mCurrentPos);
}

void SmartCtrlPoint::moveToAbsWithoutUpdatingTheOther(const QPointF &absPos) {
    NonAnimatedMovablePoint::moveToAbs(absPos);
}

void SmartCtrlPoint::rotate(const qreal &rotate) {
    QPointF savedValue = mSavedRelPos - mParentPoint_k->getSavedRelPos();
    QMatrix mat;
    mat.rotate(rotate);
    setRelativePos(mat.map(savedValue) + mParentPoint_k->getRelativePos());
}

void SmartCtrlPoint::scale(const qreal &sx,
                      const qreal &sy) {
    QPointF savedValue = mSavedRelPos - mParentPoint_k->getSavedRelPos();
    QMatrix mat;
    mat.scale(sx, sy);
    setRelativePos(mat.map(savedValue) + mParentPoint_k->getRelativePos());
}

void SmartCtrlPoint::moveByAbs(const QPointF &absTranslatione) {
    moveToAbs(mapRelativeToAbsolute(mSavedRelPos) +
              absTranslatione);
}

void SmartCtrlPoint::moveToAbs(const QPointF& absPos) {
    NonAnimatedMovablePoint::moveToAbs(absPos);
    if(mOtherCtrlPt_cv->isSelected()) return;
    if(mCtrlType == C0) {
        mParentPoint_k->c0PtPosChanged();
    } else {
        mParentPoint_k->c2PtPosChanged();
    }
}

void SmartCtrlPoint::moveByRel(const QPointF &relTranslation) {
    NonAnimatedMovablePoint::moveByRel(relTranslation);
    if(mOtherCtrlPt_cv->isSelected()) return;
    if(mCtrlType == C0) {
        mParentPoint_k->c0PtPosChanged();
    } else {
        mParentPoint_k->c2PtPosChanged();
    }
}

void SmartCtrlPoint::startTransform() {
    NonAnimatedMovablePoint::startTransform();
    //mParentPoint->NonAnimatedMovablePoint::startTransform();
    if(mParentPoint_k->getCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt_cv->NonAnimatedMovablePoint::startTransform();
    }
    mParentPoint_k->getTargetAnimator()->startPathChange();
}

void SmartCtrlPoint::finishTransform() {
    //mParentPoint->NonAnimatedMovablePoint::finishTransform();
    NonAnimatedMovablePoint::finishTransform();
    if(mParentPoint_k->getCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt_cv->NonAnimatedMovablePoint::finishTransform();
    }
    mParentPoint_k->getTargetAnimator()->finishPathChange();
}

void SmartCtrlPoint::cancelTransform() {
    //mParentPoint->NonAnimatedMovablePoint::cancelTransform();
    NonAnimatedMovablePoint::cancelTransform();
    if(mParentPoint_k->getCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt_cv->NonAnimatedMovablePoint::cancelTransform();
    }
    mParentPoint_k->getTargetAnimator()->cancelPathChange();
}

void SmartCtrlPoint::setOtherCtrlPt(SmartCtrlPoint * const ctrlPt) {
    mOtherCtrlPt_cv = ctrlPt;
}

void SmartCtrlPoint::removeFromVectorPath() {
    if(mCtrlType == C0) {
        mParentPoint_k->setC0Enabled(false);
    } else {
        mParentPoint_k->setC2Enabled(false);
    }
}

bool SmartCtrlPoint::isHidden() const {
    return NonAnimatedMovablePoint::isHidden() ||
           !mParentPoint_k->isNeighbourNormalSelected();
}
