#include "smartpathcollection.h"
#include "MovablePoints/pathpointshandler.h"
#include "Animators/transformanimator.h"

SmartPathCollection::SmartPathCollection() :
    SmartPathCollectionBase("paths") {}

SmartNodePoint *SmartPathCollection::createNewSubPathAtRelPos(const QPointF &relPos) {
    const auto newPath = createNewPath();
    const auto handler = GetAsPtr(newPath->getPointsHandler(),
                                  PathPointsHandler);
    return handler->addFirstNode(relPos);
}

SmartNodePoint *SmartPathCollection::createNewSubPathAtPos(const QPointF &absPos) {
    const auto newPath = createNewPath();
    const auto handler = GetAsPtr(newPath->getPointsHandler(),
                                  PathPointsHandler);
    const auto trans = handler->transform();
    const auto relPos = trans ? trans->mapAbsPosToRel(absPos) : absPos;
    return handler->addFirstNode(relPos);
}

SkPath SmartPathCollection::getPathAtRelFrame(const qreal relFrame) const {
    SkPath result;
    for(const auto& child : ca_mChildAnimators) {
        const auto path = GetAsPtr(child, SmartPathAnimator);
        const auto mode = path->getMode();
        if(mode == SmartPathAnimator::NORMAL)
            result.addPath(path->getPathAtRelFrame(relFrame));
        else {
            SkPathOp op;
            switch(mode) {
                case(SmartPathAnimator::NORMAL):
                case(SmartPathAnimator::ADD):
                    op = SkPathOp::kUnion_SkPathOp;
                    break;
                case(SmartPathAnimator::REMOVE):
                    op = SkPathOp::kDifference_SkPathOp;
                    break;
                case(SmartPathAnimator::REMOVE_REVERSE):
                    op = SkPathOp::kReverseDifference_SkPathOp;
                    break;
                case(SmartPathAnimator::INTERSECT):
                    op = SkPathOp::kIntersect_SkPathOp;
                    break;
                case(SmartPathAnimator::EXCLUDE):
                    op = SkPathOp::kXOR_SkPathOp;
                    break;
                case(SmartPathAnimator::DIVIDE):
                    const SkPath skPath = path->getPathAtRelFrame(relFrame);
                    SkPath intersect;
                    op = SkPathOp::kIntersect_SkPathOp;
                    if(!Op(result, skPath, op, &intersect))
                        RuntimeThrow("Operation Failed");
                    op = SkPathOp::kDifference_SkPathOp;
                    if(!Op(result, skPath, op, &result))
                        RuntimeThrow("Operation Failed");
                    result.addPath(intersect);
                    continue;
            }
            if(!Op(result, path->getPathAtRelFrame(relFrame), op, &result))
                RuntimeThrow("Operation Failed");
        }
    }
    result.setFillType(mFillType);
    return result;
}

void SmartPathCollection::applyTransform(const QMatrix &transform) const {
    const int iMax = ca_getNumberOfChildren() - 1;
    for(int i = 0; i <= iMax; i++) {
        const auto path = ca_getChildAt<SmartPathAnimator>(i);
        path->applyTransform(transform);
    }
}

void SmartPathCollection::loadSkPath(const SkPath &path) {
    const QList<SkPath> paths = gBreakApart(path);
    for(const auto& sPath : paths) createNewPath(sPath);
}
