#include "pathoperations.h"
#include "pointhelpers.h"
#include "edge.h"

MinimalPathPoint::MinimalPathPoint() {}

MinimalPathPoint::MinimalPathPoint(QPointF ctrlStart,
                                   QPointF pos,
                                   QPointF ctrlEnd) {
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

IntersectionPathPoint::IntersectionPathPoint(QPointF start,
                                             QPointF pos,
                                             QPointF end) :
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
    bool siblingReversed = mSiblingIntPoint->isReversed();
    bool thisReversed = isReversed();
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

MinimalVectorPath::MinimalVectorPath(FullVectorPath *parent) {
    mParentFullPath = parent;
}

void MinimalVectorPath::closePath() {
    mFirstPoint->setPrevPoint(mLastPoint);
    mLastPoint->setNextPoint(mFirstPoint);
}

FullVectorPath::FullVectorPath() {}

void FullVectorPath::generateSinglePathPaths() {
    mPath = QPainterPath();
    foreach(MinimalVectorPath *separatePath, mSeparatePaths) {
        separatePath->generateQPainterPath();
        mPath.addPath(separatePath->getPath());
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
            if(lastPoint != firstPoint) {
                firstPoint->setPrevPoint(lastPoint);
                firstPoint->setStartCtrlPos(firstPoint->getPos());
                lastPoint->setNextPoint(firstPoint);
                lastPoint->setEndCtrlPos(lastPoint->getPos());
            }
            currentTarget = new MinimalVectorPath(this);
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
    if(lastPoint != firstPoint) {
        firstPoint->setPrevPoint(lastPoint);
        firstPoint->setStartCtrlPos(firstPoint->getPos());
        lastPoint->setNextPoint(firstPoint);
        lastPoint->setEndCtrlPos(lastPoint->getPos());
    }

    generateSinglePathPaths();
}

int FullVectorPath::getSeparatePathsCount() {
    return mSeparatePaths.count();
}

MinimalVectorPath *FullVectorPath::getSeparatePathAt(int id) {
    return mSeparatePaths.at(id);
}

void FullVectorPath::intersectWith(FullVectorPath *otherPath,
                                   const bool &unionInterThis,
                                   const bool &unionInterOther) {
    int otherCount = otherPath->getSeparatePathsCount();
    for(int i = 0; i < otherCount; i++) {
        MinimalVectorPath *otherSPath = otherPath->getSeparatePathAt(i);
        foreach(MinimalVectorPath *thisSPath, mSeparatePaths) {
            thisSPath->intersectWith(otherSPath,
                                     unionInterThis,
                                     unionInterOther);
        }
    }
}

void FullVectorPath::getListOfGeneratedSeparatePaths(
                        QList<MinimalVectorPath *> *separate,
                        FullVectorPath *target) {
    foreach(MinimalVectorPath *thisSPath, mSeparatePaths) {
        thisSPath->addAllPaths(separate, target);
    }
}

#include "Animators/singlepathanimator.h"
void FullVectorPath::addAllToVectorPath(PathAnimator *path) {
    foreach(MinimalVectorPath *separatePath, mSeparatePaths) {
        MinimalPathPoint *firstPoint = separatePath->getFirstPoint();
        MinimalPathPoint *point = firstPoint;
        PathPoint *firstPathPoint = NULL;
        PathPoint *lastPathPoint = NULL;
        SinglePathAnimator *singlePath = new SinglePathAnimator(path);
        do {
            lastPathPoint = singlePath->addPointRelPos(point->getPos(),
                                                 point->getStartPos(),
                                                 point->getEndPos(),
                                                 lastPathPoint);
            if(firstPathPoint == NULL) {
                firstPathPoint = lastPathPoint;
            }
            point = point->getNextPoint();
        } while(point != firstPoint);
        lastPathPoint->connectToPoint(firstPathPoint);
        path->addSinglePathAnimator(singlePath);
    }
}

void FullVectorPath::getSeparatePathsFromOther(FullVectorPath *other) {
    other->getListOfGeneratedSeparatePaths(&mSeparatePaths, this);
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

void MinimalVectorPath::intersectWith(MinimalVectorPath *otherPath,
                                      const bool &unionInterThis,
                                      const bool &unionInterOther) {
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
        const QPainterPath &otherPainterPath = otherPath->getParentFullPath();
        thisCubic = firstThisCubic;
        while(thisCubic != NULL) {
            if(otherPainterPath.contains(
                        thisCubic->getPointAtT(0.5)) == unionInterThis) {
                thisCubic->disconnect();
            }
            PointsBezierCubic *nextCubic = thisCubic->getNextCubic();
            delete thisCubic;
            thisCubic = nextCubic;
        }
        otherCubic = firstOtherCubic;
        const QPainterPath &thisPainterPath = getParentFullPath();
        while(otherCubic != NULL) {
            if(thisPainterPath.contains(
                        otherCubic->getPointAtT(0.5)) == unionInterOther) {
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

void MinimalVectorPath::addAllPaths(QList<MinimalVectorPath*> *targetsList,
                                    FullVectorPath *targetFull) {
    if(mIntersectionPoints.isEmpty()) {
        MinimalVectorPath *newPath = new MinimalVectorPath(targetFull);
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

        MinimalVectorPath *target = new MinimalVectorPath(targetFull);
        targetsList->append(target);

        MinimalPathPoint *point = firstFirstPoint;

        bool reversed = point->isReversed();
        MinimalPathPoint *nextPoint;
        if(reversed) {
            nextPoint = point->getPrevPoint();
        } else {
            nextPoint = point->getNextPoint();
        }
        while(true) {
            while(!point->isIntersection() ||
                  point == firstFirstPoint) {
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

                reversed = point->isReversed();
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
    mIntersectionPoints.clear();
}

void MinimalVectorPath::generateQPainterPath() {
    MinimalPathPoint *point = mFirstPoint;
    QPointF lastPointEnd = point->getEndPos();
    mPath = QPainterPath();
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

BezierCubic::BezierCubic(QPointF p1, QPointF c1,
                         QPointF c2, QPointF p2) {
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

bool BezierCubic::intersects(BezierCubic *bezier,
                             QPointF *intersectionPt) const {
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
                    if(currentDistBetween < 0.000001) {
                        if(((pointToLen(mP1 - bezier->getP1()) < .001 ||
                           pointToLen(mP1 - bezier->getP2()) < .001) &&
                           pointToLen(mP1 - currentBezierPos) < .001) ||
                           ((pointToLen(mP2 - bezier->getP1()) < .001 ||
                           pointToLen(mP2 - bezier->getP2()) < .001) &&
                           pointToLen(mP2 - currentBezierPos) < .001)) {
                            bezierTStep *= 10;
                            currentDistBetween *= 10;
                        } else {
                            *intersectionPt = currentThisPos;
                            return true;
                        }
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

void PointsBezierCubic::setPoints(MinimalPathPoint *mpp1,
                                  MinimalPathPoint *mpp2) {
    mP1 = mpp1->getPos();
    mC1 = mpp1->getEndPos();
    mC2 = mpp2->getStartPos();
    mP2 = mpp2->getPos();
    mMPP1 = mpp1;
    mMPP2 = mpp2;
}

void PointsBezierCubic::intersectWith(PointsBezierCubic *otherBezier) {
    QPointF interPt;
    if(intersects(otherBezier, &interPt)) {
        IntersectionPathPoint *newPoint1 =
                otherBezier->divideCubicAtPointAndReturnIntersection(interPt);
        IntersectionPathPoint *newPoint2 =
                this->divideCubicAtPointAndReturnIntersection(interPt);
        newPoint1->setSibling(newPoint2);
        newPoint2->setSibling(newPoint1);
    }
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

void PointsBezierCubic::setNextCubic(PointsBezierCubic *cubic) {
    mNextCubic = cubic;
}

void PointsBezierCubic::setPrevCubic(PointsBezierCubic *cubic) {
    mPrevCubic = cubic;
}

PointsBezierCubic *PointsBezierCubic::getNextCubic() {
    return mNextCubic;
}

PointsBezierCubic *PointsBezierCubic::getPrevCubic() {
    return mPrevCubic;
}

IntersectionPathPoint *PointsBezierCubic::
divideCubicAtPointAndReturnIntersection(const QPointF &pos) {
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
    mMPP1->setReversed(true);
    mMPP2->setReversed(false);
}
