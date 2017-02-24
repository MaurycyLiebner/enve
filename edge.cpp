#include "edge.h"
#include "pathpoint.h"
#include "ctrlpoint.h"
#include "Boxes/boundingbox.h"

Edge::Edge(PathPoint *pt1, PathPoint *pt2, qreal pressedT) {
    mPoint1 = pt1;
    mPoint1EndPt = pt1->getEndCtrlPt();
    mPoint2 = pt2;
    mPoint2StartPt = pt2->getStartCtrlPt();
    mPressedT = pressedT;
}

void Edge::getNewRelPosForKnotInsertionAtT(QPointF P0, QPointF *P1_ptr, QPointF *P2_ptr, QPointF P3, QPointF *new_p_ptr, QPointF *new_p_start_ptr, QPointF *new_p_end_ptr, qreal t) {
    QPointF P1 = *P1_ptr;
    QPointF P2 = *P2_ptr;
    QPointF P0_1 = (1-t)*P0 + t*P1;
    QPointF P1_2 = (1-t)*P1 + t*P2;
    QPointF P2_3 = (1-t)*P2 + t*P3;

    QPointF P01_12 = (1-t)*P0_1 + t*P1_2;
    QPointF P12_23 = (1-t)*P1_2 + t*P2_3;

    QPointF P0112_1223 = (1-t)*P01_12 + t*P12_23;

    *P1_ptr = P0_1;
    *new_p_start_ptr = P01_12;
    *new_p_ptr = P0112_1223;
    *new_p_end_ptr = P12_23;
    *P2_ptr = P2_3;
}

QPointF Edge::getRelPosBetweenPointsAtT(qreal t, QPointF p0Pos, QPointF p1EndPos, QPointF p2StartPos, QPointF p3Pos) {
    qreal x0 = p0Pos.x();
    qreal y0 = p0Pos.y();
    qreal x1 = p1EndPos.x();
    qreal y1 = p1EndPos.y();
    qreal x2 = p2StartPos.x();
    qreal y2 = p2StartPos.y();
    qreal x3 = p3Pos.x();
    qreal y3 = p3Pos.y();

    return QPointF(calcCubicBezierVal(x0, x1, x2, x3, t),
                   calcCubicBezierVal(y0, y1, y2, y3, t) );
}

QPointF Edge::getRelPosBetweenPointsAtT(qreal t, PathPoint *point1, PathPoint *point2) {
    if(point1 == NULL) return point2->getRelativePos();
    if(point2 == NULL) return point1->getRelativePos();

    CtrlPoint *point1EndPt = point1->getEndCtrlPt();
    CtrlPoint *point2StartPt = point2->getStartCtrlPt();
    QPointF p0Pos = point1->getRelativePos();
    QPointF p1Pos = point1EndPt->getRelativePos();
    QPointF p2Pos = point2StartPt->getRelativePos();
    QPointF p3Pos = point2->getRelativePos();

    return getRelPosBetweenPointsAtT(t, p0Pos, p1Pos, p2Pos, p3Pos);
}

void Edge::makePassThrough(QPointF absPos) {
    if(!mPoint2->isStartCtrlPtEnabled() ) {
        mPoint2->setStartCtrlPtEnabled(true);
    }
    if(!mPoint1->isEndCtrlPtEnabled() ) {
        mPoint1->setEndCtrlPtEnabled(true);
    }


    QPointF p0Pos = mPoint1->getAbsolutePos();
    QPointF p1Pos = mPoint1EndPt->getAbsolutePos();
    QPointF p2Pos = mPoint2StartPt->getAbsolutePos();
    QPointF p3Pos = mPoint2->getAbsolutePos();

    if(!mEditPath) {
        BoundingBox *parentBox = mPoint1->getParent();
        PathPointValues p1Values = mPoint1->getPointValues();
        p0Pos = parentBox->getCombinedTransform().map(
                    p1Values.pointRelPos);
        p1Pos = parentBox->getCombinedTransform().map(
                    p1Values.endRelPos);
        PathPointValues p2Values = mPoint2->getPointValues();
        p2Pos = parentBox->getCombinedTransform().map(
                    p2Values.startRelPos);
        p3Pos = parentBox->getCombinedTransform().map(
                    p2Values.pointRelPos);
    }

    qreal x0 = p0Pos.x();
    qreal y0 = p0Pos.y();
    qreal x1 = p1Pos.x();
    qreal y1 = p1Pos.y();
    qreal x2 = p2Pos.x();
    qreal y2 = p2Pos.y();
    qreal x3 = p3Pos.x();
    qreal y3 = p3Pos.y();

    qreal dx = absPos.x() - calcCubicBezierVal(x0, x1, x2, x3, mPressedT);
    qreal dy = absPos.y() - calcCubicBezierVal(y0, y1, y2, y3, mPressedT);
    while(dx*dx + dy*dy > 1.) {
        x1 += (1. - mPressedT)*dx;
        y1 += (1. - mPressedT)*dy;
        x2 += mPressedT*dx;
        y2 += mPressedT*dy;

        dx = absPos.x() - calcCubicBezierVal(x0, x1, x2, x3, mPressedT);
        dy = absPos.y() - calcCubicBezierVal(y0, y1, y2, y3, mPressedT);
    }


    mPoint1EndPt->moveToAbs(QPointF(x1, y1) );
    mPoint2StartPt->moveToAbs(QPointF(x2, y2) );
}

void Edge::finishTransform() {
    mPoint1EndPt->finishTransform();
    mPoint2StartPt->finishTransform();
}

void Edge::startTransform() {
    mPoint1EndPt->startTransform();
    mPoint2StartPt->startTransform();
}

void Edge::setEditPath(bool bT) {
    mEditPath = bT;
}

void Edge::generatePainterPath() {
    mPath = QPainterPath();
    mPath.moveTo(mPoint1->getAbsolutePos());
    mPath.cubicTo(mPoint1->getEndCtrlPtAbsPos(),
                  mPoint2->getStartCtrlPtAbsPos(),
                  mPoint2->getAbsolutePos());
}

void Edge::drawHover(QPainter *p) {
    p->setBrush(Qt::NoBrush);
    QPen pen = QPen(Qt::red, 2.);
    pen.setCosmetic(true);
    p->setPen(pen);
    p->drawPath(mPath);
}
