#include "vectorpath.h"
#include <QPainter>
#include "canvas.h"
#include <QDebug>

VectorPath::VectorPath(Canvas *canvas) : ChildParent(canvas)
{

}

VectorPath::~VectorPath()
{
    foreach(PathPoint *point, mPoints) {
        delete point;
    }
}

void VectorPath::updatePath()
{
    mPath = QPainterPath();
    mPath.setFillRule(Qt::WindingFill);
    foreach (PathPoint *firstPointInPath, mSeparatePaths) {
        PathPoint *point = NULL;
        PathPoint *lastPoint = firstPointInPath;
        mPath.moveTo(firstPointInPath->getRelativePos());
        while(true) {
            point = lastPoint->getNextPoint();
            if(point == NULL) {
                break;
            }
            QPointF pointPos = point->getRelativePos();
            mPath.cubicTo(pointPos + point->getEndCtrlPt(),
                          pointPos + point->getStartCtrlPt(),
                          pointPos);
            if(point == firstPointInPath) {
                break;
            }
            lastPoint = point;
        }
    }

    updateMappedPath();
    mCanvas->repaint();
}

void VectorPath::updateAfterTransformationChanged()
{
    return;
}

void VectorPath::updateAfterCombinedTransformationChanged()
{
    updateMappedPath();
}

void VectorPath::updateMappedPath()
{
    mMappedPath = mCombinedTransformMatrix.map(mPath);
}

bool VectorPath::isContainedIn(QRectF absRect) {
    return absRect.contains(getBoundingRect());
}

bool VectorPath::isSelected()
{
    return mSelected;
}

void VectorPath::select()
{
    mSelected = true;
}

void VectorPath::deselect()
{
    mSelected = false;
}

QRectF VectorPath::getBoundingRect()
{
    return mMappedPath.boundingRect();
}

void VectorPath::draw(QPainter *p)
{
    p->save();
    p->setBrush(QColor(200, 200, 200));
    p->drawPath(mMappedPath);
    p->restore();
}

void VectorPath::drawSelected(QPainter *p, CanvasMode currentCanvasMode)
{
    p->save();
    if(currentCanvasMode == CanvasMode::MOVE_PATH) {
        p->setPen(QPen(QColor(0, 0, 0, 125), 2, Qt::DotLine));
        p->setBrush(Qt::NoBrush);
        p->drawRect(getBoundingRect());
    } else if(currentCanvasMode == CanvasMode::MOVE_POINT) {
        p->setPen(QPen(QColor(0, 0, 0, 125), 2));
        foreach (PathPoint *point, mPoints) {
            point->draw(p);
        }
    } else if(currentCanvasMode == CanvasMode::ADD_POINT) {
        p->setPen(QPen(QColor(0, 0, 0, 125), 2));
        foreach (PathPoint *point, mPoints) {
            if(point->isEndPoint()) {
                point->draw(p);
            }
        }
    }
    p->restore();
}

PathPoint *VectorPath::getPointAt(QPointF absPtPos, CanvasMode currentCanvasMode)
{
    foreach (PathPoint *point, mPoints) {
        if(point->isPointAt(absPtPos)) {
            if(currentCanvasMode == CanvasMode::MOVE_POINT) {
                return point;
            } else if(currentCanvasMode == CanvasMode::ADD_POINT) {
                if(point->isEndPoint()) {
                    return point;
                }
            }
        }
    }
    return NULL;
}

void VectorPath::addContainedPointsToList(QRectF absRect, QList<PathPoint *> *list)
{
    foreach(PathPoint *point, mPoints) {
        if(point->isSelected()) {
            continue;
        }
        if(point->isContainedInRect(absRect)) {
            point->select();
            list->append(point);
        }
    }
}

PathPoint *VectorPath::addPoint(PathPoint *pointToAdd, PathPoint *toPoint)
{
    mPoints.append(pointToAdd);
    if(toPoint == NULL) {
        mSeparatePaths.append(pointToAdd);
    } else {
        if(!toPoint->hasNextPoint()) {
            toPoint->setPointAsNext(pointToAdd);
        } else if(!toPoint->hasPreviousPoint() ) {
            mSeparatePaths.replace(mSeparatePaths.indexOf(toPoint), pointToAdd);
            toPoint->setPointAsPrevious(pointToAdd);
        }
    }

    return pointToAdd;
}

Canvas *VectorPath::getCanvas()
{
    return mCanvas;
}

PathPoint* VectorPath::addPoint(QPointF absPtPos, PathPoint *toPoint)
{
    PathPoint *newPoint = new PathPoint(absPtPos, this);
    mPoints.append(newPoint);
    if(toPoint == NULL) {
        mSeparatePaths.append(newPoint);
    } else {
        if(!toPoint->hasNextPoint()) {
            toPoint->setPointAsNext(newPoint);
        } else if(!toPoint->hasPreviousPoint() ) {
            mSeparatePaths.replace(mSeparatePaths.indexOf(toPoint), newPoint);
            toPoint->setPointAsPrevious(newPoint);
        }
    }

    return newPoint;
}

void VectorPath::removePoint(PathPoint *point) {
    PathPoint *prevPoint = point->getPreviousPoint();
    PathPoint *nextPoint = point->getNextPoint();

    if(prevPoint != NULL) {
        prevPoint->setPointAsNext(nextPoint);
    } else {
        mSeparatePaths.replace(mSeparatePaths.indexOf(point), nextPoint);
    }
    mPoints.removeOne(point);
}

bool VectorPath::pointInsidePath(QPointF point)
{
    return mMappedPath.contains(point);
}
