#include "ctrlpoint.h"
#include "nodepoint.h"
#include "Boxes/vectorpath.h"
#include "Boxes/boxesgroup.h"
#include "pointhelpers.h"
#include "Animators/PathAnimators/vectorpathanimator.h"
#include "nodepoint.h"

CtrlPoint::CtrlPoint(NodePoint *parentPoint, bool isStartCtrlPt) :
    NonAnimatedMovablePoint(parentPoint->getParent(),
                 MovablePointType::TYPE_CTRL_POINT, 5.) {
    mIsStartCtrlPt = isStartCtrlPt;
    mParentPoint = parentPoint;
    mParent = mParentPoint->getParent();
}

QPointF CtrlPoint::getRelativePos() const {
    return mParentPoint->getRelativePos() + mCurrentPos;
}

void CtrlPoint::setRelativePosVal(const QPointF &relPos) {
    mCurrentPos = relPos - mParentPoint->getRelativePos();
}

void CtrlPoint::setRelativePos(const QPointF &relPos) {
    setRelativePosVal(relPos);
    mParentPoint->getParentPath()->setElementPos(getPtId(),
                                                 QPointFToSkPoint(mCurrentPos));
}

void CtrlPoint::setIsStartCtrlPt(const bool &bT) {
    mIsStartCtrlPt = bT;
}

void CtrlPoint::moveToAbsWithoutUpdatingTheOther(const QPointF &absPos) {
    NonAnimatedMovablePoint::moveToAbs(absPos);
}

void CtrlPoint::rotate(const qreal &rotate) {
    QPointF savedValue = mSavedRelPos - mParentPoint->getSavedRelPos();
    QMatrix mat;
    mat.rotate(rotate);
    setRelativePos(mat.map(savedValue) + mParentPoint->getRelativePos());
}

void CtrlPoint::scale(const qreal &sx,
                      const qreal &sy) {
    QPointF savedValue = mSavedRelPos - mParentPoint->getSavedRelPos();
    QMatrix mat;
    mat.scale(sx, sy);
    setRelativePos(mat.map(savedValue) + mParentPoint->getRelativePos());
}

int CtrlPoint::getPtId() {
    if(mIsStartCtrlPt) {
        return mParentPoint->getPtId() - 1;
    }
    return mParentPoint->getPtId() + 1;
}

void CtrlPoint::moveByAbs(const QPointF &absTranslatione) {
    moveToAbs(mapRelativeToAbsolute(mSavedRelPos) +
              absTranslatione);
}

void CtrlPoint::moveToAbs(QPointF absPos) {
    NonAnimatedMovablePoint::moveToAbs(absPos);
    if(mOtherCtrlPt->isSelected()) {
        return;
    }
    mParentPoint->ctrlPointPosChanged(mIsStartCtrlPt);
}

void CtrlPoint::moveByRel(const QPointF &relTranslation) {
    NonAnimatedMovablePoint::moveByRel(relTranslation);
    if(mOtherCtrlPt->isSelected()) {
        return;
    }
    mParentPoint->ctrlPointPosChanged(mIsStartCtrlPt);
}

void CtrlPoint::startTransform() {
    NonAnimatedMovablePoint::startTransform();
    //mParentPoint->NonAnimatedMovablePoint::startTransform();
    if(mParentPoint->getCurrentCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt->NonAnimatedMovablePoint::startTransform();
    }
    mParentPoint->getParentPath()->startPathChange();
}

void CtrlPoint::finishTransform() {
    //mParentPoint->NonAnimatedMovablePoint::finishTransform();
    NonAnimatedMovablePoint::finishTransform();
    if(mParentPoint->getCurrentCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt->NonAnimatedMovablePoint::finishTransform();
    }
    mParentPoint->getParentPath()->finishedPathChange();
}

void CtrlPoint::cancelTransform() {
    //mParentPoint->NonAnimatedMovablePoint::cancelTransform();
    NonAnimatedMovablePoint::cancelTransform();
    if(mParentPoint->getCurrentCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt->NonAnimatedMovablePoint::cancelTransform();
    }
    mParentPoint->getParentPath()->cancelPathChange();
}

void CtrlPoint::setOtherCtrlPt(CtrlPoint *ctrlPt) {
    mOtherCtrlPt = ctrlPt;
}

void CtrlPoint::removeFromVectorPath() {
    mParentPoint->setCtrlPtEnabled(false, mIsStartCtrlPt);
}

bool CtrlPoint::isHidden() {
    return NonAnimatedMovablePoint::isHidden() ||
           (!mParentPoint->isNeighbourSelected() );
}
