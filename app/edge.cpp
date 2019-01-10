#include "edge.h"
#include "MovablePoints/nodepoint.h"
#include "MovablePoints/ctrlpoint.h"
#include "Boxes/boundingbox.h"
#include "global.h"
#include "Animators/PathAnimators/vectorpathanimator.h"
#include "pointhelpers.h"
#include "GUI/mainwindow.h"
#include "Animators/transformanimator.h"

VectorPathEdge::VectorPathEdge(NodePoint *pt1, NodePoint *pt2) {
    setPoint1(pt1);
    setPoint2(pt2);
}

void VectorPathEdge::getNewRelPosForKnotInsertionAtT(const QPointF &P0,
                                                     QPointF *P1_ptr,
                                                     QPointF *P2_ptr,
                                                     const QPointF &P3,
                                                     QPointF *new_p_ptr,
                                                     QPointF *new_p_start_ptr,
                                                     QPointF *new_p_end_ptr,
                                                     const qreal &t) {
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


void VectorPathEdge::getNewRelPosForKnotInsertionAtTSk(const SkPoint &P0,
                                                     SkPoint *P1_ptr,
                                                     SkPoint *P2_ptr,
                                                     SkPoint P3,
                                                     SkPoint *new_p_ptr,
                                                     SkPoint *new_p_start_ptr,
                                                     SkPoint *new_p_end_ptr,
                                                     const SkScalar &t) {
    SkPoint P1 = *P1_ptr;
    SkPoint P2 = *P2_ptr;
    SkPoint P0_1 = P0*(1-t) + P1*t;
    SkPoint P1_2 = P1*(1-t) + P2*t;
    SkPoint P2_3 = P2*(1-t) + P3*t;

    SkPoint P01_12 = P0_1*(1-t) + P1_2*t;
    SkPoint P12_23 = P1_2*(1-t) + P2_3*t;

    SkPoint P0112_1223 = P01_12*(1-t) + P12_23*t;

    *P1_ptr = P0_1;
    *new_p_start_ptr = P01_12;
    *new_p_ptr = P0112_1223;
    *new_p_end_ptr = P12_23;
    *P2_ptr = P2_3;
}

qreal VectorPathEdge::getLength(const QPointF &p0Pos,
                      const QPointF &p1EndPos,
                      const QPointF &p2StartPos,
                      const QPointF &p3Pos) {
    qreal length = 0.;

    qreal tInc = 0.01;
    qreal t = 0.;
    QPointF lastPoint = p0Pos;
    while(true) {
        QPointF currentPoint = getPosBetweenPointsAtT(t + tInc,
                                                      p0Pos,
                                                      p1EndPos,
                                                      p2StartPos,
                                                      p3Pos);
        qreal lenInc = pointToLen(currentPoint - lastPoint);
        if(lenInc > 5) {
            tInc = tInc * 4 / lenInc;
            continue;
        }
        if(lenInc < 3) {
            t += tInc;
            tInc = tInc * 4 / lenInc;
        } else {
            t += tInc;
        }
        length += lenInc;
        lastPoint = currentPoint;
        if(t > 0.999) {
            break;
        }
    }

    return length;
}

qreal VectorPathEdge::getLength(const QPointF &p0Pos,
                        const QPointF &p1EndPos,
                        const QPointF &p2StartPos,
                        const QPointF &p3Pos,
                        int divisions) {
    divisions++; // so that 'i' starts at 1, not 0
    qreal length = 0.;

    QPointF lastPoint = p0Pos;
    for(int i = 1; i < divisions; i++) {
        qreal t = i/static_cast<qreal>(divisions);
        QPointF currentPoint = getPosBetweenPointsAtT(t,
                                                      p0Pos,
                                                      p1EndPos,
                                                      p2StartPos,
                                                      p3Pos);
        length += pointToLen(currentPoint - lastPoint);
        lastPoint = currentPoint;
    }
    length += pointToLen(p3Pos - lastPoint);

    return length;
}

QPointF VectorPathEdge::getPosBetweenPointsAtT(const qreal &t,
                                     const QPointF &p0Pos,
                                     const QPointF &p1EndPos,
                                     const QPointF &p2StartPos,
                                     const QPointF &p3Pos) {
    return gCalcCubicBezierVal({p0Pos, p1EndPos, p2StartPos, p3Pos}, t);
}

QPointF VectorPathEdge::getRelPosBetweenPointsAtT(const qreal &t,
                                        NodePoint *point1,
                                        NodePoint *point2) {
    if(point1 == nullptr) return point2->getRelativePos();
    if(point2 == nullptr) return point1->getRelativePos();

    CtrlPoint *point1EndPt = point1->getEndCtrlPt();
    CtrlPoint *point2StartPt = point2->getStartCtrlPt();
    QPointF p0Pos = point1->getRelativePos();
    QPointF p1Pos = point1EndPt->getRelativePos();
    QPointF p2Pos = point2StartPt->getRelativePos();
    QPointF p3Pos = point2->getRelativePos();

    return getPosBetweenPointsAtT(t, p0Pos, p1Pos, p2Pos, p3Pos);
}

QPointF VectorPathEdge::getAbsPosBetweenPointsAtT(const qreal &t,
                                        NodePoint *point1,
                                        NodePoint *point2) {
    if(point1 == nullptr) return point2->getAbsolutePos();
    if(point2 == nullptr) return point1->getAbsolutePos();

    CtrlPoint *point1EndPt = point1->getEndCtrlPt();
    CtrlPoint *point2StartPt = point2->getStartCtrlPt();
    QPointF p0Pos = point1->getAbsolutePos();
    QPointF p1Pos = point1EndPt->getAbsolutePos();
    QPointF p2Pos = point2StartPt->getAbsolutePos();
    QPointF p3Pos = point2->getAbsolutePos();

    return getPosBetweenPointsAtT(t, p0Pos, p1Pos, p2Pos, p3Pos);
}

QPointF VectorPathEdge::getRelPosAtT(const qreal &t) {
    return getRelPosBetweenPointsAtT(t, mPoint1, mPoint2);
}

QPointF VectorPathEdge::getAbsPosAtT(const qreal &t) {
    return getAbsPosBetweenPointsAtT(t, mPoint1, mPoint2);
}

void VectorPathEdge::makePassThroughAbs(const QPointF &absPos) {
    if(!mPoint2->isStartCtrlPtEnabled() ) {
        mPoint2->setStartCtrlPtEnabled(true);
    }
    if(!mPoint1->isEndCtrlPtEnabled() ) {
        mPoint1->setEndCtrlPtEnabled(true);
    }

    auto absSeg = getAsAbsSegment();

    QPointF dPos = absPos - gCalcCubicBezierVal(absSeg, mPressedT);
    while(pointToLen(dPos) > 1.) {
        absSeg.fP1 += (1. - mPressedT)*dPos;
        absSeg.fP2 += mPressedT*dPos;

        dPos = absPos - gCalcCubicBezierVal(absSeg, mPressedT);
    }

    mPoint1EndPt->moveToAbs(absSeg.fP1);
    mPoint2StartPt->moveToAbs(absSeg.fP2);
}

void VectorPathEdge::makePassThroughRel(const QPointF &relPos) {
    if(!mPoint2->isStartCtrlPtEnabled() ) {
        mPoint2->setStartCtrlPtEnabled(true);
    }
    if(!mPoint1->isEndCtrlPtEnabled() ) {
        mPoint1->setEndCtrlPtEnabled(true);
    }

    auto relSeg = getAsAbsSegment();

    QPointF dPos = relPos - gCalcCubicBezierVal(relSeg, mPressedT);
    while(pointToLen(dPos) > 1.) {
        relSeg.fP1 += (1. - mPressedT)*dPos;
        relSeg.fP2 += mPressedT*dPos;

        dPos = relPos - gCalcCubicBezierVal(relSeg, mPressedT);
    }

    mPoint1EndPt->moveToRel(relSeg.fP1);
    mPoint2StartPt->moveToRel(relSeg.fP2);
}

void VectorPathEdge::finishPassThroughTransform() {
    mPoint1EndPt->finishTransform();
    mPoint2StartPt->finishTransform();
}

void VectorPathEdge::startPassThroughTransform() {
    mPoint1EndPt->startTransform();
    mPoint2StartPt->startTransform();
}

void VectorPathEdge::cancelPassThroughTransform() {
    mPoint1EndPt->cancelTransform();
    mPoint2StartPt->cancelTransform();
}

void VectorPathEdge::generatePainterPath() {
    mSkPath = SkPath();
    mSkPath.moveTo(qPointToSk(mPoint1->getAbsolutePos()));
    mSkPath.cubicTo(qPointToSk(mPoint1->getEndCtrlPtAbsPos()),
                    qPointToSk(mPoint2->getStartCtrlPtAbsPos()),
                    qPointToSk(mPoint2->getAbsolutePos()));
}

void VectorPathEdge::drawHoveredSk(SkCanvas *canvas,
                                   const SkScalar &invScale) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(2.5f*invScale);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(mSkPath, paint);

    paint.setColor(SK_ColorRED);
    paint.setStrokeWidth(1.25f*invScale);
    canvas->drawPath(mSkPath, paint);
}

NodePoint *VectorPathEdge::getPoint1() const {
    return mPoint1;
}

NodePoint *VectorPathEdge::getPoint2() const {
    return mPoint2;
}

void VectorPathEdge::setPoint1(NodePoint *point1) {
    mPoint1 = point1;
    mPoint1EndPt = mPoint1->getEndCtrlPt();
}

void VectorPathEdge::setPoint2(NodePoint *point2) {
    mPoint2 = point2;
    mPoint2StartPt = mPoint2->getStartCtrlPt();
}

void VectorPathEdge::setPressedT(const qreal &t) {
    mPressedT = t;
}

void VectorPathEdge::getNearestAbsPosAndT(const QPointF &absPos,
                                QPointF *nearestPoint,
                                qreal *t) {
    *t = gGetClosestTValueOnBezier(getAsAbsSegment(), absPos,
                                   nearestPoint);
}

void VectorPathEdge::getNearestRelPosAndT(const QPointF &relPos,
                                QPointF *nearestPoint,
                                qreal *t,
                                qreal *error) {
    *t = gGetClosestTValueOnBezier(getAsRelSegment(), relPos,
                                   nearestPoint, error);
}

qCubicSegment2D VectorPathEdge::getAsAbsSegment() const {
    Q_ASSERT(mPoint1 && mPoint2);
    return {mPoint1->getAbsolutePos(),
            mPoint1->getEndCtrlPtAbsPos(),
            mPoint2->getStartCtrlPtAbsPos(),
            mPoint2->getAbsolutePos()};
}

qCubicSegment2D VectorPathEdge::getAsRelSegment() const {
    Q_ASSERT(mPoint1 && mPoint2);
    return {mPoint1->getRelativePos(),
            mPoint1->getEndCtrlPtValue(),
            mPoint2->getStartCtrlPtValue(),
            mPoint2->getRelativePos()};
}

QPointF VectorPathEdge::getSlopeVector(const qreal &t) {
    QPointF posAtT = getRelPosAtT(t);
    QPointF posAtTPlus = getRelPosAtT(t + 0.01);
    return scalePointToNewLen(posAtTPlus - posAtT, 1.);
}
