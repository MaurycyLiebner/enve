#include "smartpathcollection.h"
#include "MovablePoints/pathpointshandler.h"
#include "Animators/transformanimator.h"

SmartPathCollection::SmartPathCollection() :
    SmartPathCollectionBase("paths") {}

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
        result.addPath(path->getPathAtRelFrame(relFrame));
    }
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
