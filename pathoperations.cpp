#include "pathoperations.h"
#include "pointhelpers.h"
#include "edge.h"
#include "Animators/pathanimator.h"
#include "Animators/PathAnimators/vectorpathanimator.h"

MinimalNodePoint::MinimalNodePoint() {}

MinimalNodePoint::MinimalNodePoint(QPointF ctrlStart,
                                   QPointF pos,
                                   QPointF ctrlEnd) {
    mPos = pos;
    mStartCtrlPos = ctrlStart;
    mEndCtrlPos = ctrlEnd;
}

void MinimalNodePoint::setNextPoint(MinimalNodePoint *point) {
    mNextPoint = point;
}

void MinimalNodePoint::setPrevPoint(MinimalNodePoint *point) {
    mPrevPoint = point;
}

MinimalNodePoint *MinimalNodePoint::getNextPoint() {
    return mNextPoint;
}

MinimalNodePoint *MinimalNodePoint::getPrevPoint() {
    return mPrevPoint;
}

void MinimalNodePoint::setStartCtrlPos(const QPointF &pos) {
    mStartCtrlPos = pos;
}

void MinimalNodePoint::setEndCtrlPos(const QPointF &pos) {
    mEndCtrlPos = pos;
}

void MinimalNodePoint::setPos(const QPointF &pos) {
    mPos = pos;
}

QPointF MinimalNodePoint::getStartPos() {
    return mStartCtrlPos;
}

QPointF MinimalNodePoint::getEndPos() {
    return mEndCtrlPos;
}

QPointF MinimalNodePoint::getPos() {
    return mPos;
}

bool MinimalNodePoint::isIntersection() {
    return false;
}

IntersectionNodePoint::IntersectionNodePoint() : MinimalNodePoint() {

}

IntersectionNodePoint::IntersectionNodePoint(QPointF start,
                                             QPointF pos,
                                             QPointF end) :
    MinimalNodePoint(start, pos, end) {

}

bool IntersectionNodePoint::isIntersection() {
    return true;
}

void IntersectionNodePoint::setSibling(IntersectionNodePoint *sibling) {
    mSiblingIntPoint = sibling;
}

IntersectionNodePoint *IntersectionNodePoint::getSibling() {
    return mSiblingIntPoint;
}

void IntersectionNodePoint::fixSiblingSideCtrlPoint() {
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
    Q_FOREACH(MinimalVectorPath *separatePath, mSeparatePaths) {
        separatePath->generateQPainterPath();
        mPath.addPath(separatePath->getPath());
    }
}

void FullVectorPath::generateFromPath(const SkPath &path) {
    MinimalNodePoint *firstPoint = nullptr;
    MinimalNodePoint *lastPoint = nullptr;
    MinimalVectorPath *currentTarget = nullptr;

    SkPath::RawIter iter = SkPath::RawIter(path);;

    SkPoint pts[4];
    int verbId = 0;
    for(;;) {
        switch(iter.next(pts)) {
            case SkPath::kMove_Verb: {
                SkPoint pt = pts[0];

                if(lastPoint != firstPoint) {
                    firstPoint->setPrevPoint(lastPoint);
                    firstPoint->setStartCtrlPos(firstPoint->getPos());
                    lastPoint->setNextPoint(firstPoint);
                    lastPoint->setEndCtrlPos(lastPoint->getPos());
                }
                currentTarget = new MinimalVectorPath(this);
                mSeparatePaths.append(currentTarget);
                lastPoint = new MinimalNodePoint(QPointF(pt.x(), pt.y()),
                                                 QPointF(pt.x(), pt.y()),
                                                 QPointF(pt.x(), pt.y()));
                currentTarget->addPoint(lastPoint);
                firstPoint = lastPoint;
            }
                break;
            case SkPath::kLine_Verb: {
                SkPoint pt = pts[1];

                bool sameAsFirstPoint = pointToLen(SkPointToQPointF(pt) -
                                                   firstPoint->getPos()) < 0.1;
                bool connectOnly = false;
                if(sameAsFirstPoint) {
                    if(path.countVerbs() > verbId + 1) {
                        SkPath::Verb nextPathVerb = iter.peek();

                        connectOnly = nextPathVerb == SkPath::kMove_Verb ||
                                nextPathVerb == SkPath::kDone_Verb ||
                                nextPathVerb == SkPath::kClose_Verb;
                    } else {
                        connectOnly = true;
                    }
                }
                if(connectOnly) {
                    firstPoint->setPrevPoint(lastPoint);
                    lastPoint->setNextPoint(firstPoint);
                    lastPoint = firstPoint;
                } else {
                    QPointF qPt = SkPointToQPointF(pt);
                    lastPoint = new MinimalNodePoint(qPt,
                                                     qPt,
                                                     qPt);
                    currentTarget->addPoint(lastPoint);
                }
            }
                break;
            case SkPath::kCubic_Verb: {
                SkPoint endPt = pts[1];
                SkPoint startPt = pts[2];
                SkPoint targetPt = pts[3];

                lastPoint->setEndCtrlPos(SkPointToQPointF(endPt));

                bool sameAsFirstPoint = pointToLen(SkPointToQPointF(targetPt) -
                                                   firstPoint->getPos()) < 0.1;
                bool connectOnly = false;
                if(sameAsFirstPoint) {
                    if(path.countVerbs() > verbId + 1) {
                        SkPath::Verb nextPathVerb = iter.peek();

                        connectOnly = nextPathVerb == SkPath::kMove_Verb ||
                                nextPathVerb == SkPath::kDone_Verb ||
                                nextPathVerb == SkPath::kClose_Verb;
                    } else {
                        connectOnly = true;
                    }
                }
                if(connectOnly) {
                    firstPoint->setPrevPoint(lastPoint);
                    lastPoint->setNextPoint(firstPoint);
                    lastPoint = firstPoint;
                } else {
                    lastPoint = new MinimalNodePoint(QPointF(),
                                                     QPointF(targetPt.x(),
                                                             targetPt.y()),
                                                     QPointF());
                    currentTarget->addPoint(lastPoint);
                }
                lastPoint->setStartCtrlPos(SkPointToQPointF(startPt));
            }
                break;
            case SkPath::kClose_Verb:

                break;
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
            case SkPath::kDone_Verb:
                goto DONE;
                break;
        }
        verbId++;
    }
DONE:
    if(lastPoint != firstPoint) {
        firstPoint->setPrevPoint(lastPoint);
        firstPoint->setStartCtrlPos(firstPoint->getPos());
        lastPoint->setNextPoint(firstPoint);
        lastPoint->setEndCtrlPos(lastPoint->getPos());
    }

    generateSinglePathPaths();
}

//void FullVectorPath::generateFromPath(const QPainterPath &path) {
//    MinimalNodePoint *firstPoint = nullptr;
//    MinimalNodePoint *lastPoint = nullptr;
//    bool firstOther = true;
//    QPointF startCtrlPoint;
//    MinimalVectorPath *currentTarget = nullptr;

//    for(int i = 0; i < path.elementCount(); i++) {
//        const QPainterPath::Element &elem = path.elementAt(i);

//        if (elem.isMoveTo()) { // move
//            if(lastPoint != firstPoint) {
//                firstPoint->setPrevPoint(lastPoint);
//                firstPoint->setStartCtrlPos(firstPoint->getPos());
//                lastPoint->setNextPoint(firstPoint);
//                lastPoint->setEndCtrlPos(lastPoint->getPos());
//            }
//            currentTarget = new MinimalVectorPath(this);
//            mSeparatePaths.append(currentTarget);
//            lastPoint = new MinimalNodePoint(QPointF(),
//                                             QPointF(elem.x, elem.y),
//                                             QPointF());
//            currentTarget->addPoint(lastPoint);
//            firstPoint = lastPoint;
//        } else if (elem.isLineTo()) { // line
//            if((QPointF(elem.x, elem.y) == firstPoint->getPos()) ?
//                    ((path.elementCount() > i + 1) ?
//                                path.elementAt(i + 1).isMoveTo() :
//                                true) :
//                    false) {
//                firstPoint->setPrevPoint(lastPoint);
//                lastPoint->setNextPoint(firstPoint);
//                lastPoint = firstPoint;
//            } else {
//                lastPoint = new MinimalNodePoint(QPointF(),
//                                                 QPointF(elem.x, elem.y),
//                                                 QPointF());
//                currentTarget->addPoint(lastPoint);
//            }
//        } else if (elem.isCurveTo()) { // curve
//            lastPoint->setEndCtrlPos(QPointF(elem.x, elem.y));
//            firstOther = true;
//        } else { // other
//            if(firstOther) {
//                startCtrlPoint = QPointF(elem.x, elem.y);
//            } else {
//                if((QPointF(elem.x, elem.y) == firstPoint->getPos()) ?
//                        ((path.elementCount() > i + 1) ?
//                                    path.elementAt(i + 1).isMoveTo() :
//                                    true) :
//                        false) {
//                    firstPoint->setPrevPoint(lastPoint);
//                    lastPoint->setNextPoint(firstPoint);
//                    lastPoint = firstPoint;
//                } else {
//                    lastPoint = new MinimalNodePoint(QPointF(),
//                                                     QPointF(elem.x, elem.y),
//                                                     QPointF());
//                    currentTarget->addPoint(lastPoint);
//                }
//                //lastPoint->setStartCtrlPtEnabled(true);
//                lastPoint->setStartCtrlPos(startCtrlPoint);
//            }
//            firstOther = !firstOther;
//        }
//    }
//    if(lastPoint != firstPoint) {
//        firstPoint->setPrevPoint(lastPoint);
//        firstPoint->setStartCtrlPos(firstPoint->getPos());
//        lastPoint->setNextPoint(firstPoint);
//        lastPoint->setEndCtrlPos(lastPoint->getPos());
//    }

//    generateSinglePathPaths();
//}

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
        Q_FOREACH(MinimalVectorPath *thisSPath, mSeparatePaths) {
            thisSPath->intersectWith(otherSPath,
                                     unionInterThis,
                                     unionInterOther);
        }
    }
}

void FullVectorPath::getListOfGeneratedSeparatePaths(
                        QList<MinimalVectorPath *> *separate,
                        FullVectorPath *target) {
    Q_FOREACH(MinimalVectorPath *thisSPath, mSeparatePaths) {
        thisSPath->addAllPaths(separate, target);
    }
}

void FullVectorPath::addAllToVectorPath(PathAnimator *path) {
    Q_FOREACH(MinimalVectorPath *separatePath, mSeparatePaths) {
        MinimalNodePoint *firstPoint = separatePath->getFirstPoint();
        MinimalNodePoint *point = firstPoint;
        NodePoint *firstNodePoint = nullptr;
        NodePoint *lastNodePoint = nullptr;
        VectorPathAnimator *singlePath =
                new VectorPathAnimator(path);
        do {
            lastNodePoint = singlePath->addNodeRelPos(point->getPos(),
                                                 point->getStartPos(),
                                                 point->getEndPos(),
                                                 lastNodePoint);
            if(firstNodePoint == nullptr) {
                firstNodePoint = lastNodePoint;
            }
            point = point->getNextPoint();
        } while(point != firstPoint);
        lastNodePoint->connectToPoint(firstNodePoint);
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

void MinimalVectorPath::addPoint(MinimalNodePoint *point) {
    if(mLastPoint == nullptr) {
        mFirstPoint = point;
    } else {
        mLastPoint->setNextPoint(point);
        point->setPrevPoint(mLastPoint);
    }
    mLastPoint = point;
}

MinimalNodePoint *MinimalVectorPath::getFirstPoint() {
    return mFirstPoint;
}

void MinimalVectorPath::intersectWith(MinimalVectorPath *otherPath,
                                      const bool &unionInterThis,
                                      const bool &unionInterOther) {
    if(mPath.intersects(otherPath->getPath())) {
        MinimalNodePoint *firstPoint = otherPath->getFirstPoint();
        MinimalNodePoint *point = firstPoint;
        if(point == nullptr) return;
        MinimalNodePoint *nextPoint = point->getNextPoint();
        if(nextPoint == nullptr) return;

        PointsBezierCubic *lastCubic = nullptr;
        PointsBezierCubic *firstOtherCubic = nullptr;
        do {
            PointsBezierCubic *newCubic = new PointsBezierCubic(point,
                                                                nextPoint,
                                                                otherPath);
            if(firstOtherCubic == nullptr) {
                firstOtherCubic = newCubic;
            } else {
                lastCubic->setNextCubic(newCubic);
                newCubic->setPrevCubic(lastCubic);
            }

            point = nextPoint;
            nextPoint = point->getNextPoint();
            lastCubic = newCubic;
        } while(point != firstPoint);
        firstOtherCubic->setPrevCubic(lastCubic);
        lastCubic->setNextCubic(firstOtherCubic);

        firstPoint = getFirstPoint();
        point = firstPoint;
        if(point == nullptr) return;
        nextPoint = point->getNextPoint();
        if(nextPoint == nullptr) return;
        lastCubic = nullptr;
        PointsBezierCubic *firstThisCubic = nullptr;
        do {
            PointsBezierCubic *newCubic = new PointsBezierCubic(point,
                                                                nextPoint,
                                                                this);
            if(firstThisCubic == nullptr) {
                firstThisCubic = newCubic;
            } else {
                lastCubic->setNextCubic(newCubic);
                newCubic->setPrevCubic(lastCubic);
            }

            point = nextPoint;
            nextPoint = point->getNextPoint();
            lastCubic = newCubic;
            if(nextPoint == nullptr) break; // why is this possible?
        } while(point != firstPoint);
        firstThisCubic->setPrevCubic(lastCubic);
        lastCubic->setNextCubic(firstThisCubic);

        PointsBezierCubic *thisCubic = firstThisCubic;
        PointsBezierCubic *otherCubic;
        do {
            otherCubic = firstOtherCubic;
            do {
                otherCubic = thisCubic->intersectWith(otherCubic);
            } while(otherCubic != firstOtherCubic);
            thisCubic = thisCubic->getNextCubic();
        } while(thisCubic != firstThisCubic);
        const QPainterPath &otherPainterPath = otherPath->getParentFullPath();
        thisCubic = firstThisCubic;
        do {
            if(otherPainterPath.contains(
                        thisCubic->getPointAtT(0.5)) == unionInterThis) {
                thisCubic->disconnect();
            }
            PointsBezierCubic *nextCubic = thisCubic->getNextCubic();
            delete thisCubic;
            thisCubic = nextCubic;
        } while(thisCubic != firstThisCubic);

        otherCubic = firstOtherCubic;
        const QPainterPath &thisPainterPath = getParentFullPath();
        do {
            if(thisPainterPath.contains(
                        otherCubic->getPointAtT(0.5)) == unionInterOther) {
                otherCubic->disconnect();
            }
            PointsBezierCubic *nextCubic = otherCubic->getNextCubic();
            delete otherCubic;
            otherCubic = nextCubic;
        } while(otherCubic != firstOtherCubic);

        Q_FOREACH(IntersectionNodePoint *interPt, mIntersectionPoints) {
            interPt->fixSiblingSideCtrlPoint();
            interPt->getSibling()->fixSiblingSideCtrlPoint();
        }
    }
}

void MinimalVectorPath::addAllPaths(QList<MinimalVectorPath*> *targetsList,
                                    FullVectorPath *targetFull) {
    if(mIntersectionPoints.isEmpty()) {
//        MinimalVectorPath *newPath = new MinimalVectorPath(targetFull);
//        MinimalNodePoint *point = mFirstPoint;
//        do {
//            newPath->addPoint(new MinimalNodePoint(point->getStartPos(),
//                                                   point->getPos(),
//                                                   point->getEndPos()));
//            point = point->getNextPoint();
//        } while(point != mFirstPoint);
//        newPath->closePath();
//        targetsList->append(newPath);
        targetsList->append(this);
        return;
    }
    while(!mIntersectionPoints.isEmpty()) {
        MinimalNodePoint *firstFirstPoint = mIntersectionPoints.takeFirst();
        if(firstFirstPoint->wasAdded()) continue;
        ((IntersectionNodePoint*)firstFirstPoint)->getSibling()->setAdded();

        MinimalVectorPath *target = new MinimalVectorPath(targetFull);
        targetsList->append(target);

        MinimalNodePoint *point = firstFirstPoint;

        bool reversed = !point->isReversed(); // ??
        MinimalNodePoint *nextPoint;
        if(reversed) {
            nextPoint = point->getPrevPoint();
        } else {
            nextPoint = point->getNextPoint();
        }
        while(true) {
            if(point->isIntersection()) {
                if(((IntersectionNodePoint*)point)->getSibling() ==
                        firstFirstPoint ||
                    point == firstFirstPoint) {
                    if(firstFirstPoint->wasAdded()) {
                        break;
                    }
                }

                mIntersectionPoints.removeOne((IntersectionNodePoint*)point);
                MinimalNodePoint *newPoint = nullptr;
                if(reversed) {
                    newPoint = new MinimalNodePoint(point->getEndPos(),
                                                    point->getPos(),
                                                    point->getStartPos());
                } else {
                    newPoint = new MinimalNodePoint(point->getStartPos(),
                                                    point->getPos(),
                                                    point->getEndPos());
                }
                target->addPoint(newPoint);
                point->setAdded();
                point = ((IntersectionNodePoint*)point)->getSibling();
                point->setAdded();

                mIntersectionPoints.removeOne((IntersectionNodePoint*)point);

                reversed = point->isReversed();
                if(reversed) {
                    point = point->getPrevPoint();
                } else {
                    point = point->getNextPoint();
                }
            } else {
                MinimalNodePoint *newPoint = nullptr;
                if(reversed) {
                    newPoint = new MinimalNodePoint(point->getEndPos(),
                                                    point->getPos(),
                                                    point->getStartPos());
                } else {
                    newPoint = new MinimalNodePoint(point->getStartPos(),
                                                    point->getPos(),
                                                    point->getEndPos());
                }
                target->addPoint(newPoint);
                point->setAdded();

                point = nextPoint;
            }
            if(point == nullptr) {
                break;
            }
            if(reversed) {
                nextPoint = point->getPrevPoint();
            } else {
                nextPoint = point->getNextPoint();
            }
        }
        target->closePath();
    }
}

void MinimalVectorPath::generateQPainterPath() {
    MinimalNodePoint *point = mFirstPoint;
    QPointF lastPointEnd = point->getEndPos();
    mPath = QPainterPath();
    mPath.moveTo(point->getPos());
    while(true) {
        point = point->getNextPoint();
        if(point == nullptr) break;

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

BezierCubic::BezierCubic(const QPointF &p1, const QPointF &c1,
                         const QPointF &c2, const QPointF &p2) {
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

QPointF BezierCubic::getPointAtT(const qreal &t) {
    return calcCubicBezierVal(mP1, mC1, mC2, mP2, t);
}

qreal BezierCubic::getTForPoint(QPointF point) {
    return getClosestTValueBezier2D(mP1, mC1, mC2, mP2, point);
}


bool isIntersecting(const QPointF &p1, const QPointF &p2,
                    const QPointF &q1, const QPointF &q2) {
    return (((q1.x() - p1.x())*(p2.y() - p1.y())  -  (q1.y() - p1.y())*(p2.x() - p1.x()))
            * ((q2.x() - p1.x())*(p2.y() - p1.y())  -  (q2.y() - p1.y())*(p2.x() - p1.x())) < 0.)
            &&
           (((p1.x() - q1.x())*(q2.y() - q1.y())  -  (p1.y() - q1.y())*(q2.x() - q1.x()))
            * ((p2.x() - q1.x())*(q2.y() - q1.y())  -  (p2.y() - q1.y())*(q2.x() - q1.x())) < 0.);
}

struct qrealInt {
    qrealInt(const qreal &v1, const int &v2) {
        realV = v1;
        intV = v2;
    }
    qreal realV;
    int intV;
};

bool qrealIntSort(const qrealInt &key1,
                   const qrealInt &key2) {
    return key1.realV < key2.realV;
}

bool PointsBezierCubic::intersects(PointsBezierCubic *bezier,
                             QList<QPointF> *intersectionPts,
                             QList<int> *bezierIntersectionPts) const {
    if(mMPP1->isIntersection()) {
        IntersectionNodePoint *interPt = (IntersectionNodePoint*)mMPP1;
        IntersectionNodePoint *otherInterPt1 =
                (IntersectionNodePoint*)bezier->getMPP1();
        IntersectionNodePoint *otherInterPt2 =
                (IntersectionNodePoint*)bezier->getMPP2();

        if(interPt->getSibling() == otherInterPt1 ||
            interPt->getSibling() == otherInterPt2) return false;
    }
    if(mMPP2->isIntersection()) {
        IntersectionNodePoint *interPt = (IntersectionNodePoint*)mMPP2;
        IntersectionNodePoint *otherInterPt1 =
                (IntersectionNodePoint*)bezier->getMPP1();
        IntersectionNodePoint *otherInterPt2 =
                (IntersectionNodePoint*)bezier->getMPP2();

        if(interPt->getSibling() == otherInterPt1 ||
            interPt->getSibling() == otherInterPt2) return false;
    }

    QPointF bP1 = bezier->getP1();
    QPointF bC1 = bezier->getC1();
    QPointF bC2 = bezier->getC2();
    QPointF bP2 = bezier->getP2();
    QPointF bTL = QPointF(qMin4(bP1.x(), bC1.x(), bC2.x(), bP2.x()),
                          qMin4(bP1.y(), bC1.y(), bC2.y(), bP2.y()));
    QPointF bBR = QPointF(qMax4(bP1.x(), bC1.x(), bC2.x(), bP2.x()),
                          qMax4(bP1.y(), bC1.y(), bC2.y(), bP2.y()));
    QPointF mTL = QPointF(qMin4(mP1.x(), mC1.x(), mC2.x(), mP2.x()),
                          qMin4(mP1.y(), mC1.y(), mC2.y(), mP2.y()));
    QPointF mBR = QPointF(qMax4(mP1.x(), mC1.x(), mC2.x(), mP2.x()),
                          qMax4(mP1.y(), mC1.y(), mC2.y(), mP2.y()));
    if(mBR.x() < bTL.x() || bBR.x() < mTL.x() ||
       mBR.y() < bTL.y() || bBR.y() < mTL.y()) {
        return false;
    }

    qreal thisAproxLen = 0.;
    qreal bezierAproxLen = 0.;

    QPointF lastPos = mP1;
    for(int i = 0; i <= 100; i++) {
        QPointF newPos = calcCubicBezierVal(mP1, mC1, mC2, mP2, i/100.);
        thisAproxLen += pointToLen(lastPos - newPos);
        lastPos = newPos;
    }
    lastPos = bP1;
    for(int i = 0; i <= 100; i++) {
        QPointF newPos = calcCubicBezierVal(bP1, bC1, bC2, bP2, i/100.);
        bezierAproxLen += pointToLen(lastPos - newPos);
        lastPos = newPos;
    }
    qreal thisT = 0.;
    qreal thisTStep = qMin(1., 5./thisAproxLen);
    qreal maxThisT = 1.;

    qreal bezierTStep = qMin(1., 5./bezierAproxLen);
    qreal maxBezierT = 1.;

    QPointF lastBezierPos;
    QPointF currentBezierPos;

    QPointF lastThisPos = calcCubicBezierVal(mP1, mC1,
                                             mC2, mP2, 0.);
    QPointF currentThisPos;
    qreal lastMinDistBetween = -1.;
    qreal lastDistBetween = -1.;
    QList<qrealInt> otherBezierInterTs;
    while(thisT < maxThisT) {
        thisT = thisT + thisTStep;
        currentThisPos = calcCubicBezierVal(mP1, mC1,
                                            mC2, mP2, qMin(1., thisT));
        if(pointToLen(currentThisPos - lastThisPos) < lastMinDistBetween*0.25) {
            continue;
        }

        currentBezierPos = calcCubicBezierVal(bP1, bC1,
                                              bC2, bP2,
                                              0.);
        lastBezierPos = currentBezierPos;
        lastDistBetween = pointToLen(currentThisPos - currentBezierPos);

        qreal bezierT = 0.;
        lastMinDistBetween = 1000000.;
        while(bezierT < maxBezierT) {
            bezierT = bezierT + bezierTStep;
            currentBezierPos = calcCubicBezierVal(bP1, bC1,
                                                  bC2, bP2, qMin(1., bezierT));
            lastDistBetween = pointToLen(currentBezierPos -
                                         currentThisPos);
            if(lastDistBetween < lastMinDistBetween) {
                lastMinDistBetween = lastDistBetween;
            }

//            if(pointToLen(currentBezierPos - lastBezierPos) < lastDistBetween*0.5) {
//                lastBezierPos = currentBezierPos;
//                continue;
//            }

            if(isIntersecting(lastBezierPos, currentBezierPos,
                              lastThisPos, currentThisPos)) {
                qreal dx1 = currentThisPos.x() - lastThisPos.x();
                qreal dy1 = currentThisPos.y() - lastThisPos.y();
                qreal m1 = dy1/dx1;
                qreal c1 = lastThisPos.y() - lastThisPos.x()*m1;

                qreal dx2 = currentBezierPos.x() - lastBezierPos.x();
                qreal dy2 = currentBezierPos.y() - lastBezierPos.y();
                qreal m2 = dy2/dx2;
                qreal c2 = lastBezierPos.y() - lastBezierPos.x()*m2;

                qreal xT = (c2 - c1)/(m1 - m2);
                QPointF intT = QPointF(xT, xT*m1 + c1);
//                if(pointToLen(intT - mP1) < 1. ||
//                    pointToLen(intT - mP2) < 1. ||
//                    pointToLen(intT - bP1) < 1. ||
//                    pointToLen(intT - bP2) < 1.) {
//                } else {
                    otherBezierInterTs <<
                                qrealInt(bezierT, intersectionPts->count());
                    intersectionPts->append(intT);
//                }
            }

            lastBezierPos = currentBezierPos;
        }

        lastThisPos = currentThisPos;
    }
    qSort(otherBezierInterTs.begin(), otherBezierInterTs.end(), qrealIntSort);
    foreach(const qrealInt &val, otherBezierInterTs) {
        bezierIntersectionPts->append(val.intV);
    }
    return !intersectionPts->isEmpty();
}

QRectF BezierCubic::getPointsBoundingRect() const {
    return QRectF(QPointF(qMin4(mP1.x(), mC1.x(), mC2.x(), mP2.x()),
                          qMin4(mP1.y(), mC1.y(), mC2.y(), mP2.y())),
                  QPointF(qMax4(mP1.x(), mC1.x(), mC2.x(), mP2.x()),
                          qMax4(mP1.y(), mC1.y(), mC2.y(), mP2.y())));
}

PointsBezierCubic::PointsBezierCubic(MinimalNodePoint *mpp1,
                                     MinimalNodePoint *mpp2,
                                     MinimalVectorPath *parentPath) :
    BezierCubic(mpp1->getPos(), mpp1->getEndPos(),
                mpp2->getStartPos(), mpp2->getPos()) {
    mParentPath = parentPath;
    mMPP1 = mpp1;
    mMPP2 = mpp2;
}

void PointsBezierCubic::setPoints(MinimalNodePoint *mpp1,
                                  MinimalNodePoint *mpp2) {
    mP1 = mpp1->getPos();
    mC1 = mpp1->getEndPos();
    mC2 = mpp2->getStartPos();
    mP2 = mpp2->getPos();
    mMPP1 = mpp1;
    mMPP2 = mpp2;
}

PointsBezierCubic *PointsBezierCubic::intersectWith(PointsBezierCubic *otherBezier) {
    QList<QPointF> thisInterQPts;
    QList<int> otherInterQPts;
    if(intersects(otherBezier, &thisInterQPts, &otherInterQPts)) {
        PointsBezierCubic *thisBezierT = this;
        PointsBezierCubic *otherBezierT = otherBezier;
        QList<IntersectionNodePoint*> thisInterPts;
        QList<IntersectionNodePoint*> otherInterPts;

        foreach(const QPointF &interPt, thisInterQPts) {
            thisInterPts << thisBezierT->
                            divideCubicAtPointAndReturnIntersection(
                                interPt);
            thisBezierT = thisBezierT->getNextCubic();
        }
        foreach(const int &interPt, otherInterQPts) {
            otherInterPts << otherBezierT->
                             divideCubicAtPointAndReturnIntersection(
                                 thisInterQPts.at(interPt));
            otherBezierT = otherBezier->getNextCubic();
        }
        for(int i = 0; i < thisInterQPts.count(); i++) {
            IntersectionNodePoint *thisPt = thisInterPts.at(i);
            IntersectionNodePoint *otherPt = otherInterPts.at(otherInterQPts.at(i));
            thisPt->setSibling(otherPt);
            otherPt->setSibling(thisPt);
        }
        return otherBezierT;
    }
    return otherBezier->getNextCubic();
}

IntersectionNodePoint *PointsBezierCubic::addIntersectionPointAt(
                                                     QPointF pos) {
    qreal tVal = getTForPoint(pos);
    QPointF sp1 = mP1;
    QPointF sc1 = mC1;
    QPointF sc2;
    QPointF sp2;
    QPointF sc3;
    QPointF sc4 = mC2;
    QPointF sp3 = mP2;
    VectorPathEdge::getNewRelPosForKnotInsertionAtT(
                sp1, &sc1, &sc4, sp3,
                &sp2, &sc2, &sc3,
                tVal);
    IntersectionNodePoint *newPoint =
            new IntersectionNodePoint(sc2, pos, sc3);
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

IntersectionNodePoint *PointsBezierCubic::
divideCubicAtPointAndReturnIntersection(const QPointF &pos) {
    IntersectionNodePoint *interPt = addIntersectionPointAt(pos);
    PointsBezierCubic *newCubic = new PointsBezierCubic(interPt,
                                                        mMPP2,
                                                        mParentPath);
    setPoints(mMPP1, interPt);

    newCubic->setNextCubic(mNextCubic);
    if(mNextCubic != nullptr) {
        mNextCubic->setPrevCubic(newCubic);
    }

    newCubic->setPrevCubic(this);
    setNextCubic(newCubic);

    return interPt;
}

void PointsBezierCubic::disconnect() {
    mMPP1->setReversed(true);
    mMPP1->setNextPoint(nullptr);
    mMPP2->setReversed(false);
    mMPP2->setPrevPoint(nullptr);
}
