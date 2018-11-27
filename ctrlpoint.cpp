#include "ctrlpoint.h"
#include "nodepoint.h"
#include "Boxes/vectorpath.h"
#include "Boxes/boxesgroup.h"
#include "pointhelpers.h"
#include "Animators/PathAnimators/vectorpathanimator.h"
#include "nodepoint.h"

CtrlPoint::CtrlPoint(NodePoint *parentPoint, bool isStartCtrlPt) :
    NonAnimatedMovablePoint(parentPoint->getParentTransform(),
                 MovablePointType::TYPE_CTRL_POINT, 5.),
    mParentPoint_k(parentPoint) {
    mIsStartCtrlPt = isStartCtrlPt;
    mParentTransform_cv = mParentPoint_k->getParentTransform();
}

QPointF CtrlPoint::getRelativePos() const {
    return mParentPoint_k->getRelativePos() + mCurrentPos;
}

void CtrlPoint::setRelativePosVal(const QPointF &relPos) {
    NonAnimatedMovablePoint::setRelativePosVal(
                relPos - mParentPoint_k->getRelativePos());
}

void CtrlPoint::setRelativePos(const QPointF &relPos) {
    setRelativePosVal(relPos);
    mParentPoint_k->getParentPath()->setElementPos(getPtId(),
                                                 QPointFToSkPoint(mCurrentPos));
}

void CtrlPoint::setIsStartCtrlPt(const bool &bT) {
    mIsStartCtrlPt = bT;
}

void CtrlPoint::moveToAbsWithoutUpdatingTheOther(const QPointF &absPos) {
    NonAnimatedMovablePoint::moveToAbs(absPos);
}

void CtrlPoint::rotate(const qreal &rotate) {
    QPointF savedValue = mSavedRelPos - mParentPoint_k->getSavedRelPos();
    QMatrix mat;
    mat.rotate(rotate);
    setRelativePos(mat.map(savedValue) + mParentPoint_k->getRelativePos());
}

void CtrlPoint::scale(const qreal &sx,
                      const qreal &sy) {
    QPointF savedValue = mSavedRelPos - mParentPoint_k->getSavedRelPos();
    QMatrix mat;
    mat.scale(sx, sy);
    setRelativePos(mat.map(savedValue) + mParentPoint_k->getRelativePos());
}

int CtrlPoint::getPtId() {
    if(mIsStartCtrlPt) {
        return mParentPoint_k->getPtId() - 1;
    }
    return mParentPoint_k->getPtId() + 1;
}

void CtrlPoint::moveByAbs(const QPointF &absTranslatione) {
    moveToAbs(mapRelativeToAbsolute(mSavedRelPos) +
              absTranslatione);
}

void CtrlPoint::moveToAbs(QPointF absPos) {
    NonAnimatedMovablePoint::moveToAbs(absPos);
    if(mOtherCtrlPt_cv->isSelected()) {
        return;
    }
    mParentPoint_k->ctrlPointPosChanged(mIsStartCtrlPt);
}

void CtrlPoint::moveByRel(const QPointF &relTranslation) {
    NonAnimatedMovablePoint::moveByRel(relTranslation);
    if(mOtherCtrlPt_cv->isSelected()) {
        return;
    }
    mParentPoint_k->ctrlPointPosChanged(mIsStartCtrlPt);
}

void CtrlPoint::startTransform() {
    NonAnimatedMovablePoint::startTransform();
    //mParentPoint->NonAnimatedMovablePoint::startTransform();
    if(mParentPoint_k->getCurrentCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt_cv->NonAnimatedMovablePoint::startTransform();
    }
    mParentPoint_k->getParentPath()->startPathChange();
}

void CtrlPoint::finishTransform() {
    //mParentPoint->NonAnimatedMovablePoint::finishTransform();
    NonAnimatedMovablePoint::finishTransform();
    if(mParentPoint_k->getCurrentCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt_cv->NonAnimatedMovablePoint::finishTransform();
    }
    mParentPoint_k->getParentPath()->finishedPathChange();
}

void CtrlPoint::cancelTransform() {
    //mParentPoint->NonAnimatedMovablePoint::cancelTransform();
    NonAnimatedMovablePoint::cancelTransform();
    if(mParentPoint_k->getCurrentCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt_cv->NonAnimatedMovablePoint::cancelTransform();
    }
    mParentPoint_k->getParentPath()->cancelPathChange();
}

void CtrlPoint::setOtherCtrlPt(CtrlPoint *ctrlPt) {
    mOtherCtrlPt_cv = ctrlPt;
}

void CtrlPoint::removeFromVectorPath() {
    mParentPoint_k->setCtrlPtEnabled(false, mIsStartCtrlPt);
}

bool CtrlPoint::isHidden() {
    return NonAnimatedMovablePoint::isHidden() ||
           (!mParentPoint_k->isNeighbourSelected() );
}
