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

bool MinimalPathPoint::hasNoConnections() {
    return mNextPoint == NULL && mPrevPoint == NULL;
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

IntersectionPathPoint::~IntersectionPathPoint()
{

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

void IntersectionPathPoint::fixSiblingSideCtrlPoint() {
    MinimalPathPoint *siblingNext = mSiblingIntPoint->getNextPoint();

    bool siblingReversed = siblingNext == NULL;
    bool thisReversed = getNextPoint() == NULL;
    QPointF siblingCtrlPt;
    if(siblingReversed) {
        siblingCtrlPt = mSiblingIntPoint->getStartPos();
    } else {
        siblingCtrlPt = mSiblingIntPoint->getEndPos();
    }
    if(thisReversed) {
        setEndCtrlPos(siblingCtrlPt);
    } else {
        setStartCtrlPos(siblingCtrlPt);
    }
}

MinimalVectorPath::MinimalVectorPath() {

}

void MinimalVectorPath::closePath() {
    mFirstPoint->setPrevPoint(mLastPoint);
    mLastPoint->setNextPoint(mFirstPoint);
}

FullVectorPath::FullVectorPath() {}

void FullVectorPath::generateSignlePathPaths() {
    foreach(MinimalVectorPath *separatePath, mSeparatePaths) {
        separatePath->generateQPainterPath();
    }
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

int FullVectorPath::getSeparatePathsCount() {
    return mSeparatePaths.count();
}

MinimalVectorPath *FullVectorPath::getSeparatePathAt(int id) {
    return mSeparatePaths.at(id);
}

void FullVectorPath::intersectWith(FullVectorPath *otherPath) {
    int otherCount = otherPath->getSeparatePathsCount();
    for(int i = 0; i < otherCount; i++) {
        MinimalVectorPath *otherSPath = otherPath->getSeparatePathAt(i);
        foreach(MinimalVectorPath *thisSPath, mSeparatePaths) {
            thisSPath->intersectWith(otherSPath);
        }
    }
}

void FullVectorPath::getListOfGeneratedSeparatePaths(QList<MinimalVectorPath *> *separate) {
    foreach(MinimalVectorPath *thisSPath, mSeparatePaths) {
        thisSPath->addAllPaths(separate);
    }
}

void FullVectorPath::addAllToVectorPath(VectorPath *path) {
    foreach(MinimalVectorPath *separatePath, mSeparatePaths) {
        MinimalPathPoint *firstPoint = separatePath->getFirstPoint();
        MinimalPathPoint *point = firstPoint;
        PathPoint *firstPathPoint = NULL;
        PathPoint *lastPathPoint = NULL;
        do {
            lastPathPoint = path->addPointRelPos(point->getPos(),
                                                 point->getStartPos(),
                                                 point->getEndPos(),
                                                 lastPathPoint);
            if(firstPathPoint == NULL) {
                firstPathPoint = lastPathPoint;
            }
            point = point->getNextPoint();
        } while(point != firstPoint);
        lastPathPoint->connectToPoint(firstPathPoint);
    }
}

void FullVectorPath::getSeparatePathsFromOther(FullVectorPath *other) {
    other->getListOfGeneratedSeparatePaths(&mSeparatePaths);
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
    if(mLastPoint == NULL) {
        mFirstPoint = point;
    } else {
        mLastPoint->setNextPoint(point);
        point->setPrevPoint(mLastPoint);
    }
    mLastPoint = point;
}

MinimalVectorPath::~MinimalVectorPath() {
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

        PointsBezierCubic *lastCubic = NULL;
        PointsBezierCubic *firstOtherCubic = NULL;
        do {
            PointsBezierCubic *newCubic = new PointsBezierCubic(point,
                                                                nextPoint,
                                                                otherPath);
            if(firstOtherCubic == NULL) {
                firstOtherCubic = newCubic;
            } else {
                lastCubic->setNextCubic(newCubic);
                newCubic->setPrevCubic(lastCubic);
            }

            point = nextPoint;
            nextPoint = point->getNextPoint();
            lastCubic = newCubic;
        } while(point != firstPoint);
        //lastCubic->setNextCubic(firstCubic);
        //firstOtherCubic->setPrevCubic(lastCubic);

        firstPoint = getFirstPoint();
        point = firstPoint;
        if(point == NULL) return;
        nextPoint = point->getNextPoint();
        if(nextPoint == NULL) return;
        lastCubic = NULL;
        PointsBezierCubic *firstThisCubic = NULL;
        do {
            PointsBezierCubic *newCubic = new PointsBezierCubic(point,
                                                                nextPoint,
                                                                this);
            if(firstThisCubic == NULL) {
                firstThisCubic = newCubic;
            } else {
                lastCubic->setNextCubic(newCubic);
                newCubic->setPrevCubic(lastCubic);
            }

            point = nextPoint;
            nextPoint = point->getNextPoint();
            lastCubic = newCubic;
        } while(point != firstPoint);

        PointsBezierCubic *thisCubic = firstThisCubic;
        PointsBezierCubic *otherCubic;
        while(thisCubic != NULL) {
            otherCubic = firstOtherCubic;
            while(otherCubic != NULL) {
                thisCubic->intersectWith(otherCubic);
                otherCubic = otherCubic->getNextCubic();
            }
            thisCubic = thisCubic->getNextCubic();
        }
        const QPainterPath &otherPainterPath = otherPath->getPath();
        thisCubic = firstThisCubic;
        while(thisCubic != NULL) {
            if(otherPainterPath.contains(
                        thisCubic->getPointAtT(0.5))) {
                thisCubic->disconnect();
            }
            PointsBezierCubic *nextCubic = thisCubic->getNextCubic();
            delete thisCubic;
            thisCubic = nextCubic;
        }
        otherCubic = firstOtherCubic;
        while(otherCubic != NULL) {
            if(mPath.contains(
                        otherCubic->getPointAtT(0.5))) {
                otherCubic->disconnect();
            }
            PointsBezierCubic *nextCubic = otherCubic->getNextCubic();
            delete otherCubic;
            otherCubic = nextCubic;
        }

        foreach(IntersectionPathPoint *interPt, mIntersectionPoints) {
            interPt->fixSiblingSideCtrlPoint();
            interPt->getSibling()->fixSiblingSideCtrlPoint();
        }
    }
}

void MinimalVectorPath::addAllPaths(QList<MinimalVectorPath*> *targetsList) {
    if(mIntersectionPoints.isEmpty()) {
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
        ((IntersectionPathPoint*)firstFirstPoint)->getSibling()->setAdded();

        MinimalVectorPath *target = new MinimalVectorPath();
        targetsList->append(target);

        // set first point control point
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
            firstFirstPoint->setEndCtrlPos(ctrlPt);
        } else {
            firstFirstPoint->setStartCtrlPos(ctrlPt);
        }
        //

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
                MinimalPathPoint *newPoint = NULL;
                if(reversed) {
                    newPoint = new MinimalPathPoint(point->getEndPos(),
                                                    point->getPos(),
                                                    point->getStartPos());
                } else {
                    newPoint = new MinimalPathPoint(point);
                }
                target->addPoint(newPoint);
                point->setAdded();

                point = nextPoint;
                if(reversed) {
                    nextPoint = point->getPrevPoint();
                } else {
                    nextPoint = point->getNextPoint();
                }
            }

            if(point->isIntersection()) {
                if(((IntersectionPathPoint*)point)->getSibling() ==
                        firstFirstPoint) {
                    break;
                }

                mIntersectionPoints.removeOne((IntersectionPathPoint*)point);
                MinimalPathPoint *newPoint = NULL;
                if(reversed) {
                    newPoint = new MinimalPathPoint(point->getEndPos(),
                                                    point->getPos(),
                                                    point->getStartPos());
                } else {
                    newPoint = new MinimalPathPoint(point);
                }
                target->addPoint(newPoint);
                point->setAdded();
                point = ((IntersectionPathPoint*)point)->getSibling();
                point->setAdded();

                mIntersectionPoints.removeOne((IntersectionPathPoint*)point);

                reversed = point->getNextPoint() == NULL;
                if(reversed) {
                    point = point->getPrevPoint();
                    nextPoint = point->getPrevPoint();
                } else {
                    point = point->getNextPoint();
                    nextPoint = point->getNextPoint();
                }
            }
        }
        target->closePath();
    }
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
}

BezierCubic::BezierCubic(BezierCubic *cubic) {
    mP1 = cubic->getP1();
    mC1 = cubic->getC1();
    mC2 = cubic->getC2();
    mP2 = cubic->getP2();
}

qreal BezierCubic::getTForPoint(QPointF point) {
    return getTforBezierPoint(mP1, mC1, mC2, mP2, point);
}

bool BezierCubic::intersects(BezierCubic *bezier) const {
    if(bezier->getPointsBoundingRect().intersects(getPointsBoundingRect())) {
        QPointF bP1 = bezier->getP1();
        QPointF bC1 = bezier->getC1();
        QPointF bC2 = bezier->getC2();
        QPointF bP2 = bezier->getP2();
        qreal thisT = 0.;

        QPointF lastBezierPos;
        QPointF currentBezierPos;

        QPointF lastThisPos = mP1;
        qreal thisTStep = 0.0001;

        QPointF currentThisPos;
        qreal bezierTStep;
        qreal lastDistBetween;
        qreal currentDistBetween;
        qreal lowestDistBetween = pointToLen(bP1 - mP1);
        while(thisT < 1.) {
            qreal currThisT = thisT + thisTStep;
            currentThisPos = calcCubicBezierVal(mP1, mC1,
                                                mC2, mP2, currThisT);
            qreal thisInc = pointToLen(lastThisPos - currentThisPos);

            thisTStep *= 0.5*qclamp(lowestDistBetween/thisInc, .1, 10.);
            if(thisInc > 0.5*lowestDistBetween) {
                continue;
            }

            lowestDistBetween = 10000000.;
            bezierTStep = 0.0001;
            currentBezierPos = calcCubicBezierVal(bP1, bC1,
                                                  bC2, bP2,
                                                  bezierTStep);
            lastBezierPos = bP1;
            lastDistBetween = pointToLen(currentThisPos - bP1);
            qreal bezierT = 0.;
            while(bezierT < 1.) {
                qreal currBezierT = bezierT + bezierTStep;
                currentBezierPos = calcCubicBezierVal(bP1, bC1,
                                                      bC2, bP2, currBezierT);

                qreal bezierInc = pointToLen(lastBezierPos -
                                             currentBezierPos);
                bezierTStep *= 0.5*qclamp(lastDistBetween/bezierInc, .1, 10.);
                if(bezierInc > 0.5*lastDistBetween) {
                    continue;
                }
                currentDistBetween = pointToLen(currentBezierPos -
                                                currentThisPos);
                if(currentDistBetween < lowestDistBetween) {
                    if(currentDistBetween < 0.0001) {
                        return true;
                    }
                    lowestDistBetween = currentDistBetween;
                }

                lastBezierPos = currentBezierPos;
                lastDistBetween = currentDistBetween;
                bezierT = currBezierT;
            }

            lastThisPos = currentThisPos;
            thisT = currThisT;
        }
    }
    return false;
}

bool BezierCubic::intersectWithSub(PointsBezierCubic *otherBezier,
                                   PointsBezierCubic *parentBezier) const {
    if(intersects(otherBezier)) {
        QRectF boundingRect = getPointsBoundingRect();
        qreal totalLen = boundingRect.width() + boundingRect.height();
        if(totalLen < 0.1) {
            IntersectionPathPoint *newPoint1 =
                    otherBezier->divideCubicAtPointAndReturnIntersection(mP1);
            IntersectionPathPoint *newPoint2 =
                    parentBezier->divideCubicAtPointAndReturnIntersection(mP1);
            newPoint1->setSibling(newPoint2);
            newPoint2->setSibling(newPoint1);
            return true;
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
        BezierCubic subBezier1(sp1, sc1, sc2, sp2);
        if(subBezier1.intersectWithSub(otherBezier,
                                       parentBezier) ) {
            return true;
        }

        BezierCubic subBezier2(sp2, sc3, sc4, sp3);
        if(subBezier2.intersectWithSub(otherBezier,
                                       parentBezier) ) {
            return true;
        }
    }
    return false;
}

QRectF BezierCubic::getPointsBoundingRect() const {
    return QRectF(QPointF(qMin4(mP1.x(), mC1.x(), mC2.x(), mP2.x()),
                          qMin4(mP1.y(), mC1.y(), mC2.y(), mP2.y())),

                  QPointF(qMax4(mP1.x(), mC1.x(), mC2.x(), mP2.x()),
                          qMax4(mP1.y(), mC1.y(), mC2.y(), mP2.y())));
}

const QPointF &BezierCubic::getP1() { return mP1; }

const QPointF &BezierCubic::getC1() { return mC1; }

const QPointF &BezierCubic::getC2() { return mC2; }

const QPointF &BezierCubic::getP2() { return mP2; }

PointsBezierCubic::PointsBezierCubic(MinimalPathPoint *mpp1,
                                     MinimalPathPoint *mpp2,
                                     MinimalVectorPath *parentPath) :
    BezierCubic(mpp1->getPos(), mpp1->getEndPos(),
                mpp2->getStartPos(), mpp2->getPos()) {
    mParentPath = parentPath;
    mMPP1 = mpp1;
    mMPP2 = mpp2;
}

void PointsBezierCubic::setPoints(MinimalPathPoint *mpp1, MinimalPathPoint *mpp2) {
    mP1 = mpp1->getPos();
    mC1 = mpp1->getEndPos();
    mC2 = mpp2->getStartPos();
    mP2 = mpp2->getPos();
    mMPP1 = mpp1;
    mMPP2 = mpp2;
}

void PointsBezierCubic::intersectWith(PointsBezierCubic *bezier) {

    if(pointToLen(mP1 - bezier->getP1()) < 1. ||
            pointToLen(mP1 - bezier->getP2()) < 1.) return;
    if(pointToLen(mP2 - bezier->getP1()) < 1. ||
            pointToLen(mP2 - bezier->getP2()) < 1.) return;
    intersectWithSub(bezier, this);
}

IntersectionPathPoint *PointsBezierCubic::addIntersectionPointAt(
                                                     QPointF pos) {
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
    mParentPath->addIntersectionPoint(newPoint);
    mMPP1->setEndCtrlPos(sc1);
    mMPP1->setNextPoint(newPoint);
    mMPP2->setStartCtrlPos(sc4);
    mMPP2->setPrevPoint(newPoint);
    newPoint->setNextPoint(mMPP2);
    newPoint->setPrevPoint(mMPP1);
    return newPoint;
}

void PointsBezierCubic::setNextCubic(PointsBezierCubic *cubic) { mNextCubic = cubic; }

void PointsBezierCubic::setPrevCubic(PointsBezierCubic *cubic) { mPrevCubic = cubic; }

PointsBezierCubic *PointsBezierCubic::getNextCubic() { return mNextCubic; }

PointsBezierCubic *PointsBezierCubic::getPrevCubic() { return mPrevCubic; }

IntersectionPathPoint *PointsBezierCubic::divideCubicAtPointAndReturnIntersection(const QPointF &pos) {
    IntersectionPathPoint *interPt = addIntersectionPointAt(pos);
    PointsBezierCubic *newCubic = new PointsBezierCubic(interPt,
                                                        mMPP2,
                                                        mParentPath);
    setPoints(mMPP1, interPt);

    newCubic->setNextCubic(mNextCubic);
    newCubic->setPrevCubic(this);
    setNextCubic(newCubic);
    mNextCubic->setPrevCubic(newCubic);

    return interPt;
}

void PointsBezierCubic::disconnect() {
    mMPP1->setNextPoint(NULL);
    mMPP2->setPrevPoint(NULL);
    if(mMPP1->hasNoConnections()) {
        qDebug() << "delete " << mMPP1;
        delete mMPP1;
        mMPP1 = NULL;
    }
    if(mMPP2->hasNoConnections()) {
        qDebug() << "delete " << mMPP2;
        delete mMPP2;
        mMPP2 = NULL;
    }
}
