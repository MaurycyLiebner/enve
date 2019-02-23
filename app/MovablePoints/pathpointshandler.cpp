#include "pathpointshandler.h"
#include "canvas.h"
#include "Animators/SmartPath/smartpathanimator.h"

PathPointsHandler::PathPointsHandler(
        SmartPathAnimator * const targetAnimator,
        BasicTransformAnimator * const parentTransform) :
    mCurrentTarget(targetAnimator->getCurrentlyEditedPath()),
    mTargetAnimator(targetAnimator),
    mParentTransform(parentTransform) {}

MovablePoint *PathPointsHandler::getPointAtAbsPos(
        const QPointF &absPtPos,
        const CanvasMode &currentCanvasMode,
        const qreal &canvasScaleInv) const {
    for(const auto& point : mPoints) {
        const auto pointToReturn = point->getPointAtAbsPos(
                    absPtPos, currentCanvasMode, canvasScaleInv);
        if(!pointToReturn) continue;
        return pointToReturn;
    }
    return nullptr;
}

void PathPointsHandler::selectAndAddContainedPointsToList(
        const QRectF &absRect, QList<stdptr<MovablePoint>> &list) const {
    for(const auto& point : mPoints) {
        point->rectPointsSelection(absRect, list);
    }
}

void PathPointsHandler::drawSelected(SkCanvas * const canvas,
                                     const CanvasMode &currentCanvasMode,
                                     const SkScalar &invScale,
                                     const SkMatrix &combinedTransform) const {
    Q_UNUSED(combinedTransform);

    const bool keyOnCurrentFrame = mKeyOnCurrentFrame;
    if(currentCanvasMode == CanvasMode::MOVE_POINT) {
        for(int i = mPoints.count() - 1; i >= 0; i--) {
            auto point = mPoints.at(i);
            point->drawNodePoint(canvas, currentCanvasMode, invScale,
                                 keyOnCurrentFrame);
        }
    } else if(currentCanvasMode == CanvasMode::ADD_POINT) {
        for(int i = mPoints.count() - 1; i >= 0; i--) {
            auto point = mPoints.at(i);
            if(point->isEndPoint() || point->isSelected()) {
                point->drawNodePoint(canvas, currentCanvasMode, invScale,
                                     keyOnCurrentFrame);
            }
        }
    }
}

SmartNodePoint *PathPointsHandler::createNewNodePoint(const int &nodeId) {
    const auto newPt = SPtrCreate(SmartNodePoint)(nodeId, this,
                                                  mTargetAnimator,
                                                  mParentTransform);
    mPoints.insert(nodeId, newPt);
    return newPt.get();
}

void PathPointsHandler::updatePoints() {
    mPoints.clear();
    const int nodeCount = mCurrentTarget->getNodeCount();
    for(int i = 0; i < nodeCount; i++) {
        createNewNodePoint(i);
    }
}
