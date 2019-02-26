#include "segment.h"
#include "MovablePoints/smartnodepoint.h"
#include "MovablePoints/smartctrlpoint.h"
#include "Boxes/boundingbox.h"
#include "global.h"
#include "Animators/PathAnimators/vectorpathanimator.h"
#include "pointhelpers.h"
#include "GUI/mainwindow.h"
#include "Animators/transformanimator.h"
#include "pathpointshandler.h"

Segment::Segment(PathPointsHandler * const handler) :
    mHandler_k(handler),
    mPoint1(nullptr),
    mPoint1C2Pt(nullptr),
    mPoint2C0Pt(nullptr),
    mPoint2(nullptr) {}

Segment::Segment(SmartNodePoint * const pt1, SmartNodePoint * const pt2,
                 PathPointsHandler * const handler) : mHandler_k(handler) {
    setPoint1(pt1);
    setPoint2(pt2);
}

void Segment::setPoint1(SmartNodePoint * const pt1) {
    mPoint1 = pt1;
    if(mPoint1) mPoint1C2Pt = mPoint1->getC2Pt();
}

void Segment::setPoint2(SmartNodePoint * const pt2) {
    mPoint2 = pt2;
    if(mPoint2) mPoint2C0Pt = mPoint2->getC0Pt();
}

void Segment::disconnect() const {
    mPoint1->setPointAsNext(nullptr);
}

QPointF Segment::getRelPosAtT(const qreal &t) const {
    return getAsRelSegment().posAtT(t);
}

QPointF Segment::getAbsPosAtT(const qreal &t) const {
    return getAsAbsSegment().posAtT(t);
}

void Segment::makePassThroughAbs(const QPointF &absPos, const qreal& t) {
    if(!mPoint2->getC0Enabled()) mPoint2->setC0Enabled(true);
    if(!mPoint1->getC2Enabled()) mPoint1->setC2Enabled(true);

    auto absSeg = getAsAbsSegment();

    QPointF dPos = absPos - gCubicValueAtT(absSeg, t);
    while(pointToLen(dPos) > 1) {
        absSeg.setC1(absSeg.c1() + (1 - t)*dPos);
        absSeg.setC2(absSeg.c2() + t*dPos);

        dPos = absPos - gCubicValueAtT(absSeg, t);
    }

    mPoint1C2Pt->moveToAbs(absSeg.c1());
    mPoint2C0Pt->moveToAbs(absSeg.c2());
}

void Segment::makePassThroughRel(const QPointF &relPos, const qreal &t) {
    if(!mPoint2->getC0Enabled()) mPoint2->setC0Enabled(true);
    if(!mPoint1->getC2Enabled()) mPoint1->setC2Enabled(true);

    auto relSeg = getAsRelSegment();

    QPointF dPos = relPos - gCubicValueAtT(relSeg, t);
    while(pointToLen(dPos) > 1) {
        relSeg.setC1(relSeg.c1() + (1 - t)*dPos);
        relSeg.setC2(relSeg.c2() + t*dPos);

        dPos = relPos - gCubicValueAtT(relSeg, t);
    }

    mPoint1C2Pt->moveToRel(relSeg.c1());
    mPoint2C0Pt->moveToRel(relSeg.c2());
}

void Segment::finishPassThroughTransform() {
    mPoint1C2Pt->finishTransform();
    mPoint2C0Pt->finishTransform();
}

void Segment::startPassThroughTransform() {
    mPoint1C2Pt->startTransform();
    mPoint2C0Pt->startTransform();
}

void Segment::cancelPassThroughTransform() {
    mPoint1C2Pt->cancelTransform();
    mPoint2C0Pt->cancelTransform();
}

void Segment::generateSkPath() {
    mSkPath = SkPath();
    mSkPath.moveTo(qPointToSk(mPoint1->getAbsolutePos()));
    mSkPath.cubicTo(qPointToSk(mPoint1->getC2AbsPos()),
                    qPointToSk(mPoint2->getC0AbsPos()),
                    qPointToSk(mPoint2->getAbsolutePos()));
}

void Segment::drawHoveredSk(SkCanvas * const canvas,
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

SmartNodePoint *Segment::getPoint1() const {
    return mPoint1;
}

SmartNodePoint *Segment::getPoint2() const {
    return mPoint2;
}

qCubicSegment2D Segment::getAsAbsSegment() const {
    Q_ASSERT(mPoint1 && mPoint2);
    return {mPoint1->getAbsolutePos(),
            mPoint1->getC2AbsPos(),
            mPoint2->getC0AbsPos(),
            mPoint2->getAbsolutePos()};
}

qCubicSegment2D Segment::getAsRelSegment() const {
    Q_ASSERT(mPoint1 && mPoint2);
    return {mPoint1->getRelativePos(),
            mPoint1->getC2Value(),
            mPoint2->getC0Value(),
            mPoint2->getRelativePos()};
}

QPointF Segment::getSlopeVector(const qreal &t) {
    QPointF posAtT = getRelPosAtT(t);
    QPointF posAtTPlus = getRelPosAtT(t + 0.01);
    return scalePointToNewLen(posAtTPlus - posAtT, 1);
}
