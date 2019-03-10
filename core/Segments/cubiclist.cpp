#include "cubiclist.h"
#include "pointhelpers.h"

CubicList::CubicList() {

}

CubicList::CubicList(const QList<qCubicSegment2D> &segs) {
    mSegments = segs;
    updateClosed();
}

CubicList::CubicList(const CubicList &src) {
    mSegments = src.getSegments();
    updateClosed();
}

CubicList CubicList::getFragment(const double &minLenFrac,
                                 const double &maxLenFrac) {
    if(minLenFrac > maxLenFrac) return CubicList();
    //Q_ASSERT(minLenFrac > 0);
    //Q_ASSERT(maxLenFrac < 1);
    const double minLen = getTotalLength()*CLAMP01(minLenFrac);
    const double maxLen = getTotalLength()*CLAMP01(maxLenFrac);
    double minT = 0, maxT = 0;
    int minI = -1, maxI = -1;
    double currLen = 0;
    const int iMax = mSegments.count();
    for(int i = 0; i < iMax; i++) {
        auto& seg = mSegments[i];
        const double lastLen = currLen;
        currLen += seg.length();
        if(minI < 0) {
            if(currLen > minLen) {
                minT = seg.tAtLength(minLen - lastLen);
                minI = i;
            }
        }
        if(currLen > maxLen) {
            maxT = seg.tAtLength(maxLen - lastLen);
            maxI = i;
            break;
        }
    }
    if(minI == -1) {
        minI = 0; minT = 0;
    }
    if(maxI == -1) {
        maxI = mSegments.count() - 1; maxT = 1;
    }
    QList<qCubicSegment2D> fragSegs;
    if(minI == maxI) {
        fragSegs << mSegments[minI].tFragment(minT, maxT);
    } else {
        fragSegs << mSegments[minI].dividedAtT(minT).second;
        for(int i = minI + 1; i < maxI; i++) {
            fragSegs << static_cast<qCubicSegment2D>(mSegments[i]);
        }
        fragSegs << mSegments[maxI].dividedAtT(maxT).first;
    }
    return CubicList(fragSegs);
}

CubicList CubicList::getFragmentUnbound(const double &minLenFrac,
                                        const double &maxLenFrac) {
    if(minLenFrac > maxLenFrac) return CubicList();
    const double shiftToPos = -floor4Dec(minLenFrac);
    const double posMinLenFrac = minLenFrac + shiftToPos; // always between 0 and 1
    const double posMaxLenFrac = maxLenFrac + shiftToPos;
    if((isZero4Dec(posMinLenFrac) || posMinLenFrac > 0) &&
       (isZero4Dec(posMaxLenFrac) || posMaxLenFrac < 1)) {
        return getFragment(posMinLenFrac, posMaxLenFrac);
    }

    QList<qCubicSegment2D> fragSegs;
    if(!isInteger4Dec(posMinLenFrac)) {
        fragSegs << getFragment(posMinLenFrac, qCeil(posMinLenFrac)).getSegments();
    }
    const int nFull = qFloor(posMaxLenFrac - posMinLenFrac);
    for(int i = 0; i < nFull; i++) {
        fragSegs << mSegments;
    }
    if(!isInteger4Dec(posMaxLenFrac)) {
        fragSegs << getFragmentUnbound(
                        qFloor(posMaxLenFrac), posMaxLenFrac).getSegments();
    }
    return CubicList(fragSegs);
}

QList<CubicList> CubicList::sMakeFromSkPath(const SkPath &src) {
    QList<CubicList> result;
    QList<qCubicSegment2D> segs;

    QPointF lastMovePos;
    QPointF lastPos;
    SkPath::Iter iter(src, false);
    for(;;) {
        SkPoint pts[4];
        switch(iter.next(pts, true, true)) {
        case SkPath::kLine_Verb: {
            QPointF pt1 = skPointToQ(pts[1]);
            segs << qCubicSegment2D(lastPos, lastPos, pt1, pt1);
            lastPos = pt1;
        } break;
        case SkPath::kQuad_Verb: {
            QPointF pt2 = skPointToQ(pts[2]);
            segs << qCubicSegment2D::fromQuad(lastPos, skPointToQ(pts[1]), pt2);
            lastPos = pt2;
        } break;
        case SkPath::kConic_Verb: {
            QPointF pt2 = skPointToQ(pts[2]);
            segs << qCubicSegment2D::fromConic(lastPos, skPointToQ(pts[1]), pt2,
                                               skScalarToQ(iter.conicWeight()));
            lastPos = pt2;
        } break;
        case SkPath::kCubic_Verb: {
            QPointF pt3 = skPointToQ(pts[3]);
            segs << qCubicSegment2D(lastPos,
                                    skPointToQ(pts[1]),
                                    skPointToQ(pts[2]),
                                    pt3);
            lastPos = pt3;
        } break;
        case SkPath::kClose_Verb: {
//            SkCubicSegment2D seg{lastPos, lastPos, lastMovePos, lastMovePos};
//            segs << qCubicSegment2D(seg);
        } break;
        case SkPath::kMove_Verb: {
            if(!segs.isEmpty()) {
                result << segs;
                segs.clear();
            }
            lastMovePos = skPointToQ(pts[0]);
            lastPos = lastMovePos;
        } break;
        case SkPath::kDone_Verb: {
            if(!segs.isEmpty()) {
                result << segs;
                segs.clear();
            }
            return result;
        }
        }
    }
}

qreal CubicList::getTotalLength() {
    if(!mTotalLengthUpToDate) updateTotalLength();
    return mTotalLength;
}

bool CubicList::isEmpty() const { return mSegments.isEmpty(); }

qreal CubicList::minDistanceTo(const QPointF &p,
                               qreal * const pBestT,
                               QPointF * const pBestPos) {
    qreal bestT = 0;
    QPointF bestPos;
    qreal smallestDist = __DBL_MAX__;
    const int iMax = mSegments.count();
    for(int i = 0; i < iMax; i++) {
        auto& seg = mSegments[i];
        qreal thisT;
        QPointF thisPos;
        qreal thisDist = seg.minDistanceTo(p, &thisT, &thisPos);
        if(thisDist < smallestDist) {
            bestT = thisT;
            bestPos = thisPos;
            smallestDist = thisDist;
        }
    }
    if(pBestT) *pBestT = bestT;
    if(pBestPos) *pBestPos = bestPos;

    return smallestDist;
}

void CubicList::opSmoothOut(const qreal& smoothness) {
    if(mSegments.count() < 2) return;
    qCubicSegment2D * prevSeg = nullptr;
    QPointF lastC2;
    if(mClosed) {
        prevSeg = &mSegments[mSegments.count() - 2];
        auto& seg = mSegments.last();
        QPointF c1 = prevSeg->c2();
        QPointF c2 = seg.c1();
        gSmoothyAbsCtrlsForPtBetween(prevSeg->p0(), seg.p0(), seg.p1(),
                                     c1, c2, smoothness);
        lastC2 = c2;
        prevSeg = &seg;
    }

    for(int i = 0; i < mSegments.count(); i++) {
        auto& seg = mSegments[i];
        if(!mClosed) {
            if(i == 0) {
                prevSeg = &seg;
                lastC2 = prevSeg->c1();
                continue;
            }
        }
        QPointF c1 = prevSeg->c2();
        QPointF c2 = seg.c1();
        gSmoothyAbsCtrlsForPtBetween(prevSeg->p0(), seg.p0(), seg.p1(),
                                     c1, c2, smoothness);
        prevSeg->setC1(lastC2);
        prevSeg->setC2(c1);
        lastC2 = c2;
        prevSeg = &seg;
    }
    if(!mClosed) {
        auto& seg = mSegments.last();
        QPointF c1;
        if(smoothness > 0) {
            c1 = (seg.c2() - seg.p1())*(1 - smoothness) + seg.p1();
        } else {
            c1 = (seg.c2() - seg.p1())*(1 + smoothness) + seg.p1();
        }
        seg.setC1(lastC2);
        seg.setC2(c1);
    }
}

void CubicList::subdivide(const int &sub) {
    if(sub <= 0) return;
    for(int i = 0; i < mSegments.count(); i++) {
        auto& seg = mSegments[i];
        qreal totLen = seg.length();
        qreal lenFrac = totLen/(sub + 1);
        auto remSeg = seg;
        for(int j = 1; j <= static_cast<int>(sub); j++) {
            auto divided = remSeg.dividedAtT(seg.tAtLength(lenFrac));
            auto newSeg = divided.first;
            remSeg = divided.second;
            mSegments.insert(i + j, newSeg);
        }
        mSegments.insert(i + sub + 1, remSeg);
        mSegments.removeAt(i);
        i += sub;
    }
}

void CubicList::smoothOutNode(const int &nodeId) {
    if(nodeId < 0) return;
    if(nodeId > mSegments.count()) return;
    int seg1Id = nodeId - 1;
    int seg2Id = nodeId;
    if(mClosed) {
        if(seg1Id == -1) { // first node
            seg1Id = mSegments.count() - 1;
        } else if(seg2Id >= mSegments.count()) { // last node
            seg2Id = 0;
        }
        auto& seg1 = mSegments[seg1Id];
        auto& seg2 = mSegments[seg2Id];

    } else {
        if(seg1Id == -1) { // first node

        } else if(seg2Id >= mSegments.count()) { // last node

        }
    }
}

void CubicList::updateTotalLength() {
    mTotalLengthUpToDate = true;
    mTotalLength = 0;
    const int iMax = mSegments.count();
    for(int i = 0; i < iMax; i++) {
        mTotalLength += mSegments[i].length();
    }
}

void CubicList::updateClosed() {
    if(mSegments.isEmpty()) {
        mClosed = false;
        return;
    }
    const auto& firstSeg = mSegments.first();
    const auto& lastSeg = mSegments.last();

    mClosed = pointToLen(firstSeg.p0() - lastSeg.p1()) < 0.1;
}
