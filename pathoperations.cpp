#include "pathoperations.h"
#include "pointhelpers.h"

MinimalPathPoint::MinimalPathPoint() {}

MinimalPathPoint::MinimalPathPoint(QPointF ctrlStart, QPointF pos, QPointF ctrlEnd) {
    mPos = pos;
    mStartCtrlPos = ctrlStart;
    mEndCtrlPos = ctrlEnd;
}

MinimalPathPoint::MinimalPathPoint(MinimalPathPoint *point) {
    mPos = point->getPos();
    mStartCtrlPos = point->getStartPos();
    mEndCtrlPos = point->getEndPos();
}

void MinimalPathPoint::setNextPoint(MinimalPathPoint *point) {
    mNextPoint = point;
}

void MinimalPathPoint::setPrevPoint(MinimalPathPoint *point) {
    mPrevPoint = point;
}

MinimalPathPoint *MinimalPathPoint::getNextPoint() {
    return mNextPoint;
}

MinimalPathPoint *MinimalPathPoint::getPrevPoint() {
    return mPrevPoint;
}

void MinimalPathPoint::setStartCtrlPos(QPointF pos) {
    mStartCtrlPos = pos;
}

void MinimalPathPoint::setEndCtrlPos(QPointF pos) {
    mEndCtrlPos = pos;
}

void MinimalPathPoint::setPos(QPointF pos) {
    mPos = pos;
}

QPointF MinimalPathPoint::getStartPos() {
    return mStartCtrlPos;
}

QPointF MinimalPathPoint::getEndPos() {
    return mEndCtrlPos;
}

QPointF MinimalPathPoint::getPos() {
    return mPos;
}

bool MinimalPathPoint::isIntersection() {
    return false;
}

IntersectionPathPoint::IntersectionPathPoint() : MinimalPathPoint() {

}

IntersectionPathPoint::IntersectionPathPoint(QPointF start, QPointF pos, QPointF end) :
    MinimalPathPoint(start, pos, end) {

}

bool IntersectionPathPoint::isIntersection() {
    return true;
}

void IntersectionPathPoint::setSibling(IntersectionPathPoint *sibling) {
    mSiblingIntPoint = sibling;
}

IntersectionPathPoint *IntersectionPathPoint::getSibling() {
    return mSiblingIntPoint;
}

MinimalVectorPath::MinimalVectorPath() {

}

void MinimalVectorPath::closePath() {
    mFirstPoint->setPrevPoint(mLastPoint);
    mLastPoint->setNextPoint(mFirstPoint);
}

void FullVectorPath::generateFromPath(const QPainterPath &path) {
    MinimalPathPoint *firstPoint = NULL;
    MinimalPathPoint *lastPoint = NULL;
    bool firstOther = true;
    QPointF startCtrlPoint;
    MinimalVectorPath *currentTarget = NULL;

    for(int i = 0; i < path.elementCount(); i++) {
        const QPainterPath::Element &elem = path.elementAt(i);

        if (elem.isMoveTo()) { // move
            if(currentTarget != NULL) {
                currentTarget->generateQPainterPath();
            }
            currentTarget = new MinimalVectorPath();
            mSeparatePaths.append(currentTarget);
            lastPoint = new MinimalPathPoint(QPointF(),
                                             QPointF(elem.x, elem.y),
                                             QPointF());
            currentTarget->addPoint(lastPoint);
            firstPoint = lastPoint;
        } else if (elem.isLineTo()) { // line
            if((QPointF(elem.x, elem.y) == firstPoint->getPos()) ?
                    ((path.elementCount() > i + 1) ?
                                path.elementAt(i + 1).isMoveTo() :
                                true) :
                    false) {
                firstPoint->setPrevPoint(lastPoint);
                lastPoint->setNextPoint(firstPoint);
                lastPoint = firstPoint;
            } else {
                lastPoint = new MinimalPathPoint(QPointF(),
                                                 QPointF(elem.x, elem.y),
                                                 QPointF());
                currentTarget->addPoint(lastPoint);
            }
        } else if (elem.isCurveTo()) { // curve
            lastPoint->setEndCtrlPos(QPointF(elem.x, elem.y));
            firstOther = true;
        } else { // other
            if(firstOther) {
                startCtrlPoint = QPointF(elem.x, elem.y);
            } else {
                if((QPointF(elem.x, elem.y) == firstPoint->getPos()) ?
                        ((path.elementCount() > i + 1) ?
                                    path.elementAt(i + 1).isMoveTo() :
                                    true) :
                        false) {
                    firstPoint->setPrevPoint(lastPoint);
                    lastPoint->setNextPoint(firstPoint);
                    lastPoint = firstPoint;
                } else {
                    lastPoint = new MinimalPathPoint(QPointF(),
                                                     QPointF(elem.x, elem.y),
                                                     QPointF());
                    currentTarget->addPoint(lastPoint);
                }
                //lastPoint->setStartCtrlPtEnabled(true);
                lastPoint->setStartCtrlPos(startCtrlPoint);
            }
            firstOther = !firstOther;
        }
    }
    if(currentTarget != NULL) {
        currentTarget->generateQPainterPath();
    }
}

void MinimalVectorPath::setLastPointStart(QPointF start) {
    mLastPoint->setStartCtrlPos(start);
}

void MinimalVectorPath::setLastPointEnd(QPointF end) {
    mLastPoint->setEndCtrlPos(end);
}

void MinimalVectorPath::setLastPointPos(QPointF pos) {
    mLastPoint->setPos(pos);
}

void MinimalVectorPath::addPoint(MinimalPathPoint *point) {
    mPoints << point;
    if(mLastPoint == NULL) {
        mFirstPoint = point;
    } else {
        mLastPoint->setNextPoint(point);
        point->setPrevPoint(mLastPoint);
    }
    mLastPoint = point;
}

MinimalVectorPath::~MinimalVectorPath() {
    foreach(MinimalPathPoint *point, mPoints) {
        delete point;
    }
    foreach(MinimalPathPoint *point, mIntersectionPoints) {
        delete point;
    }
}

MinimalPathPoint *MinimalVectorPath::getFirstPoint() {
    return mFirstPoint;
}

void MinimalVectorPath::intersectWith(MinimalVectorPath *otherPath) {
    if(mPath.intersects(otherPath->getPath())) {
        MinimalPathPoint *firstPoint = otherPath->getFirstPoint();
        MinimalPathPoint *point = firstPoint;
        if(point == NULL) return;
        MinimalPathPoint *nextPoint = point->getNextPoint();
        if(nextPoint == NULL) return;
        QList<PointsBezierCubic*> otherCubics;
        do {
            otherCubics << new PointsBezierCubic(point, nextPoint);

            point = nextPoint;
            nextPoint = point->getNextPoint();
        } while(point != firstPoint);

        QList<PointsBezierCubic*> thisCubics;
        firstPoint = getFirstPoint();
        point = firstPoint;
        if(point == NULL) return;
        nextPoint = point->getNextPoint();
        if(nextPoint == NULL) return;
        do {
            thisCubics << new PointsBezierCubic(point, nextPoint);

            point = nextPoint;
            nextPoint = point->getNextPoint();
        }  while(point != firstPoint);

        foreach(PointsBezierCubic *cubic1, thisCubics) {
            foreach(PointsBezierCubic *cubic2, otherCubics) {
                cubic1->intersectWith(cubic2);
            }
            delete cubic1;
        }
        foreach(PointsBezierCubic *cubic2, otherCubics) {
            delete cubic2;
        }
        thisCubics.clear();
        otherCubics.clear();

        otherPath->removeNonIntersectionPointsContainedInAndSaveIntersectionPoints(this);
        removeNonIntersectionPointsContainedInAndSaveIntersectionPoints(otherPath);
    }
}

void MinimalVectorPath::addAllPaths(QList<MinimalVectorPath*> *targetsList) {
    if(mIntersectionPoints.isEmpty()) {
        if(mPoints.isEmpty()) return;
        MinimalVectorPath *newPath = new MinimalVectorPath();
        MinimalPathPoint *point = mFirstPoint;
        do {
            newPath->addPoint(new MinimalPathPoint(point));
            point = point->getNextPoint();
        } while(point != mFirstPoint);
        newPath->closePath();
        targetsList->append(newPath);
        return;
    }
    while(!mIntersectionPoints.isEmpty()) {
        MinimalPathPoint *firstFirstPoint = mIntersectionPoints.takeFirst();
        if(firstFirstPoint->wasAdded()) continue;
        MinimalVectorPath *target = new MinimalVectorPath();
        targetsList->append(target);
        MinimalPathPoint *point = firstFirstPoint;

        bool reversed = point->getNextPoint() == NULL;
        MinimalPathPoint *nextPoint;
        if(reversed) {
            nextPoint = point->getPrevPoint();
        } else {
            nextPoint = point->getNextPoint();
        }
        while(true) {
            while(nextPoint != NULL) {
                target->addPoint(new MinimalPathPoint(point));
                point->setAdded();

                point = nextPoint;
                if(reversed) {
                    nextPoint = point->getPrevPoint();
                } else {
                    nextPoint = point->getNextPoint();
                }
            }

            if(point->isIntersection()) {
                mIntersectionPoints.removeOne((IntersectionPathPoint*)point);
                target->addPoint(new MinimalPathPoint(point));
                point->setAdded();
                point = ((IntersectionPathPoint*)point)->getSibling();
                point->setAdded();
                if(point == firstFirstPoint) {
                    bool firstReversed = firstFirstPoint->getNextPoint() == NULL;
                    bool siblingReversed = firstFirstPoint->getNextPoint() == NULL;
                    QPointF ctrlPt;
                    if(siblingReversed) {
                        ctrlPt = ((IntersectionPathPoint*)firstFirstPoint)->
                                getSibling()->getEndPos();
                    } else {
                        ctrlPt = ((IntersectionPathPoint*)firstFirstPoint)->
                                getSibling()->getStartPos();
                    }
                    if(firstReversed) {
                        target->getFirstPoint()->setEndCtrlPos(ctrlPt);
                    } else {
                        target->getFirstPoint()->setStartCtrlPos(ctrlPt);
                    }

                    break;
                }


                bool wasReversed = reversed;
                reversed = point->getNextPoint() == NULL;
                QPointF ctrlVal;
                if(reversed) {
                    ctrlVal = point->getStartPos();

                    point = point->getPrevPoint();
                    nextPoint = point->getPrevPoint();
                } else {
                    ctrlVal = point->getEndPos();

                    point = point->getNextPoint();
                    nextPoint = point->getNextPoint();
                }
                if(wasReversed) {
                    target->setLastPointStart(ctrlVal);
                } else {
                    target->setLastPointEnd(ctrlVal);
                }
            }
        }
        target->closePath();
    }
}

void MinimalVectorPath::
removeNonIntersectionPointsContainedInAndSaveIntersectionPoints(
        MinimalVectorPath *path) {
    QList<MinimalPathPoint*> pointsToDelete;
    MinimalPathPoint *point = mFirstPoint;
    do {
        if(point->isIntersection()) {
            mIntersectionPoints << (IntersectionPathPoint*)point;
        } else {
            if(path->pointInsidePath(point->getPos()) ) {
                pointsToDelete << point;
            }
        }
        point = point->getNextPoint();
    } while(point != mFirstPoint);
    foreach(MinimalPathPoint *point, pointsToDelete) {
        MinimalPathPoint *nextPoint = point->getNextPoint();
        if(nextPoint != NULL) {
            nextPoint->setPrevPoint(NULL);
        }
        MinimalPathPoint *prevPoint = point->getPrevPoint();
        if(prevPoint != NULL) {
            prevPoint->setNextPoint(NULL);
        }
        mPoints.removeOne(point);
        delete point;
    }

    foreach(IntersectionPathPoint *point, mIntersectionPoints) {
        MinimalPathPoint *next = point->getNextPoint();
        if(next != NULL) {
            if(next->isIntersection()) {
                point->setNextPoint(NULL);
                next->setPrevPoint(NULL);
            }
        }
        MinimalPathPoint *prev = point->getPrevPoint();
        if(prev != NULL) {
            if(prev->isIntersection()) {
                point->setPrevPoint(NULL);
                prev->setNextPoint(NULL);
            }
        }
    }

    mFirstPoint = NULL;
}

void MinimalVectorPath::generateQPainterPath() {
    MinimalPathPoint *point = mFirstPoint;
    QPointF lastPointEnd = point->getEndPos();
    mPath.moveTo(point->getPos());
    while(true) {
        point = point->getNextPoint();
        if(point == NULL) break;

        mPath.cubicTo(lastPointEnd,
                      point->getStartPos(),
                      point->getPos());

        lastPointEnd = point->getEndPos();

        if(point == mFirstPoint) break;
    }
}

bool MinimalVectorPath::pointInsidePath(QPointF point) {
    return mPath.contains(point);
}

BezierCubic::BezierCubic(QPointF p1, QPointF c1, QPointF c2, QPointF p2) {
    mP1 = p1;
    mC1 = c1;
    mC2 = c2;
    mP2 = p2;
    generatePath();
}

BezierCubic::BezierCubic(BezierCubic *cubic) {
    mP1 = cubic->getP1();
    mC1 = cubic->getC1();
    mC2 = cubic->getC2();
    mP2 = cubic->getP2();
    generatePath();
}

qreal BezierCubic::getTForPoint(QPointF point) {
    return getTforBezierPoint(mP1, mC1, mC2, mP2, point);
}

void BezierCubic::generatePath() {
    mPainterPath.moveTo(mP1);
    mPainterPath.cubicTo(mC1, mC2, mP2);
}

const QPainterPath &BezierCubic::getAsPainterPath() {
    return mPainterPath;
}

bool BezierCubic::intersects(BezierCubic *bezier) const {
    if(bezier->getPointsBoundingRect().intersects(getPointsBoundingRect())) {
        if(mPainterPath.intersects(bezier->getAsPainterPath())) {
            return true;
        }
    }
    return false;
}

void BezierCubic::intersectWithSub(PointsBezierCubic *otherBezier,
                                   PointsBezierCubic *parentBezier) const {
    if(intersects(otherBezier)) {
        qreal totalLen = mPainterPath.length();
        if(totalLen < 1.) {
            IntersectionPathPoint *newPoint1 =
                    otherBezier->addIntersectionPointAt(mP1);
            IntersectionPathPoint *newPoint2 =
                    parentBezier->addIntersectionPointAt(mP1);
            newPoint1->setSibling(newPoint2);
            newPoint2->setSibling(newPoint1);
            return;
        }

        QPointF sp1 = mP1;
        QPointF sc1 = mC1;
        QPointF sc2;
        QPointF sp2;
        QPointF sc3;
        QPointF sc4 = mC2;
        QPointF sp3 = mP2;
        Edge::getNewRelPosForKnotInsertionAtT(
                    sp1, &sc1, &sc4, sp3,
                    &sp2, &sc2, &sc3,
                    0.5);
        for(int i = 0; i < 2; i ++) {
            BezierCubic subBezier(sp1, sc1, sc2, sp2);
            subBezier.intersectWithSub(otherBezier,
                                       parentBezier);
            sp1 = sp2;
            sc1 = sc3;
            sc2 = sc4;
            sp2 = sp3;
        }
    }
}

QRectF BezierCubic::getPointsBoundingRect() const {
    return QRectF(QPointF(qMin4(mP1.x(), mC1.x(), mC2.x(), mP2.x()),
                          qMin4(mP1.y(), mC1.y(), mC2.y(), mP2.y())),

                  QPointF(qMax4(mP1.x(), mC1.x(), mC2.x(), mP2.x()),
                          qMax4(mP1.y(), mC1.y(), mC2.y(), mP2.y())));
}

qreal BezierCubic::percentAtLength(qreal length) {
    return mPainterPath.percentAtLength(length);
}

const QPointF &BezierCubic::getP1() { return mP1; }

const QPointF &BezierCubic::getC1() { return mC1; }

const QPointF &BezierCubic::getC2() { return mC2; }

const QPointF &BezierCubic::getP2() { return mP2; }

PointsBezierCubic::PointsBezierCubic(MinimalPathPoint *mpp1, MinimalPathPoint *mpp2) :
    BezierCubic(mpp1->getPos(), mpp1->getEndPos(),
                mpp2->getStartPos(), mpp2->getPos()) {
    mMPP1 = mpp1;
    mMPP2 = mpp2;
}

void PointsBezierCubic::intersectWith(PointsBezierCubic *bezier) {

    intersectWithSub(bezier, this);
}

IntersectionPathPoint *PointsBezierCubic::addIntersectionPointAt(QPointF pos) {
    qreal tVal = getTForPoint(pos);
    QPointF sp1 = mP1;
    QPointF sc1 = mC1;
    QPointF sc2;
    QPointF sp2;
    QPointF sc3;
    QPointF sc4 = mC2;
    QPointF sp3 = mP2;
    Edge::getNewRelPosForKnotInsertionAtT(
                sp1, &sc1, &sc4, sp3,
                &sp2, &sc2, &sc3,
                tVal);
    IntersectionPathPoint *newPoint =
            new IntersectionPathPoint(sc2, sp2, sc3);
    mMPP1->setEndCtrlPos(sc1);
    mMPP1->setNextPoint(newPoint);
    mMPP2->setStartCtrlPos(sc4);
    mMPP2->setPrevPoint(newPoint);
    newPoint->setNextPoint(mMPP2);
    newPoint->setPrevPoint(mMPP1);
    return newPoint;
}
