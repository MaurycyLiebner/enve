#include "smartpathcollectionhandler.h"
#include "Animators/SmartPath/smartpathcollection.h"
#include "MovablePoints/pathpointshandler.h"
#include "MovablePoints/segment.h"

SmartPathCollectionHandler::SmartPathCollectionHandler(
        BasicTransformAnimator * const parentTransform) :
    mAnimator(SPtrCreate(SmartPathCollection)()),
    mParentTransform(parentTransform) {

}

PathPointsHandler *SmartPathCollectionHandler::createNewPath() {
    const auto newAnimator = mAnimator->createNewPath();
    const auto newHandler = SPtrCreate(PathPointsHandler)(
                newAnimator, mParentTransform);
    mPointsHandlers.append(newHandler);
    return newHandler.get();
}

NormalSegment SmartPathCollectionHandler::getNormalSegmentAtAbsPos(
        const QPointF &absPos, const qreal &canvasScaleInv) {
    for(const auto& handler : mPointsHandlers) {
        const auto pt = handler->getPointAtAbsPos(
                    absPtPos, currentCanvasMode, canvasScaleInv);
        if(pt) return pt;
    }
    return nullptr;
}

MovablePoint *SmartPathCollectionHandler::getPointAtAbsPos(
        const QPointF &absPtPos, const CanvasMode &currentCanvasMode,
        const qreal &canvasScaleInv) const {
    for(const auto& handler : mPointsHandlers) {
        const auto pt = handler->getPointAtAbsPos(
                    absPtPos, currentCanvasMode, canvasScaleInv);
        if(pt) return pt;
    }
    return nullptr;
}

void SmartPathCollectionHandler::selectAndAddContainedPointsToList(
        const QRectF &absRect, QList<stdptr<MovablePoint> > &list) const {

}

void SmartPathCollectionHandler::drawPoints(
        SkCanvas * const canvas, const CanvasMode &currentCanvasMode,
        const SkScalar &invScale, const SkMatrix &combinedTransform) const {

}

SkPath SmartPathCollectionHandler::getPathAtRelFrame(const qreal &relFrame) {

}
