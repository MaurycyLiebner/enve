#include "pathoperations.h"

PathOperations::PathOperations()
{

}
/*

qreal qMin4(qreal v1, qreal v2, qreal v3, qreal v4) {
    return qMin(v1, qMin(v2, qMin(v3, v4) ) );
}

qreal qMax4(qreal v1, qreal v2, qreal v3, qreal v4) {
    return qMax(v1, qMax(v2, qMax(v3, v4) ) );
}

struct MinimalPathPoint {
    MinimalPathPoint() {}

    MinimalPathPoint(QPointF ctrlStart, QPointF pos, QPointF ctrlEnd) {
        mPos = pos;
        mStartCtrlPos = ctrlStart;
        mEndCtrlPos = ctrlEnd;
    }

    MinimalPathPoint(MinimalPathPoint *point) {
        mPos = point->getPos();
        mStartCtrlPos = point->getStartPos();
        mEndCtrlPos = point->getEndPos();
    }

    void setNextPoint(MinimalPathPoint *point) {
        mNextPoint = point;
    }

    void setPrevPoint(MinimalPathPoint *point) {
        mPrevPoint = point;
    }

    MinimalPathPoint *getNextPoint() {
        return mNextPoint;
    }

    MinimalPathPoint *getPrevPoint() {
        return mPrevPoint;
    }

    void setStartCtrlPos(QPointF pos) {
        mStartCtrlPos = pos;
    }

    void setEndCtrlPos(QPointF pos) {
        mEndCtrlPos = pos;
    }

    void setPos(QPointF pos) {
        mPos = pos;
    }

    QPointF getStartPos() {
        return mStartCtrlPos;
    }

    QPointF getEndPos() {
        return mEndCtrlPos;
    }

    QPointF getPos() {
        return mPos;
    }
private:
    MinimalPathPoint *mNextPoint = NULL;
    MinimalPathPoint *mPrevPoint = NULL;
    QPointF mStartCtrlPos;
    QPointF mPos;
    QPointF mEndCtrlPos;
};

class MinimalVectorPath {
    MinimalVectorPath() {

    }

    void setLastPointStart(QPointF start) {
        mLastPoint->setStartCtrlPos(start);
    }

    void setLastPointEnd(QPointF end) {
        mLastPoint->setEndCtrlPos(end);
    }

    void setLastPointPos(QPointF pos) {
        mLastPoint->setPos(pos);
    }

    void addPoint(MinimalPathPoint *point) {
        mPoints << point;
        if(mLastPoint == NULL) {
            mFirstPoint = point;
        } else {
            mLastPoint->setNextPoint(point);
        }
        mLastPoint = point;
    }

    virtual ~MinimalVectorPath() {
        foreach(MinimalPathPoint *point, mPoints) {
            delete point;
        }
    }

    MinimalPathPoint *getFirstPoint() {
        return mFirstPoint;
    }

private:
    MinimalPathPoint *mFirstPoint = NULL;
    MinimalPathPoint *mLastPoint = NULL;
    QList<MinimalPathPoint*> mPoints;
};

class MinimalVectorPathTMP : public MinimalVectorPath {
    MinimalVectorPathTMP() : MinimalVectorPath() {

    }

    void generateQPainterPath() {
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

    bool pointInsidePath(QPointF point) {
        return mPath.contains(point);
    }
private:
    QPainterPath mPath;
};

struct Intersection {
    bool otherReverse() {
        const QPainterPath &otherPath = otherCubic->getAsPainterPath();

    }

    PointsBezierCubic *otherCubic;
    PointsBezierCubic *checkedCubic;
    QPointF point;
    qreal percentChecked;
};

class BezierCubic {
public:
    BezierCubic(QPointF p1, QPointF c1, QPointF c2, QPointF p2) {
        mP1 = p1;
        mC1 = c1;
        mC2 = c2;
        mP2 = p2;
        generatePath();
    }

    BezierCubic(BezierCubic *cubic) {
        mP1 = cubic->getP1();
        mC1 = cubic->getC1();
        mC2 = cubic->getC2();
        mP2 = cubic->getP2();
        generatePath();
    }

    void generatePath() {
        mPainterPath.moveTo(mP1);
        mPainterPath.cubicTo(mC1, mC2, mP2);
    }

    const QPainterPath &getAsPainterPath() {
        return mPainterPath;
    }

    bool intersects(BezierCubic *bezier) {
        if(bezier->getPointsBoundingRect().intersects(getPointsBoundingRect())) {
            if(mPainterPath.intersects(bezier->getAsPainterPath())) {
                return true;
            }
        }
        return false;
    }

    QRectF getPointsBoundingRect() const {
        return QRectF(QPointF(qMin4(mP1.x(), mC1.x(), mC2.x(), mP2.x()),
                              qMin4(mP1.y(), mC1.y(), mC2.y(), mP2.y())),

                      QPointF(qMax4(mP1.x(), mC1.x(), mC2.x(), mP2.x()),
                              qMax4(mP1.y(), mC1.y(), mC2.y(), mP2.y())));
    }

    void getAllIntersectionsWithSub(BezierCubic *otherBezier,
                                    BezierCubic *parentBezier,
                                    QList<Intersection*> *intersections,
                                    qreal currentLength) const {
        if(intersects(otherBezier)) {
            qreal totalLen = mPainterPath.length();
            if(totalLen < 2.) {
                Intersection *intersection = new Intersection();
                intersection->point = mP1;
                intersection->percentChecked = parentBezier->
                                        percentAtLength(currentLength);
                intersection->otherCubic = otherBezier;
                intersection->checkedCubic = parentBezier;
                intersections->append(intersection);
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
                        mPainterPath.percentAtLength(totalLen*0.5));
            for(int i = 0; i < 2; i ++) {
                BezierCubic subBezier(sp1, sc1, sc2, sp2);
                subBezier.getAllIntersectionsWithSub(otherBezier,
                                                     parentBezier,
                                                     intersections,
                                                     currentLength +
                                                     i*totalLen*0.5);
                sp1 = sp2;
                sc1 = sc3;
                sc2 = sc4;
                sp2 = sp3;
            }
        }
    }

    void getAllIntersectionsWith(QList<Intersection*> *intersections,
                                 BezierCubic *bezier) const {

        getAllIntersectionsWithSub(bezier, this, intersections, 0.);
    }

    QRectF getPointsBoundingRect() const {
        return QRectF(QPointF(qMin4(mP1.x(), mC1.x(), mC2.x(), mP2.x()),
                              qMin4(mP1.y(), mC1.y(), mC2.y(), mP2.y())),

                      QPointF(qMax4(mP1.x(), mC1.x(), mC2.x(), mP2.x()),
                              qMax4(mP1.y(), mC1.y(), mC2.y(), mP2.y())));
    }

    qreal percentAtLength(qreal length) {
        return mPainterPath.percentAtLength(length);
    }

    const QPointF &getP1() { return mP1; }
    const QPointF &getC1() { return mC1; }
    const QPointF &getC2() { return mC2; }
    const QPointF &getP2() { return mP2; }

protected:
    QPointF mP1;
    QPointF mC1;
    QPointF mC2;
    QPointF mP2;
    QPainterPath mPainterPath;
};

class PointsBezierCubic : public BezierCubic {
public:
    PointsBezierCubic(MinimalPathPoint *mpp1,
                      MinimalPathPoint *mpp2) :
        BezierCubic(mpp1->getPos(), mpp1->getEndPos(),
                    mpp2->getStartPos(), mpp2->getPos()) {
        mMPP1 = mpp1;
        mMPP2 = mpp2;
    }

    MinimalPathPoint *getPathPoint1(mMPP1);
    MinimalPathPoint *getPathPoint2(mMPP2);
private:
    MinimalPathPoint *mMPP1;
    MinimalPathPoint *mMPP2;
};*/
