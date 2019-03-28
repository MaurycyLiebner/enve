#include "smartpathcollectionhandler.h"
#include "Animators/SmartPath/smartpathcollection.h"
#include "MovablePoints/pathpointshandler.h"
#include "MovablePoints/segment.h"
#include "Animators/transformanimator.h"
#include "Animators/SmartPath/smartpathanimator.h"
#include "PropertyUpdaters/smartnodepointupdater.h"
#include "PropertyUpdaters/nodepointupdater.h"
#include "Boxes/smartvectorpath.h"

SmartPathCollectionHandler::SmartPathCollectionHandler(
        BasicTransformAnimator * const parentTransform,
        SmartVectorPath* const parentPath) :
    mAnimator(SPtrCreate(SmartPathCollection)()),
    mParentTransform(parentTransform),
    mParentPath(parentPath) {
    const auto updater = SPtrCreate(NodePointUpdater)(mParentPath);
    mAnimator->prp_setOwnUpdater(updater);

    QObject::connect(mAnimator.get(), &SmartPathCollection::pathAdded,
                     [this](SmartPathAnimator * anim) {
        createHandlerForAnimator(anim);
    });
    QObject::connect(mAnimator.get(), &SmartPathCollection::pathRemoved,
                     [this](SmartPathAnimator * anim) {
        removeHandlerForAnimator(anim);
    });
}

SmartNodePoint *SmartPathCollectionHandler::createNewSubPathAtPos(
        const QPointF &pos) {
    const auto newHandler = createNewPath();
    return newHandler->addFirstNode(pos);
}

PathPointsHandler *SmartPathCollectionHandler::createNewPath() {
    const auto newAnimator = mAnimator->createNewPath();
    return getHandlerForAnimator(newAnimator);
}

PathPointsHandler *SmartPathCollectionHandler::createHandlerForAnimator(
        SmartPathAnimator * const newAnimator) {
    const auto newHandler = SPtrCreate(PathPointsHandler)(
                this, newAnimator, mParentTransform);
    newHandler->updateAllPoints();
    mPointsHandlers.append(newHandler);
    return newHandler.get();
}

int SmartPathCollectionHandler::getHandlerIdForAnimator(
        const SmartPathAnimator *anim) {
    for(int i = 0; i < mPointsHandlers.count(); i++) {
        const auto& handler = mPointsHandlers.at(i);
        if(handler->getAnimator() == anim) return i;
    }
    return -1;
}

PathPointsHandler *SmartPathCollectionHandler::getHandlerForAnimator(
        const SmartPathAnimator *anim) {
    const int id = getHandlerIdForAnimator(anim);
    if(id == -1) return nullptr;
    return mPointsHandlers.at(id).get();
}

NormalSegment SmartPathCollectionHandler::getNormalSegmentAtAbsPos(
        const QPointF &absPos, const qreal &canvasScaleInv) {
    for(const auto& handler : mPointsHandlers) {
        const auto seg = handler->getNormalSegmentAtAbsPos(absPos, canvasScaleInv);
        if(seg.isValid()) return seg;
    }
    return NormalSegment();
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
        const QRectF &absRect, QList<stdptr<MovablePoint>> &list) const {
    for(const auto& handler : mPointsHandlers) {
        handler->selectAndAddContainedPointsToList(absRect, list);
    }
}

void SmartPathCollectionHandler::selectAllPoints(Canvas * const canvas) {
    for(const auto& handler : mPointsHandlers) {
        handler->selectAllPoints(canvas);
    }
}

void SmartPathCollectionHandler::drawPoints(
        SkCanvas * const canvas, const CanvasMode &currentCanvasMode,
        const SkScalar &invScale, const SkMatrix &totalTransform) const {
    for(const auto& handler : mPointsHandlers) {
        handler->drawPoints(canvas, currentCanvasMode,
                            invScale, totalTransform);
    }
}

SkPath SmartPathCollectionHandler::getPathAtRelFrame(
        const qreal &relFrame) const {
    return mAnimator->getPathAtRelFrame(relFrame);
}

qsptr<SmartPathAnimator> SmartPathCollectionHandler::takeAnimatorAt(
        const int &id) {
    const auto anim = GetAsSPtr(mAnimator->ca_getChildAt(id),
                                SmartPathAnimator);
    mAnimator->removePath(anim);
    return anim;
}

PathPointsHandler* SmartPathCollectionHandler::addAnimator(
        const qsptr<SmartPathAnimator> &anim) {
    mAnimator->addPath(anim);
    return getHandlerForAnimator(anim.get());
}

void SmartPathCollectionHandler::removeHandlerForAnimator(
        SmartPathAnimator * const anim) {
    const int id = getHandlerIdForAnimator(anim);
    if(id == -1) return;
    mPointsHandlers.removeAt(id);
}

void SmartPathCollectionHandler::moveAllFrom(
        SmartPathCollectionHandler * const from) {
    const int iMax = from->numberOfAnimators() - 1;
    for(int i = iMax; i >= 0; i--)
        addAnimator(from->takeAnimatorAt(i));
}

void SmartPathCollectionHandler::applyTransform(const QMatrix &transform) {
    mAnimator->applyTransform(transform);
}

int SmartPathCollectionHandler::numberOfAnimators() const {
    return mAnimator->ca_getNumberOfChildren();
}
