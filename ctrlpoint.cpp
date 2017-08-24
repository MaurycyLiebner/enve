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
                                                 QPointFToSkPoint(relPos));
}

void CtrlPoint::setIsStartCtrlPt(const bool &bT) {
    mIsStartCtrlPt = bT;
}

void CtrlPoint::moveToAbsWithoutUpdatingTheOther(const QPointF &absPos) {
    NonAnimatedMovablePoint::moveToAbs(absPos);
}

void CtrlPoint::rotate(const qreal &rotate) {
    QPointF savedValue = mSavedRelPos;
    QMatrix mat;
    mat.rotate(rotate);
    mCurrentPos = mat.map(savedValue);
}

int CtrlPoint::getPtId() {
    if(mIsStartCtrlPt) {
        return mParentPoint->getPtId() - 1;
    }
    return mParentPoint->getPtId() + 1;
}

void CtrlPoint::moveByAbs(const QPointF &absTranslatione) {
    moveToAbs(mapRelativeToAbsolute(mSavedRelPos +
                                    mParentPoint->getRelativePos()) +
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
}

void CtrlPoint::finishTransform() {
    //mParentPoint->NonAnimatedMovablePoint::finishTransform();
    NonAnimatedMovablePoint::finishTransform();
    if(mParentPoint->getCurrentCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt->NonAnimatedMovablePoint::finishTransform();
    }
}

void CtrlPoint::cancelTransform() {
    //mParentPoint->NonAnimatedMovablePoint::cancelTransform();
    NonAnimatedMovablePoint::cancelTransform();
    if(mParentPoint->getCurrentCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt->NonAnimatedMovablePoint::cancelTransform();
    }
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
