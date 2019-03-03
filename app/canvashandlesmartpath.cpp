#include "canvas.h"
#include "MovablePoints/smartnodepoint.h"
#include "Boxes/smartvectorpath.h"

void Canvas::setCurrentSmartEndPoint(SmartNodePoint * const point) {
    if(mCurrentSmartEndPoint) mCurrentSmartEndPoint->deselect();
    if(point) point->select();
    mCurrentSmartEndPoint = point;
}

void Canvas::handleAddSmartPointMousePress() {
    if(mCurrentSmartEndPoint ? mCurrentSmartEndPoint->isHidden() : false) {
        setCurrentEndPoint(nullptr);
    }
    qptr<BoundingBox> test;

    auto nodePointUnderMouse = GetAsPtr(mLastPressedPoint, SmartNodePoint);
    if(nodePointUnderMouse ? !nodePointUnderMouse->isEndPoint() : false) {
        nodePointUnderMouse = nullptr;
    }
    if(nodePointUnderMouse == mCurrentSmartEndPoint &&
            nodePointUnderMouse) return;
    if(!mCurrentSmartEndPoint && !nodePointUnderMouse) {
        const auto newPath = SPtrCreate(SmartVectorPath)();
        mCurrentBoxesGroup->addContainedBox(newPath);
        clearBoxesSelection();
        addBoxToSelection(newPath.get());
        const auto newHandler = newPath->getHandler();
        const auto node = newHandler->createNewSubPathAtPos(
                    mLastMouseEventPosRel);
        setCurrentSmartEndPoint(node);
    } else {
        if(!nodePointUnderMouse) {
            SmartNodePoint * const newPoint =
                    mCurrentSmartEndPoint->actionAddPointAbsPos(mLastMouseEventPosRel);
            //newPoint->startTransform();
            setCurrentSmartEndPoint(newPoint);
        } else if(!mCurrentSmartEndPoint) {
            setCurrentSmartEndPoint(nodePointUnderMouse);
        } else {
            //NodePointUnderMouse->startTransform();
            if(mCurrentSmartEndPoint->getTargetAnimator() ==
               nodePointUnderMouse->getTargetAnimator()) {
                mCurrentSmartEndPoint->actionConnectToNormalPoint(nodePointUnderMouse);
            } else {
//                connectPointsFromDifferentPaths(mCurrentSmartEndPoint,
//                                                nodePointUnderMouse);
            }
            setCurrentSmartEndPoint(nodePointUnderMouse);
        }
    } // pats is not null
}


void Canvas::handleAddSmartPointMouseMove() {
    if(!mCurrentSmartEndPoint) return;
    if(mFirstMouseMove) mCurrentSmartEndPoint->startTransform();
    if(mCurrentSmartEndPoint->hasNextNormalPoint() &&
       mCurrentSmartEndPoint->hasPrevNormalPoint()) {
        if(mCurrentSmartEndPoint->getCtrlsMode() != CtrlsMode::CTRLS_CORNER) {
            mCurrentSmartEndPoint->setCtrlsMode(CtrlsMode::CTRLS_CORNER);
        }
        if(mCurrentSmartEndPoint->isSeparateNodePoint()) {
            mCurrentSmartEndPoint->moveC0ToAbsPos(mLastMouseEventPosRel);
        } else {
            mCurrentSmartEndPoint->moveC2ToAbsPos(mLastMouseEventPosRel);
        }
    } else {
        if(!mCurrentSmartEndPoint->hasNextNormalPoint() &&
           !mCurrentSmartEndPoint->hasPrevNormalPoint()) {
            if(mCurrentSmartEndPoint->getCtrlsMode() != CtrlsMode::CTRLS_CORNER) {
                mCurrentSmartEndPoint->setCtrlsMode(CtrlsMode::CTRLS_CORNER);
            }
        } else {
            if(mCurrentSmartEndPoint->getCtrlsMode() != CtrlsMode::CTRLS_SYMMETRIC) {
                mCurrentSmartEndPoint->setCtrlsMode(CtrlsMode::CTRLS_SYMMETRIC);
            }
        }
        if(mCurrentSmartEndPoint->hasNextNormalPoint()) {
            mCurrentSmartEndPoint->moveC0ToAbsPos(mLastMouseEventPosRel);
        } else {
            mCurrentSmartEndPoint->moveC2ToAbsPos(mLastMouseEventPosRel);
        }
    }
}

void Canvas::handleAddSmartPointMouseRelease() {
    if(mCurrentSmartEndPoint) {
        if(!mFirstMouseMove) mCurrentSmartEndPoint->finishTransform();
        //mCurrentSmartEndPoint->prp_updateInfluenceRangeAfterChanged();
        if(!mCurrentSmartEndPoint->isEndPoint()) setCurrentEndPoint(nullptr);
    }
}
