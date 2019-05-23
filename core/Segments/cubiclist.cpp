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

CubicList CubicList::getFragment(const qreal &minLenFrac,
                                 const qreal &maxLenFrac) {
    if(minLenFrac > maxLenFrac) return CubicList();
    //Q_ASSERT(minLenFrac > 0);
    //Q_ASSERT(maxLenFrac < 1);
    const qreal minLen = getTotalLength()*CLAMP01(minLenFrac);
    const qreal maxLen = getTotalLength()*CLAMP01(maxLenFrac);
    qreal minT = 0, maxT = 1;
    int minI = -1, maxI = mSegments.count() - 1;
    qreal currLen = 0;
    const int iMax = mSegments.count();
    for(int i = 0; i < iMax; i++) {
        auto& seg = mSegments[i];
        const qreal lastLen = currLen;
        currLen += seg.length();
        if(minI == -1) {
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

CubicList CubicList::getFragmentUnbound(const qreal &minLenFrac,
                                        const qreal &maxLenFrac) {
    if(minLenFrac > maxLenFrac) return CubicList();
    const qreal shiftToPos = -floor4Dec(minLenFrac);
    const qreal posMinLenFrac = minLenFrac + shiftToPos; // always between 0 and 1
    const qreal posMaxLenFrac = maxLenFrac + shiftToPos;
    QList<qCubicSegment2D> fragSegs;
    if(isInteger4Dec(posMinLenFrac) && isInteger4Dec(posMaxLenFrac)) {
        const int nFull = qRound(posMaxLenFrac - posMinLenFrac);
        for(int i = 0; i < nFull; i++) fragSegs << mSegments;
        return fragSegs;
    }

    if(!isInteger4Dec(posMinLenFrac)) {
        fragSegs << getFragment(qCeil(posMinLenFrac) - posMinLenFrac, 1).getSegments();
    }
    const int nFull = qFloor(posMaxLenFrac - posMinLenFrac);
    for(int i = 0; i < nFull; i++) fragSegs << mSegments;

    if(!isInteger4Dec(posMaxLenFrac)) {
        fragSegs << getFragment(0, posMaxLenFrac - qFloor(posMaxLenFrac)).getSegments();
    }
    return fragSegs;
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
            QPointF pt1 = toQPointF(pts[1]);
            segs << qCubicSegment2D(lastPos, lastPos, pt1, pt1);
            lastPos = pt1;
        } break;
        case SkPath::kQuad_Verb: {
            QPointF pt2 = toQPointF(pts[2]);
            segs << qCubicSegment2D::sFromQuad(lastPos, toQPointF(pts[1]), pt2);
            lastPos = pt2;
        } break;
        case SkPath::kConic_Verb: {
            QPointF pt2 = toQPointF(pts[2]);
            segs << qCubicSegment2D::sFromConic(lastPos, toQPointF(pts[1]), pt2,
                                               toQreal(iter.conicWeight()));
            lastPos = pt2;
        } break;
        case SkPath::kCubic_Verb: {
            QPointF pt3 = toQPointF(pts[3]);
            segs << qCubicSegment2D(lastPos,
                                    toQPointF(pts[1]),
                                    toQPointF(pts[2]),
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
            lastMovePos = toQPointF(pts[0]);
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
        gSmoothyAbsCtrlsForPtBetween(prevSeg->p0(), seg.p0(), seg.p3(),
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
        gSmoothyAbsCtrlsForPtBetween(prevSeg->p0(), seg.p0(), seg.p3(),
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
            c1 = (seg.c2() - seg.p3())*(1 - smoothness) + seg.p3();
        } else {
            c1 = (seg.c2() - seg.p3())*(1 + smoothness) + seg.p3();
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

void CubicList::updateTotalLength() {
    mTotalLengthUpToDate = true;
    mTotalLength = 0;
    for(auto& seg : mSegments)
        mTotalLength += seg.length();
}

void CubicList::updateClosed() {
    if(mSegments.isEmpty()) {
        mClosed = false;
        return;
    }
    const auto& firstSeg = mSegments.first();
    const auto& lastSeg = mSegments.last();

    mClosed = pointToLen(firstSeg.p0() - lastSeg.p3()) < 0.1;
}

int cubicBezierLine(const qCubicSegment2D& seg,
                    const QLineF& line,
                    QList<QPointF>& result);

bool ascendingX(const QPointF& pt1, const QPointF& pt2) {
    return pt1.x() < pt2.x();
}

bool descendingX(const QPointF& pt1, const QPointF& pt2) {
    return pt1.x() > pt2.x();
}

bool ascendingY(const QPointF& pt1, const QPointF& pt2) {
    return pt1.y() < pt2.y();
}

bool descendingY(const QPointF& pt1, const QPointF& pt2) {
    return pt1.y() > pt2.y();
}

int CubicList::lineIntersections(const QLineF &line, QList<QPointF> &pts) {
    if(isZero6Dec(line.length())) return 0;
    for(auto& seg : mSegments) cubicBezierLine(seg, line, pts);
    if(isZero6Dec(line.dx())) {
        if(line.dy() > 0) {
            std::sort(pts.begin(), pts.end(), ascendingY);
        } else {
            std::sort(pts.begin(), pts.end(), descendingY);
        }
    } else {
        if(line.dx() > 0) {
            std::sort(pts.begin(), pts.end(), ascendingX);
        } else {
            std::sort(pts.begin(), pts.end(), descendingX);
        }
    }
    return pts.count();
}

QList<qreal> getCubicRoots(const qreal& C0, const qreal& C1,
                           const qreal& C2, const qreal& C3) {
    QList<qreal> results;
    const qreal c3 = C3;
    const qreal c2 = C2 / c3;
    const qreal c1 = C1 / c3;
    const qreal c0 = C0 / c3;

    const qreal a       = (3 * c1 - c2 * c2) / 3;
    const qreal b       = (2 * c2 * c2 * c2 - 9 * c1 * c2 + 27 * c0) / 27;
    const qreal offset  = c2 / 3;
    qreal discrim = b * b / 4 + a * a * a / 27;
    const qreal halfB   = b / 2;
    qreal tmp, root;

    if(isZero6Dec(discrim)) discrim = 0;

    if(discrim > 0) {
        const qreal e = qSqrt(discrim);

        tmp = -halfB + e;
        if(tmp >= 0)root = pow(tmp, 1./3);
        else root = -pow(-tmp, 1./3);

        tmp = -halfB - e;
        if(tmp >= 0) root += pow( tmp, 1./3);
        else root -= pow(-tmp, 1./3);

        results.append(root - offset);
    } else if (discrim < 0) {
        const qreal distance = sqrt(-a/3);
        const qreal angle    = atan2(sqrt(-discrim), -halfB) / 3;
        const qreal cos      = qCos(angle);
        const qreal sin      = qSin(angle);

        results.append(2 * distance * cos - offset);
        results.append(-distance * (cos + sqrt3 * sin) - offset);
        results.append(-distance * (cos - sqrt3 * sin) - offset);
    } else {
        if (halfB >= 0) tmp = -pow(halfB, 1./3);
        else            tmp =  pow(-halfB, 1./3);

        results.append(2 * tmp - offset);
        // really should return next root twice, but we return only one
        results.append(-tmp - offset);
    }

    return results;
}

int cubicBezierLine(const qreal& p1x, const qreal& p1y,
                    const qreal& p2x, const qreal& p2y,
                    const qreal& p3x, const qreal& p3y,
                    const qreal& p4x, const qreal& p4y,
                    const qreal& a1x, const qreal& a1y,
                    const qreal& a2x, const qreal& a2y,
                    QList<QPointF>& result) {
    qreal ax, ay, bx, by, cx, cy, dx, dy;         // temporary variables
    qreal c3x, c3y, c2x, c2y, c1x, c1y, c0x, c0y; // coefficients of cubic

    // Start with Bezier using Bernstein polynomials for weighting functions:
    //     (1-t^3)P1 + 3t(1-t)^2P2 + 3t^2(1-t)P3 + t^3P4
    //
    // Expand and collect terms to form linear combinations of original Bezier
    // controls.  This ends up with a vector cubic in t:
    //     (-P1+3P2-3P3+P4)t^3 + (3P1-6P2+3P3)t^2 + (-3P1+3P2)t + P1
    //             ||                  ||                ||       ||
    //             c3                  c2                c1       c0

    // Calculate the coefficients
    ax = p1x * -1; ay = p1y * -1;
    bx = p2x * 3;  by = p2y * 3;
    cx = p3x * -3; cy = p3y * -3;
    dx = ax + bx + cx + p4x;
    dy = ay + by + cy + p4y;
    c3x = dx; c3y = dy; // vec

    ax = p1x * 3;  ay = p1y * 3;
    bx = p2x * -6; by = p2y * -6;
    cx = p3x * 3;  cy = p3y * 3;
    dx = ax + bx + cx;
    dy = ay + by + cy;
    c2x = dx; c2y = dy; // vec

    ax = p1x * -3; ay = p1y * -3;
    bx = p2x * 3;  by = p2y * 3;
    cx = ax + bx;
    cy = ay + by;
    c1x = cx;
    c1y = cy; // vec

    c0x = p1x;
    c0y = p1y;

    // Convert line to normal form: ax + by + c = 0
    // Find normal to line: negative inverse of original line's slope
    const qreal nx = a1y - a2y;
    const qreal ny = a2x - a1x;

    // Determine new c coefficient
    const qreal cl = a1x * a2y - a2x * a1y;

    // ?Rotate each cubic coefficient using line for new coordinate system?
    // Find roots of rotated cubic
    const QList<qreal> roots = getCubicRoots(
        // dot products => x * x + y * y
        nx * c3x + ny * c3y,
        nx * c2x + ny * c2y,
        nx * c1x + ny * c1y,
        nx * c0x + ny * c0y + cl
    );

    // used to determine if point is on line segment
    const qreal minx = qMin(a1x, a2x);
    const qreal miny = qMin(a1y, a2y);
    const qreal maxx = qMax(a1x, a2x);
    const qreal maxy = qMax(a1y, a2y);

    // Any roots in closed interval [0,1] are intersections on Bezier, but
    // might not be on the line segment.
    // Find intersections and calculate point coordinates
    for(const qreal& t : roots) {
        if(0 > t || t > 1) continue;
        // Find point on Bezier
        // lerp: x1 + (x2 - x1) * t
        const qreal p5x = p1x + (p2x - p1x) * t;
        const qreal p5y = p1y + (p2y - p1y) * t; // lerp(p1, p2, t);

        const qreal p6x = p2x + (p3x - p2x) * t;
        const qreal p6y = p2y + (p3y - p2y) * t;

        const qreal p7x = p3x + (p4x - p3x) * t;
        const qreal p7y = p3y + (p4y - p3y) * t;

        const qreal p8x = p5x + (p6x - p5x) * t;
        const qreal p8y = p5y + (p6y - p5y) * t;

        const qreal p9x = p6x + (p7x - p6x) * t;
        const qreal p9y = p6y + (p7y - p6y) * t;

        // candidate
        const qreal p10x = p8x + (p9x - p8x) * t;
        const qreal p10y = p8y + (p9y - p8y) * t;

        // See if point is on line segment
        if(isZero6Dec(a1x - a2x)) {                       // vertical
            if(miny <= p10y && p10y <= maxy) {
                result.append({p10x, p10y});
            }
        } else if(isZero6Dec(a1y - a2y)) {               // horizontal
            if(minx <= p10x && p10x <= maxx) {
                result.append({p10x, p10y});
            }
        } else if(p10x >= minx && p10y >= miny &&
                  p10x <= maxx && p10y <= maxy) {
            result.append({p10x, p10y});
        }
    }
    return result.count();
}

int cubicBezierLine(const qCubicSegment2D& seg,
                    const QLineF& line,
                    QList<QPointF>& result) {
    const auto& p0 = seg.p0();
    const auto& p1 = seg.c1();
    const auto& p2 = seg.c2();
    const auto& p3 = seg.p3();
    return cubicBezierLine(p0.x(), p0.y(), p1.x(), p1.y(),
                           p2.x(), p2.y(), p3.x(), p3.y(),
                           line.x1(), line.y1(), line.x2(), line.y2(),
                           result);
}
