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
    MinimalPathPoint *point = otherPath->getFirstPoint();
    if(point == NULL) return;
    MinimalPathPoint *nextPoint = point->getNextPoint();
    QList<PointsBezierCubic*> otherCubics;
    while(nextPoint != NULL) {
        otherCubics << new PointsBezierCubic(point, nextPoint);

        nextPoint = point->getNextPoint();
    }

    QList<PointsBezierCubic*> thisCubics;
    point = getFirstPoint();
    if(point == NULL) return;
    nextPoint = point->getNextPoint();
    while(nextPoint != NULL) {
        thisCubics << new PointsBezierCubic(point, nextPoint);

        nextPoint = point->getNextPoint();
    }

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

void MinimalVectorPath::addAllPointsToPath(MinimalVectorPath *target) {
    MinimalPathPoint *firstFirstPoint = mIntersectionPoints.first();
    MinimalPathPoint *point = firstFirstPoint;
    MinimalPathPoint *nextPoint = point;
    while(true) {
        while(nextPoint != NULL) {
            target->addPoint(new MinimalPathPoint(point));

            point = nextPoint;
            nextPoint = point->getNextPoint();
        }

        if(point->isIntersection()) {
            target->addPoint(new MinimalPathPoint(point));
            point = ((IntersectionPathPoint*)point)->getSibling();
            if(point == firstFirstPoint) {
                firstFirstPoint->setStartCtrlPos(point->getStartPos());
                return;
            }
            target->setLastPointEnd(point->getEndPos());
            point = point->getNextPoint();
            nextPoint = point->getNextPoint();
        }

    }
}

void MinimalVectorPath::removeNonIntersectionPointsContainedInAndSaveIntersectionPoints(MinimalVectorPath *path) {
    QList<MinimalPathPoint*> pointsToDelete;
    foreach(MinimalPathPoint *point, mPoints) {
        if(point->isIntersection()) {
            mIntersectionPoints << (IntersectionPathPoint*)point;
            continue;
        }
        if(path->pointInsidePath(point->getPos()) ) {
            pointsToDelete << point;
        }
    }
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
}

void MinimalVectorPath::generateQPainterPath() {
    MinimalPathPoint *point = mFirstPoint;
    QPointF lastPointEnd = point->getEndPos();
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

void BezierCubic::intersectWithSub(PointsBezierCubic *otherBezier, PointsBezierCubic *parentBezier) const {
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
