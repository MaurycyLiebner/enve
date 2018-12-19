#include "vectorpath.h"
#include <QPainter>
#include "canvas.h"
#include <QDebug>
#include "undoredo.h"
#include "GUI/mainwindow.h"
#include "MovablePoints/pathpivot.h"
#include "pointhelpers.h"
#include "PropertyUpdaters/nodepointupdater.h"
#include "MovablePoints/nodepoint.h"
#include "Animators/pathanimator.h"
#include "Animators/PathAnimators/vectorpathanimator.h"
#include "Animators/gradientpoints.h"
#include "Animators/effectanimators.h"
#include "Animators/transformanimator.h"

VectorPath::VectorPath() :
    PathBox(BoundingBoxType::TYPE_VECTOR_PATH) {
    setName("Path");
    mPathAnimator = SPtrCreate(PathAnimator)(this);
    mPathAnimator->prp_setUpdater(SPtrCreate(NodePointUpdater)(this));
    mPathAnimator->prp_blockUpdater();
    ca_addChildAnimator(mPathAnimator);
    ca_moveChildBelow(mPathAnimator.data(), mEffectsAnimators.data());
}

void VectorPath::shiftAllPointsForAllKeys(const int &by) {
    mPathAnimator->shiftAllPointsForAllKeys(by);
}

void VectorPath::revertAllPointsForAllKeys() {
    mPathAnimator->revertAllPointsForAllKeys();
}

void VectorPath::shiftAllPoints(const int &by) {
    mPathAnimator->shiftAllPoints(by);
}

void VectorPath::revertAllPoints() {
    mPathAnimator->revertAllPoints();
}

void VectorPath::breakPathsApart() {
    QList<qsptr<VectorPathAnimator>> pathsList = mPathAnimator->getSinglePathsList();
    foreach(const qsptr<VectorPathAnimator>& path, pathsList) {
        qsptr<VectorPath> newPath = SPtrCreate(VectorPath)();
        copyPathBoxDataTo(newPath.get());
        mParentGroup->addContainedBox(newPath);
        PathAnimator *pathAnimator = newPath->getPathAnimator();
        pathAnimator->addSinglePathAnimator(path, false);
        mPathAnimator->removeSinglePathAnimator(path);
        path->setParentPath(pathAnimator);
    }
    removeFromParent();
}

bool VectorPath::differenceInEditPathBetweenFrames(
        const int &frame1, const int &frame2) const {
    return mPathAnimator->prp_differencesBetweenRelFrames(frame1, frame2);
}

void VectorPath::selectAllPoints(Canvas *canvas) {
    mPathAnimator->selectAllPoints(canvas);
}

NodePoint *VectorPath::createNewPointOnLineNear(const QPointF &absPos,
                                                const bool &adjust,
                                                const qreal &canvasScaleInv) {
    return mPathAnimator->createNewPointOnLineNear(absPos, adjust,
                                                   canvasScaleInv);
}

VectorPath::~VectorPath() {

}

PathAnimator *VectorPath::getPathAnimator() {
    return mPathAnimator.data();
}

void VectorPath::applyCurrentTransformation() {
    mNReasonsNotToApplyUglyTransform++;
    mPathAnimator->applyTransformToPoints(
                mTransformAnimator->getCurrentTransformationMatrix());

    mTransformAnimator->reset(true);
    centerPivotPosition(true);
    mNReasonsNotToApplyUglyTransform--;
}

VectorPathEdge *VectorPath::getEdge(const QPointF &absPos,
                                    const qreal &canvasScaleInv) {
    return mPathAnimator->getEdge(absPos, canvasScaleInv);
}

void VectorPath::loadPathFromSkPath(const SkPath &path) {
    mPathAnimator->loadPathFromSkPath(path);
}


//#include <QMenu>
//void VectorPath::showContextMenu(const QPoint &globalPos) {
//    QMenu menu(mMainWindow);

//    QAction *outlineScaled = new QAction("Scale outline");
//    outlineScaled->setCheckable(true);
//    outlineScaled->setChecked(mOutlineAffectedByScale);
//    menu.addAction(outlineScaled);

//    menu.addAction("Delete");
//    QAction *selected_action = menu.exec(globalPos);
//    if(selected_action != nullptr)
//    {
//        if(selected_action->text() == "Delete")
//        {

//        } else if(selected_action == outlineScaled) {
//            setOutlineAffectedByScale(!mOutlineAffectedByScale);
//        }
//    } else {

//    }
//}

void VectorPath::drawSelectedSk(SkCanvas *canvas,
                              const CanvasMode &currentCanvasMode,
                              const SkScalar &invScale) {
    if(isVisibleAndInVisibleDurationRect()) {
        canvas->save();
        drawBoundingRectSk(canvas, invScale);
        mPathAnimator->drawSelected(canvas,
                                    currentCanvasMode,
                                    invScale,
                                    QMatrixToSkMatrix(getCombinedTransform()));
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            mFillGradientPoints->drawGradientPointsSk(canvas, invScale);
            mStrokeGradientPoints->drawGradientPointsSk(canvas, invScale);
        } else if(currentCanvasMode == MOVE_PATH) {
            mTransformAnimator->getPivotMovablePoint()->
                    drawSk(canvas, invScale);
        }
        canvas->restore();
    }
}

MovablePoint *VectorPath::getPointAtAbsPos(const QPointF &absPtPos,
                                     const CanvasMode &currentCanvasMode,
                                     const qreal &canvasScaleInv) {
    MovablePoint* pointToReturn = PathBox::getPointAtAbsPos(absPtPos,
                                                            currentCanvasMode,
                                                            canvasScaleInv);
    if(pointToReturn == nullptr) {
        pointToReturn = mPathAnimator->getPointAtAbsPos(absPtPos,
                                                      currentCanvasMode,
                                                      canvasScaleInv);
    }
    return pointToReturn;
}

void VectorPath::selectAndAddContainedPointsToList(const QRectF &absRect,
                                                   QList<stdptr<MovablePoint>>& list) {
    mPathAnimator->selectAndAddContainedPointsToList(absRect, list);
}

SkPath VectorPath::getPathAtRelFrame(const int &relFrame) {
     return mPathAnimator->getPathAtRelFrame(relFrame);
}

SkPath VectorPath::getPathAtRelFrameF(const qreal &relFrame) {
     return mPathAnimator->getPathAtRelFrameF(relFrame);
}

void VectorPath::getMotionBlurProperties(QList<Property*> &list) {
    PathBox::getMotionBlurProperties(list);
    list.append(mPathAnimator.get());
}
