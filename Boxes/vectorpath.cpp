#include "vectorpath.h"
#include <QPainter>
#include "canvas.h"
#include <QDebug>
#include "undoredo.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include "pathpivot.h"
#include "pointhelpers.h"
#include "Animators/animatorupdater.h"
#include "nodepoint.h"
#include "Animators/pathanimator.h"
#include "gradientpoints.h"

VectorPath::VectorPath() :
    PathBox(BoundingBoxType::TYPE_VECTOR_PATH) {
    mPathAnimator =
            (new PathAnimator(this))->ref<PathAnimator>();
    setName("Path");
    mPathAnimator->prp_setUpdater(new NodePointUpdater(this));
    mPathAnimator->prp_blockUpdater();
    ca_addChildAnimator(mPathAnimator.data());
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
    mPathAnimator->applyTransformToPoints(
                mTransformAnimator->getCurrentTransformationMatrix());

    mTransformAnimator->reset(true);
    centerPivotPosition(true);
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
//    if(selected_action != NULL)
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
    MovablePoint *pointToReturn = PathBox::getPointAtAbsPos(absPtPos,
                                                            currentCanvasMode,
                                                            canvasScaleInv);
    if(pointToReturn == NULL) {
        pointToReturn = mPathAnimator->getPointAtAbsPos(absPtPos,
                                                      currentCanvasMode,
                                                      canvasScaleInv);
    }
    return pointToReturn;
}

void VectorPath::selectAndAddContainedPointsToList(const QRectF &absRect,
                                                   QList<MovablePoint *> *list) {
    mPathAnimator->selectAndAddContainedPointsToList(absRect, list);
}

SkPath VectorPath::getPathAtRelFrame(const int &relFrame) {
     return mPathAnimator->getPathAtRelFrame(relFrame);
}

void VectorPath::duplicatePathAnimatorFrom(
        PathAnimator *source) {
    source->duplicatePathsTo(mPathAnimator.data());
}

void VectorPath::makeDuplicate(Property *targetBox) {
    PathBox::makeDuplicate(targetBox);
    ((VectorPath*)targetBox)->
            duplicatePathAnimatorFrom(mPathAnimator.data());
}

BoundingBox *VectorPath::createNewDuplicate() {
    return new VectorPath();
}
