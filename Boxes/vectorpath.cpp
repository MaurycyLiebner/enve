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
#include "pathpoint.h"
#include "Animators/pathanimator.h"
#include "gradientpoints.h"

VectorPath::VectorPath(BoxesGroup *group) :
    PathBox(group, BoundingBoxType::TYPE_VECTOR_PATH) {
    mPathAnimator =
            (new PathAnimator(this))->ref<PathAnimator>();
    setName("Path");
    mPathAnimator->prp_setUpdater(new PathPointUpdater(this));
    mPathAnimator->prp_blockUpdater();
    ca_addChildAnimator(mPathAnimator.data());
}

#include <QSqlError>
int VectorPath::prp_saveToSql(QSqlQuery *query, const int &parentId) {
    int boundingBoxId = PathBox::prp_saveToSql(query, parentId);
    mPathAnimator->prp_saveToSql(query, boundingBoxId);

    return boundingBoxId;
}

PathPoint *VectorPath::createNewPointOnLineNear(const QPointF &absPos,
                                                const bool &adjust,
                                                const qreal &canvasScaleInv) {
    return mPathAnimator->createNewPointOnLineNear(absPos, adjust,
                                                   canvasScaleInv);
}

void VectorPath::removeChildPathAnimator(PathAnimator *path) {
    if(path == mPathAnimator.data()) {
        removeFromParent();
    } else {
        BoundingBox::removeChildPathAnimator(path);
    }
}

void VectorPath::prp_loadFromSql(const int &boundingBoxId) {
    PathBox::prp_loadFromSql(boundingBoxId);
    mPathAnimator->prp_loadFromSql(boundingBoxId);
    if(!mPivotChanged) centerPivotPosition();
}

VectorPath *VectorPath::createPathFromSql(int boundingBoxId,
                                          BoxesGroup *parent) {
    VectorPath *path = new VectorPath(parent);
    path->prp_loadFromSql(boundingBoxId);

    return path;
}

VectorPath::~VectorPath() {

}

PathAnimator *VectorPath::getPathAnimator() {
    return mPathAnimator.data();
}

void VectorPath::applyCurrentTransformation() {
    mPathAnimator->
            applyTransformToPoints(mTransformAnimator->getCurrentTransformationMatrix());

    mTransformAnimator->reset(true);
    centerPivotPosition(true);
}

VectorPathEdge *VectorPath::getEgde(const QPointF &absPos,
                                    const qreal &canvasScaleInv) {
    return mPathAnimator->getEgde(absPos, canvasScaleInv);
}

void VectorPath::loadPathFromQPainterPath(const QPainterPath &path) {
    mPathAnimator->loadPathFromQPainterPath(path);
}

void VectorPath::loadPathFromSkPath(const SkPath &path) {
    mPathAnimator->loadPathFromSkPath(path);
}

QPointF VectorPath::getRelCenterPosition() {
    return mPathAnimator->getRelCenterPosition();
}

void VectorPath::updatePath() {
//    mPath = QPainterPath();
//    //mPath.setFillRule(Qt::WindingFill);

//    mEditPath = QPainterPath();

    mPathAnimator->updatePath();
    mPath = mPathAnimator->getCurrentPath();
    mPathSk = mPathAnimator->getCurrentSkPath();

    updateOutlinePathSk();
}

#include <QMenu>
void VectorPath::showContextMenu(QPoint globalPos) {
    QMenu menu(mMainWindow);

    QAction *outlineScaled = new QAction("Scale outline");
    outlineScaled->setCheckable(true);
    outlineScaled->setChecked(mOutlineAffectedByScale);
    menu.addAction(outlineScaled);

    menu.addAction("Delete");
    QAction *selected_action = menu.exec(globalPos);
    if(selected_action != NULL)
    {
        if(selected_action->text() == "Delete")
        {

        } else if(selected_action == outlineScaled) {
            setOutlineAffectedByScale(!mOutlineAffectedByScale);
        }
    } else {

    }
}

void VectorPath::drawSelectedSk(SkCanvas *canvas,
                              const CanvasMode &currentCanvasMode,
                              const qreal &invScale) {
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
        pointToReturn = mPathAnimator->qra_getPointAt(absPtPos,
                                                      currentCanvasMode,
                                                      canvasScaleInv);
    }
    return pointToReturn;
}

void VectorPath::selectAndAddContainedPointsToList(const QRectF &absRect,
                                                   QList<MovablePoint *> *list) {
    mPathAnimator->selectAndAddContainedPointsToList(absRect, list);
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

BoundingBox *VectorPath::createNewDuplicate(BoxesGroup *parent) {
    return new VectorPath(parent);
}
