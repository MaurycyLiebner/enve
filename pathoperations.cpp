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

MinimalNodePoint::MinimalNodePoint(MinimalNodePoint *point) {
    mPos = point->getPos();
    mStartCtrlPos = point->getStartPos();
    mEndCtrlPos = point->getEndPos();
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

IntersectionNodePoint::~IntersectionNodePoint()
{

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
    MinimalNodePoint *firstPoint = NULL;
    MinimalNodePoint *lastPoint = NULL;
    MinimalVectorPath *currentTarget = NULL;

    SkPath::RawIter iter = SkPath::RawIter(path);;

    SkPoint pts[4];
    int verbId = 0;
    for (;;) {
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
                lastPoint = new MinimalNodePoint(QPointF(),
                                                 QPointF(pt.x(), pt.y()),
                                                 QPointF());
                currentTarget->addPoint(lastPoint);
                firstPoint = lastPoint;
            }
                break;
            case SkPath::kLine_Verb: {
                SkPoint pt = pts[1];

                bool sameAsFirstPoint = SkPointToQPointF(pt) ==
                                            firstPoint->getPos();
                bool connectOnly = false;
                if(sameAsFirstPoint) {
                    if(path.countVerbs() > verbId + 1) {
                        connectOnly = iter.peek() == SkPath::kMove_Verb;
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
                                                     SkPointToQPointF(pt),
                                                     QPointF());
                    currentTarget->addPoint(lastPoint);
                }
            }
                break;
            case SkPath::kCubic_Verb: {
                SkPoint endPt = pts[1];
                SkPoint startPt = pts[2];
                SkPoint targetPt = pts[3];

                lastPoint->setEndCtrlPos(SkPointToQPointF(endPt));

                bool sameAsFirstPoint = SkPointToQPointF(targetPt) ==
                                            firstPoint->getPos();
                bool connectOnly = false;
                if(sameAsFirstPoint) {
                    if(path.countVerbs() > verbId + 1) {
                        connectOnly = iter.peek() == SkPath::kMove_Verb;
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
//    MinimalNodePoint *firstPoint = NULL;
//    MinimalNodePoint *lastPoint = NULL;
//    bool firstOther = true;
//    QPointF startCtrlPoint;
//    MinimalVectorPath *currentTarget = NULL;

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
        NodePoint *firstNodePoint = NULL;
        NodePoint *lastNodePoint = NULL;
        VectorPathAnimator *singlePath =
                new VectorPathAnimator(path);
        do {
            lastNodePoint = singlePath->addNodeRelPos(point->getPos(),
                                                 point->getStartPos(),
                                                 point->getEndPos(),
                                                 lastNodePoint);
            if(firstNodePoint == NULL) {
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
    if(mLastPoint == NULL) {
        mFirstPoint = point;
    } else {
        mLastPoint->setNextPoint(point);
        point->setPrevPoint(mLastPoint);
    }
    mLastPoint = point;
}

MinimalVectorPath::~MinimalVectorPath() {
    Q_FOREACH(MinimalNodePoint *point, mIntersectionPoints) {
        delete point;
    }
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
        if(point == NULL) return;
        MinimalNodePoint *nextPoint = point->getNextPoint();
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

        Q_FOREACH(IntersectionNodePoint *interPt, mIntersectionPoints) {
            interPt->fixSiblingSideCtrlPoint();
            interPt->getSibling()->fixSiblingSideCtrlPoint();
        }
    }
}

void MinimalVectorPath::addAllPaths(QList<MinimalVectorPath*> *targetsList,
                                    FullVectorPath *targetFull) {
    if(mIntersectionPoints.isEmpty()) {
        MinimalVectorPath *newPath = new MinimalVectorPath(targetFull);
        MinimalNodePoint *point = mFirstPoint;
        do {
            newPath->addPoint(new MinimalNodePoint(point));
            point = point->getNextPoint();
        } while(point != mFirstPoint);
        newPath->closePath();
        targetsList->append(newPath);
        return;
    }
    while(!mIntersectionPoints.isEmpty()) {
        MinimalNodePoint *firstFirstPoint = mIntersectionPoints.takeFirst();
        if(firstFirstPoint->wasAdded()) continue;
        ((IntersectionNodePoint*)firstFirstPoint)->getSibling()->setAdded();

        MinimalVectorPath *target = new MinimalVectorPath(targetFull);
        targetsList->append(target);

        MinimalNodePoint *point = firstFirstPoint;

        bool reversed = point->isReversed();
        MinimalNodePoint *nextPoint;
        if(reversed) {
            nextPoint = point->getPrevPoint();
        } else {
            nextPoint = point->getNextPoint();
        }
        while(true) {
            while(!point->isIntersection() ||
                  point == firstFirstPoint) {
                MinimalNodePoint *newPoint = NULL;
                if(reversed) {
                    newPoint = new MinimalNodePoint(point->getEndPos(),
                                                    point->getPos(),
                                                    point->getStartPos());
                } else {
                    newPoint = new MinimalNodePoint(point);
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
                if(((IntersectionNodePoint*)point)->getSibling() ==
                        firstFirstPoint) {
                    break;
                }

                mIntersectionPoints.removeOne((IntersectionNodePoint*)point);
                MinimalNodePoint *newPoint = NULL;
                if(reversed) {
                    newPoint = new MinimalNodePoint(point->getEndPos(),
                                                    point->getPos(),
                                                    point->getStartPos());
                } else {
                    newPoint = new MinimalNodePoint(point);
                }
                target->addPoint(newPoint);
                point->setAdded();
                point = ((IntersectionNodePoint*)point)->getSibling();
                point->setAdded();
                if(((IntersectionNodePoint*)point)->getSibling() ==
                        firstFirstPoint) {
                    break;
                }

                mIntersectionPoints.removeOne((IntersectionNodePoint*)point);

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
    MinimalNodePoint *point = mFirstPoint;
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
    return getTforBezierPoint(mP1, mC1, mC2, mP2, point);
}


bool isIntersecting(const QPointF &p1, const QPointF &p2,
                    const QPointF &q1, const QPointF &q2) {
    return (((q1.x() - p1.x())*(p2.y() - p1.y())  -  (q1.y() - p1.y())*(p2.x() - p1.x()))
            * ((q2.x() - p1.x())*(p2.y() - p1.y())  -  (q2.y() - p1.y())*(p2.x() - p1.x())) < 0.)
            &&
           (((p1.x() - q1.x())*(q2.y() - q1.y())  -  (p1.y() - q1.y())*(q2.x() - q1.x()))
            * ((p2.x() - q1.x())*(q2.y() - q1.y())  -  (p2.y() - q1.y())*(q2.x() - q1.x())) < 0.);
}

bool BezierCubic::intersects(BezierCubic *bezier, QPointF *intersectionPt) const {
    if(bezier->getPointsBoundingRect().intersects(getPointsBoundingRect())) {
        QPointF bP1 = bezier->getP1();
        QPointF bC1 = bezier->getC1();
        QPointF bC2 = bezier->getC2();
        QPointF bP2 = bezier->getP2();
        qreal thisT = 0.01;

        QPointF lastBezierPos;
        QPointF currentBezierPos;

        QPointF lastThisPos = calcCubicBezierVal(mP1, mC1,
                                                 mC2, mP2, 0.01);
        qreal thisTStep = 0.01;

        QPointF currentThisPos;
        qreal bezierTStep = 0.01;
        qreal lastMinDistBetween = -1.;
        qreal lastDistBetween = -1.;
        while(thisT < .99) {
            thisT = thisT + thisTStep;
            currentThisPos = calcCubicBezierVal(mP1, mC1,
                                                mC2, mP2, thisT);
            //qreal thisInc = pointToLen(lastThisPos - currentThisPos);
            if(pointToLen(currentThisPos - lastThisPos) < lastMinDistBetween*0.5) {
                lastThisPos = currentThisPos;
                continue;
            }

            currentBezierPos = calcCubicBezierVal(bP1, bC1,
                                                  bC2, bP2,
                                                  0.01);
            lastBezierPos = calcCubicBezierVal(mP1, mC1,
                                               mC2, mP2, 0.01);
            lastDistBetween = pointToLen(currentThisPos - currentBezierPos);

            qreal bezierT = 0.01;
            while(bezierT < .99) {
                bezierT = bezierT + bezierTStep;
                currentBezierPos = calcCubicBezierVal(bP1, bC1,
                                                      bC2, bP2, bezierT);
                lastDistBetween = pointToLen(currentBezierPos -
                                             currentThisPos);
                if(lastDistBetween < lastMinDistBetween) {
                    lastMinDistBetween = lastDistBetween;
                }

                if(pointToLen(currentBezierPos - lastBezierPos) < lastDistBetween*0.5) {
                    lastBezierPos = currentBezierPos;
                    continue;
                }
//                qreal bezierInc = pointToLen(lastBezierPos - currentBezierPos);

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
                    *intersectionPt = QPointF(xT, xT*m1 + c1);
                    return true;
                }

                lastBezierPos = currentBezierPos;
            }

            lastThisPos = currentThisPos;
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

void PointsBezierCubic::intersectWith(PointsBezierCubic *otherBezier) {
    QPointF interPt;
    if(intersects(otherBezier, &interPt)) {
        IntersectionNodePoint *newPoint1 =
                otherBezier->divideCubicAtPointAndReturnIntersection(interPt);
        IntersectionNodePoint *newPoint2 =
                this->divideCubicAtPointAndReturnIntersection(interPt);
        newPoint1->setSibling(newPoint2);
        newPoint2->setSibling(newPoint1);
    }
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
            new IntersectionNodePoint(sc2, sp2, sc3);
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
    newCubic->setPrevCubic(this);
    setNextCubic(newCubic);
    mNextCubic->setPrevCubic(newCubic);

    return interPt;
}

void PointsBezierCubic::disconnect() {
    mMPP1->setReversed(true);
    mMPP2->setReversed(false);
}
