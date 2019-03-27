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

bool NormalSegment::operator==(const NormalSegment &other) const {
    return mFirstNode == other.getFirstNode() &&
           mLastNode == other.getLastNode();
}

NormalSegment::NormalSegment() {}

NormalSegment::NormalSegment(SmartNodePoint * const firstNode,
                             SmartNodePoint * const lastNode,
                             PathPointsHandler * const handler) :
    mHandler_k(handler) {
    mFirstNode = firstNode;
    if(mFirstNode) mFirstNodeC2 = mFirstNode->getC2Pt();
    mLastNode = lastNode;
    if(mLastNode) mLastNodeC0 = mLastNode->getC0Pt();
    updateDnD();
}

SmartNodePoint *NormalSegment::divideAtAbsPos(const QPointF &absPos) {
    if(!isValid()) return nullptr;
    const int firstId = mFirstNode->getNodeId();
    const int lastId = mLastNode->getNodeId();
    const qreal t = closestAbsT(absPos);
    const auto result = mHandler_k->divideSegment(firstId, lastId, t);
    reset();
    return result;
}

void NormalSegment::disconnect() const {
    mHandler_k->removeSegment(*this);
}

int NormalSegment::nodesCount() const {
    return mInnerDnD.count() + (mFirstNode ? 1 : 0) + (mLastNode ? 1 : 0);
}

SmartNodePoint *NormalSegment::getNodeAt(const int &id) const {
    if(id < 0 || id >= nodesCount()) return nullptr;
    if(id == 0) return mFirstNode;
    const int innerId = id - 1;
    if(innerId < mInnerDnD.count()) mInnerDnD.at(innerId);
    return mLastNode;
}

NormalSegment::SubSegment NormalSegment::getClosestSubSegmentForDummy(
        const QPointF &relPos, qreal &minDist) const {
    auto prevNode = mFirstNode;
    minDist = TEN_MIL;
    SubSegment bestSeg{nullptr, nullptr, nullptr};
    for(const auto &nextNode : mInnerDnD) {
        const auto subSeg = SubSegment{prevNode, nextNode, this};
        const QPointF halfPos = subSeg.getRelPosAtT(0.5);
        const qreal dist = pointToLen(halfPos - relPos);
        if(dist < minDist) {
            minDist = dist;
            bestSeg = subSeg;
        }
        prevNode = nextNode;
    }
    const auto subSeg = SubSegment{prevNode, mLastNode, this};
    const QPointF halfPos = subSeg.getRelPosAtT(0.5);
    const qreal dist = pointToLen(halfPos - relPos);
    if(dist < minDist) {
        minDist = dist;
        bestSeg = subSeg;
    }
    return bestSeg;
}

void NormalSegment::updateDnDPos() const {
    for(const auto& inner : mInnerDnD)
        inner->updateFromNodeDataPosOnly();
}

QPointF NormalSegment::getRelPosAtT(const qreal &t) const {
    return getAsRelSegment().posAtT(t);
}

QPointF NormalSegment::getAbsPosAtT(const qreal &t) const {
    return getAsAbsSegment().posAtT(t);
}

void NormalSegment::makePassThroughAbs(const QPointF &absPos, const qreal& t) {
    if(!mLastNode->getC0Enabled()) mLastNode->setC0Enabled(true);
    if(!mFirstNode->getC2Enabled()) mFirstNode->setC2Enabled(true);

    auto absSeg = getAsAbsSegment();

    QPointF dPos = absPos - gCubicValueAtT(absSeg, t);
    while(pointToLen(dPos) > 1) {
        absSeg.setC1(absSeg.c1() + (1 - t)*dPos);
        absSeg.setC2(absSeg.c2() + t*dPos);

        dPos = absPos - gCubicValueAtT(absSeg, t);
    }

    mFirstNodeC2->moveToAbs(absSeg.c1());
    mLastNodeC0->moveToAbs(absSeg.c2());
}

void NormalSegment::makePassThroughRel(const QPointF &relPos, const qreal &t) {
    if(!mLastNode->getC0Enabled()) mLastNode->setC0Enabled(true);
    if(!mFirstNode->getC2Enabled()) mFirstNode->setC2Enabled(true);

    auto relSeg = getAsRelSegment();

    QPointF dPos = relPos - gCubicValueAtT(relSeg, t);
    while(pointToLen(dPos) > 1) {
        relSeg.setC1(relSeg.c1() + (1 - t)*dPos);
        relSeg.setC2(relSeg.c2() + t*dPos);

        dPos = relPos - gCubicValueAtT(relSeg, t);
    }

    mFirstNodeC2->moveToRel(relSeg.c1());
    mLastNodeC0->moveToRel(relSeg.c2());
}

void NormalSegment::finishPassThroughTransform() {
    mFirstNodeC2->finishTransform();
    mLastNodeC0->finishTransform();
}

void NormalSegment::startPassThroughTransform() {
    mFirstNodeC2->startTransform();
    mLastNodeC0->startTransform();
}

void NormalSegment::cancelPassThroughTransform() {
    mFirstNodeC2->cancelTransform();
    mLastNodeC0->cancelTransform();
}

void NormalSegment::generateSkPath() {
    mSkPath.reset();
    mSkPath.moveTo(toSkPoint(mFirstNode->getAbsolutePos()));
    mSkPath.cubicTo(toSkPoint(mFirstNode->getC2AbsPos()),
                    toSkPoint(mLastNode->getC0AbsPos()),
                    toSkPoint(mLastNode->getAbsolutePos()));
}

void NormalSegment::drawHoveredSk(SkCanvas * const canvas,
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

SmartNodePoint *NormalSegment::getFirstNode() const {
    return mFirstNode;
}

SmartNodePoint *NormalSegment::getLastNode() const {
    return mLastNode;
}

qCubicSegment2D NormalSegment::getAsAbsSegment() const {
    Q_ASSERT(mFirstNode && mLastNode);
    return {mFirstNode->getAbsolutePos(),
            mFirstNode->getC2AbsPos(),
            mLastNode->getC0AbsPos(),
            mLastNode->getAbsolutePos()};
}

qCubicSegment2D NormalSegment::getAsRelSegment() const {
    Q_ASSERT(mFirstNode && mLastNode);
    return {mFirstNode->getRelativePos(),
            mFirstNode->getC2Value(),
            mLastNode->getC0Value(),
            mLastNode->getRelativePos()};
}

void NormalSegment::updateDnD() {
    mInnerDnD.clear();
    if(!mFirstNode || !mLastNode) return;
    auto currNode = mFirstNode->getNextPoint();
    while(currNode && currNode != mLastNode) {
        mInnerDnD << currNode;
        currNode = currNode->getNextPoint();
    }
    updateDnDPos();
}

NormalSegment::SubSegment NormalSegment::subSegmentAtT(const qreal &t) const {
    SmartNodePoint* firstNode = mFirstNode;
    SmartNodePoint* lastNode = mLastNode;
    qreal firstNodeT = 0;
    qreal lastNodeT = 1;
    for(const auto& inner : mInnerDnD) {
        const qreal innerT = inner->getT();
        if(innerT < t && innerT > firstNodeT) {
            firstNodeT = innerT;
            firstNode = inner;
        }
        if(innerT > t && innerT < lastNodeT) {
            lastNodeT = innerT;
            lastNode = inner;
        }
    }
    return {firstNode, lastNode, this};
}

QPointF NormalSegment::getSlopeVector(const qreal &t) {
    const QPointF posAtT = getRelPosAtT(t);
    const QPointF posAtTPlus = getRelPosAtT(t + 0.01);
    return scalePointToNewLen(posAtTPlus - posAtT, 1);
}

bool NormalSegment::isValid() const {
    return mFirstNode && mFirstNodeC2 &&
           mLastNodeC0 && mLastNode && mHandler_k;
}

bool NormalSegment::isNormal() const {
    if(!isValid()) return false;
    return mFirstNode->getType() == Node::NORMAL &&
            mLastNode->getType() == Node::NORMAL;
}

void NormalSegment::clear() {
    mFirstNode = nullptr;
    mFirstNodeC2 = nullptr;
    mLastNodeC0 = nullptr;
    mLastNode = nullptr;
    mInnerDnD.clear();
}

qreal NormalSegment::SubSegment::getMinT() const {
    if(fFirstPt == fParentSeg->getFirstNode()) return 0;
    return fFirstPt->getT();
}

qreal NormalSegment::SubSegment::getMaxT() const {
    if(fLastPt == fParentSeg->getLastNode()) return 1;
    return fLastPt->getT();
}

qreal NormalSegment::SubSegment::getParentTAtThisT(const qreal& thisT) const {
    return gMapTFromFragment(getMinT(), getMaxT(), thisT);
}

QPointF NormalSegment::SubSegment::getRelPosAtT(const qreal& thisT) const {
    const qreal parentT = getParentTAtThisT(thisT);
    return fParentSeg->getRelPosAtT(parentT);
}
