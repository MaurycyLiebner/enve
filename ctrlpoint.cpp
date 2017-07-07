#include "ctrlpoint.h"
#include "pathpoint.h"
#include "Boxes/vectorpath.h"
#include "Boxes/boxesgroup.h"
#include "pointhelpers.h"

CtrlPoint::CtrlPoint(PathPoint *parentPoint, bool isStartCtrlPt) :
    MovablePoint(parentPoint->getParent(),
                 MovablePointType::TYPE_CTRL_POINT, 5.) {
    mIsStartCtrlPt = isStartCtrlPt;
    mParentPoint = parentPoint;
    mParent = mParentPoint->getParent();
    anim_setTraceKeyOnCurrentFrame(true);
}

QPointF CtrlPoint::getRelativePos() const {
    return mParentPoint->getRelativePos() + getCurrentPointValue();
}

QPointF CtrlPoint::getRelativePosAtRelFrame(const int &frame) const {
    return mParentPoint->getRelativePosAtRelFrame(frame) +
            getCurrentPointValueAtRelFrame(frame);
}

void CtrlPoint::setRelativePos(const QPointF &relPos,
                               const bool &saveUndoRedo) {
    setCurrentPointValue(relPos - mParentPoint->getRelativePos(),
                         saveUndoRedo);
}

void CtrlPoint::setIsStartCtrlPt(const bool &bT) {
    mIsStartCtrlPt = bT;
}

void CtrlPoint::moveToAbsWithoutUpdatingTheOther(const QPointF &absPos) {
    MovablePoint::moveToAbs(absPos);
}

void CtrlPoint::rotate(const qreal &rotate) {
    QPointF savedValue = getSavedPointValue();
    QMatrix mat;
    mat.rotate(rotate);
    setCurrentPointValue(mat.map(savedValue));
}

void CtrlPoint::moveByAbs(const QPointF &absTranslatione) {
    moveToAbs(mapRelativeToAbsolute(getSavedPointValue() +
                                    mParentPoint->getCurrentPointValue()) +
              absTranslatione);
}

void CtrlPoint::moveToAbs(QPointF absPos) {
    MovablePoint::moveToAbs(absPos);
    if(mOtherCtrlPt->isSelected()) {
        return;
    }
    mParentPoint->ctrlPointPosChanged(mIsStartCtrlPt);
}

void CtrlPoint::moveByRel(const QPointF &relTranslation) {
    MovablePoint::moveByRel(relTranslation);
    if(mOtherCtrlPt->isSelected()) {
        return;
    }
    mParentPoint->ctrlPointPosChanged(mIsStartCtrlPt);
}

void CtrlPoint::startTransform() {
    MovablePoint::startTransform();
    //mParentPoint->MovablePoint::startTransform();
    if(mParentPoint->getCurrentCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt->MovablePoint::startTransform();
    }
}

void CtrlPoint::finishTransform() {
    //mParentPoint->MovablePoint::finishTransform();
    MovablePoint::finishTransform();
    if(mParentPoint->getCurrentCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt->MovablePoint::finishTransform();
    }
}

void CtrlPoint::cancelTransform() {
    //mParentPoint->MovablePoint::cancelTransform();
    MovablePoint::cancelTransform();
    if(mParentPoint->getCurrentCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt->MovablePoint::cancelTransform();
    }
}

void CtrlPoint::setOtherCtrlPt(CtrlPoint *ctrlPt) {
    mOtherCtrlPt = ctrlPt;
}

void CtrlPoint::removeFromVectorPath() {
    mParentPoint->setCtrlPtEnabled(false, mIsStartCtrlPt);
}

bool CtrlPoint::isHidden() {
    return MovablePoint::isHidden() ||
           (!mParentPoint->isNeighbourSelected() );
}
