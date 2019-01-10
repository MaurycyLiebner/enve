#include "lengthpatheffect.h"
#include "edge.h"
#include "pointhelpers.h"

LengthPathEffect::LengthPathEffect(const bool &outlinePathEffect) :
    PathEffect("length effect", LENGTH_PATH_EFFECT, outlinePathEffect) {
    mLength = SPtrCreate(QrealAnimator)("segment length");
    mLength->qra_setValueRange(0., 100.);
    mLength->qra_setCurrentValue(100.);

    mReverse = SPtrCreate(BoolProperty)("reverse");
    mReverse->setValue(false);

    ca_addChildAnimator(mLength);
    ca_addChildAnimator(mReverse);
}

void LengthPathEffect::filterPathForRelFrame(const int &relFrame,
                                                const SkPath &src,
                                                SkPath *dst,
                                                const qreal &,
                                                const bool &) {
    filterPathForRelFrameF(relFrame, src, dst, false);
}

void LengthPathEffect::filterPathForRelFrameF(const qreal &relFrame,
                                                 const SkPath &src,
                                                 SkPath *dst,
                                                 const bool &) {
    qreal targetLen = mLength->getCurrentEffectiveValueAtRelFrameF(relFrame);
    if(targetLen < 0.001) {
        *dst = SkPath();
        return;
    }
    if(targetLen > 99.999) {
        *dst = src;
        return;
    }
    QPainterPath dstT = SkPathToQPainterPath(src);
    if(mReverse->getValue()) {
        dstT = dstT.toReversed();
    }

    qreal finalLen = dstT.length()*targetLen/100.;
    QPointF finalPt = dstT.pointAtPercent(targetLen/100.);

    QPointF pt1Pos;
    QPointF pt1End;
    QPointF pt2Start;
    QPointF pt2Pos;
    qreal pt12T;

    QPainterPath dstT2;
    bool firstOther = true;
    QPointF endPt;
    QPointF startPt;

    QPointF lastPt;
    bool found = false;
    for(int i = 0; i < dstT.elementCount(); i++) {
        const QPainterPath::Element &elem = dstT.elementAt(i);
        if(elem.isMoveTo()) { // move
            dstT2.moveTo(elem.x, elem.y);
            lastPt = QPointF(elem.x, elem.y);
        } else if(elem.isLineTo()) { // line
            QPointF nearestPtT;
            qreal errorT;
            qreal nearestT = gGetClosestTValueOnBezier(
                                {lastPt, lastPt,
                                 QPointF(elem.x, elem.y), QPointF(elem.x, elem.y)},
                                     finalPt, &nearestPtT, &errorT);
            if(errorT < 0.01) {
                QPainterPath dstT3 = dstT2;
                dstT3.lineTo(elem.x, elem.y);
                if(dstT3.length() + 2. > finalLen) {
                    found = true;
                    pt1Pos = lastPt;
                    pt1End = lastPt;
                    pt2Start = QPointF(elem.x, elem.y);
                    pt2Pos = QPointF(elem.x, elem.y);
                    pt12T = nearestT;
                    break;
                }
            }
            dstT2.lineTo(elem.x, elem.y);
            lastPt = QPointF(elem.x, elem.y);
        } else if(elem.isCurveTo()) { // curve
            endPt = QPointF(elem.x, elem.y);
            firstOther = true;
        } else { // other
            if(firstOther) {
                startPt = QPointF(elem.x, elem.y);
            } else {
                QPointF nearestPtT;
                qreal errorT;
                qreal nearestT = gGetClosestTValueOnBezier({lastPt, endPt,
                                         startPt, QPointF(elem.x, elem.y)},
                                         finalPt, &nearestPtT, &errorT);
                if(errorT < 0.01) {
                    QPainterPath dstT3 = dstT2;
                    dstT3.cubicTo(endPt, startPt, QPointF(elem.x, elem.y));
                    if(dstT3.length() + 2. > finalLen) {
                        found = true;
                        pt1Pos = lastPt;
                        pt1End = endPt;
                        pt2Start = startPt;
                        pt2Pos = QPointF(elem.x, elem.y);
                        pt12T = nearestT;
                        break;
                    }
                }
                dstT2.cubicTo(endPt, startPt, QPointF(elem.x, elem.y));
                lastPt = QPointF(elem.x, elem.y);
            }
            firstOther = !firstOther;
        }
    }
    if(!found) {
        *dst = src;
        return;
    }

    QPointF newPointPos;
    QPointF newPointStart;
    QPointF newPointEnd;
    VectorPathEdge::getNewRelPosForKnotInsertionAtT(
                  pt1Pos,
                  &pt1End,
                  &pt2Start,
                  pt2Pos,
                  &newPointPos,
                  &newPointStart,
                  &newPointEnd,
                  pt12T);
    dstT2.cubicTo(pt1End, newPointStart, newPointPos);

    if(mReverse->getValue()) {
        *dst = QPainterPathToSkPath(dstT2.toReversed());
    } else {
        *dst = QPainterPathToSkPath(dstT2);
    }
}
