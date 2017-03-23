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
    mPathAnimator.setParentBox(this);
    addActiveAnimator(&mPathAnimator);
    mPathAnimator.setUpdater(new PathPointUpdater(this));

    mPathAnimator.blockPointer();
}

#include <QSqlError>
int VectorPath::saveToSql(QSqlQuery *query, int parentId) {
    int boundingBoxId = PathBox::saveToSql(query, parentId);
    mPathAnimator.savePointsToSql(query, boundingBoxId);

    return boundingBoxId;
}

PathPoint *VectorPath::createNewPointOnLineNear(QPointF absPos, bool adjust) {
    return mPathAnimator.createNewPointOnLineNear(absPos, adjust);
}

void VectorPath::removeChildPathAnimator(PathAnimator *path) {
    if(path == &mPathAnimator) {
        removeFromParent();
    } else {
        BoundingBox::removeChildPathAnimator(path);
    }
}

void VectorPath::loadFromSql(int boundingBoxId) {
    PathBox::loadFromSql(boundingBoxId);
    mPathAnimator.loadPointsFromSql(boundingBoxId);
    if(!mPivotChanged) centerPivotPosition();
}

VectorPath *VectorPath::createPathFromSql(int boundingBoxId,
                                          BoxesGroup *parent) {
    VectorPath *path = new VectorPath(parent);
    path->loadFromSql(boundingBoxId);

    return path;
}

VectorPath::~VectorPath()
{

}

PathAnimator *VectorPath::getPathAnimator() {
    return &mPathAnimator;
}

void VectorPath::applyCurrentTransformation() {
    mPathAnimator.
            applyTransformToPoints(mTransformAnimator.getCurrentValue());

    mTransformAnimator.reset(true);
    centerPivotPosition(true);
}

Edge *VectorPath::getEgde(QPointF absPos) {
    return mPathAnimator.getEgde(absPos);
}

void VectorPath::loadPathFromQPainterPath(const QPainterPath &path) {
    mPathAnimator.loadPathFromQPainterPath(path);
}

QPointF VectorPath::getRelCenterPosition() {
    return mPathAnimator.getRelCenterPosition();
}

void VectorPath::updatePath()
{
//    mPath = QPainterPath();
//    //mPath.setFillRule(Qt::WindingFill);

//    mEditPath = QPainterPath();

    mPathAnimator.updatePath();
    mPath = mPathAnimator.getCurrentPath();

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
        drawBoundingRect(p);
        mPathAnimator.drawSelected(p,
                                   currentCanvasMode,
                                   mCombinedTransformMatrix);
        p->save();
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            mFillGradientPoints.drawGradientPoints(p);
            mStrokeGradientPoints.drawGradientPoints(p);
        }
        p->restore();
    }
}

MovablePoint *VectorPath::getPointAt(const QPointF &absPtPos,
                                     const CanvasMode &currentCanvasMode)
{
    MovablePoint *pointToReturn = NULL;
    if(currentCanvasMode == MOVE_POINT) {
        pointToReturn = mStrokeGradientPoints.getPointAt(absPtPos);
        if(pointToReturn == NULL) {
            pointToReturn = mFillGradientPoints.getPointAt(absPtPos);
        }
    }
    if(pointToReturn == NULL) {
        pointToReturn = mPathAnimator.getPointAt(absPtPos,
                                                 currentCanvasMode);
    }
    return pointToReturn;
}

void VectorPath::selectAndAddContainedPointsToList(QRectF absRect,
                                                   QList<MovablePoint *> *list) {
    mPathAnimator.selectAndAddContainedPointsToList(absRect,
                                                    list);
}

void VectorPath::duplicatePathAnimatorFrom(
        PathAnimator *source) {
    source->duplicatePathsTo(&mPathAnimator);
}

void VectorPath::makeDuplicate(BoundingBox *targetBox) {
    BoundingBox::makeDuplicate(targetBox);
    PathBox::makeDuplicate(targetBox);
    ((VectorPath*)targetBox)->
            duplicatePathAnimatorFrom(&mPathAnimator);
}

BoundingBox *VectorPath::createNewDuplicate(BoxesGroup *parent) {
    return new VectorPath(parent);
}
