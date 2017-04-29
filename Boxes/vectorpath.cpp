#include "Boxes/vectorpath.h"
#include <QPainter>
#include "canvas.h"
#include <QDebug>
#include "undoredo.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include "pathpivot.h"
#include "pointhelpers.h"

VectorPath::VectorPath(BoxesGroup *group) :
    PathBox(group, BoundingBoxType::TYPE_VECTOR_PATH) {
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

PathPoint *VectorPath::createNewPointOnLineNear(QPointF absPos, bool adjust) {
    return mPathAnimator->createNewPointOnLineNear(absPos, adjust);
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

VectorPath::~VectorPath()
{

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

VectorPathEdge *VectorPath::getEgde(QPointF absPos) {
    return mPathAnimator->getEgde(absPos);
}

void VectorPath::loadPathFromQPainterPath(const QPainterPath &path) {
    mPathAnimator->loadPathFromQPainterPath(path);
}

QPointF VectorPath::getRelCenterPosition() {
    return mPathAnimator->getRelCenterPosition();
}

void VectorPath::updatePath()
{
//    mPath = QPainterPath();
//    //mPath.setFillRule(Qt::WindingFill);

//    mEditPath = QPainterPath();

    mPathAnimator->updatePath();
    mPath = mPathAnimator->getCurrentPath();

    updateOutlinePath();
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

void VectorPath::drawSelected(QPainter *p,
                              const CanvasMode &currentCanvasMode) {
    if(isVisibleAndInVisibleDurationRect()) {
        p->save();

        drawBoundingRect(p);
        mPathAnimator->drawSelected(p,
                                   currentCanvasMode,
                                   getCombinedTransform());

        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            mFillGradientPoints->drawGradientPoints(p);
            mStrokeGradientPoints->drawGradientPoints(p);
        } else if(currentCanvasMode == MOVE_PATH) {
            mTransformAnimator->getPivotMovablePoint()->draw(p);
        }
        p->restore();
    }
}

MovablePoint *VectorPath::getPointAt(const QPointF &absPtPos,
                                     const CanvasMode &currentCanvasMode)
{
    MovablePoint *pointToReturn = NULL;
    if(currentCanvasMode == MOVE_POINT) {
        pointToReturn = mStrokeGradientPoints->qra_getPointAt(absPtPos);
        if(pointToReturn == NULL) {
            pointToReturn = mFillGradientPoints->qra_getPointAt(absPtPos);
        }
    } else if(currentCanvasMode == MOVE_PATH) {
        MovablePoint *pivotMovable = mTransformAnimator->getPivotMovablePoint();
        if(pivotMovable->isPointAtAbsPos(absPtPos)) {
            return pivotMovable;
        }
    }
    if(pointToReturn == NULL) {
        pointToReturn = mPathAnimator->qra_getPointAt(absPtPos,
                                                 currentCanvasMode);
    }
    return pointToReturn;
}

void VectorPath::selectAndAddContainedPointsToList(QRectF absRect,
                                                   QList<MovablePoint *> *list) {
    mPathAnimator->selectAndAddContainedPointsToList(absRect,
                                                    list);
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
