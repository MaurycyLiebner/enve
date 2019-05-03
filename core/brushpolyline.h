#ifndef BRUSHPOLYLINE_H
#define BRUSHPOLYLINE_H
#include "polyline.h"
#include "framerange.h"

struct BrushPt {
    QPointF fPos;
    qreal fRadius;
    qreal fPressure;
    qreal fT; // initially corresponds to length

    BrushPt operator+(const BrushPt& other) const {
        return {fPos + other.fPos,
                fRadius + other.fRadius,
                fPressure + other.fPressure,
                fT + other.fT};
    }

    BrushPt operator*(const qreal& val) const {
        return {fPos*val, fRadius*val, fPressure*val, fT*val};
    }
};

class BrushPolyline : public Polyline<3> {
public:
    BrushPolyline() {}
    BrushPolyline(const BrushPolyline & src,
                  const int& minId, const int& maxId) {
        mPts = src.mPts.mid(minId, maxId - minId + 1);
        for(const auto& moveId : src.mMovePtIds) {
            if(moveId < minId) continue;
            if(moveId > maxId) break;
            mMovePtIds.append(moveId);
        }
    }

    class Stroker {
        struct StrokerPt {
            QPointF fPos;
            qreal fRadius;
            qreal fPressure;

            StrokerPt operator+(const StrokerPt& other) const {
                return {fPos + other.fPos,
                        fRadius + other.fRadius,
                        fPressure + other.fPressure};
            }

            StrokerPt operator*(const qreal& val) const {
                return {fPos*val, fRadius*val, fPressure*val};
            }
        };
    public:
        Stroker(BrushPolyline& polyline) : mPolyline(polyline) {}

        void setWindowize(const int& wSize) {
            mNSamples = 1 + 2*clamp(wSize, 0, 99);
            processPts();
        }

        void strokeTo(const QPointF& pos,
                      const qreal& radius,
                      const qreal& pressure) {
            mPts.append({pos, radius, pressure});
            processPts();
        }
    private:
        void processPts() {
            while(mPts.count() >= mNSamples) {
                StrokerPt sum{{0, 0}, 0, 0};
                for(int i = 0; i < mNSamples; i++) {
                    sum = sum + mPts.at(i);
                }
                mPolyline.lineTo(sum.fPos/mNSamples,
                                 sum.fRadius/mNSamples,
                                 sum.fPressure/mNSamples);
                mPts.removeFirst();
            }
        }

        BrushPolyline& mPolyline;
        QList<StrokerPt> mPts;
        int mNSamples = 1;
    };

    static BrushPolyline sInterpolate(
            const BrushPolyline& ply1, const BrushPolyline& ply2,
            const qreal& weight2) {
        const auto path1Range = ply1.tRange();
        const auto path2Range = ply2.tRange();

        const qreal weight1 = 1 - weight2;
        const auto iRange = path1Range*weight1 + path2Range*weight2;

        const auto iPath1Range = iRange*path1Range;
        const auto iPath2Range = iRange*path2Range;
        const auto iSharedRange = iPath1Range*iPath2Range;
        auto ply1Part = ply1.mid(iPath1Range);
        if(!iSharedRange.isValid()) {
            auto ply2Part = ply2.mid(iPath2Range);
            ply1Part.remesh(5);
            ply2Part.remesh(5);
            return ply1Part;
        }

        if(iPath2Range.fMin < iPath1Range.fMin) { // path 1 has less before
            auto before1 = ply2.mid(iPath2Range.fMin, iPath1Range.fMin);
            ply1Part.prepend(before1);
        }

        if(iPath1Range.fMax < iPath2Range.fMax) { // path 1 has less after
            auto after1 = ply2.mid(iPath1Range.fMax, iPath2Range.fMax);
            ply1Part.append(after1);
        }

        ply1Part.addPtAtT(iSharedRange.fMin);
        ply1Part.addPtAtT(iSharedRange.fMax);
        const int minShared = ply1Part.idClosestToT(iSharedRange.fMin);
        const int maxShared = ply1Part.idClosestToT(iSharedRange.fMax);
        for(int i = minShared; i <= maxShared; i++) {
            auto& pt = ply1Part.mPts[i];
            const auto pt2 = ply2.ptAtT(pt.fVals[4]);
            pt = pt*weight1 + pt2*weight2;
        }
        ply1Part.remesh(5);
        return ply1Part;
    }

    BrushPolyline mid(const qValueRange& range) const {
        return mid(range.fMin, range.fMax);
    }

    BrushPolyline mid(const qreal& minT, const qreal& maxT) const {
        auto ply = mid(idAfterT(minT), idBeforeT(maxT));
        ply.addPtAtT(minT);
        ply.addPtAtT(maxT);
        return ply;
    }

    BrushPolyline mid(const int& minId) const {
        const int maxId = mPts.count() - 1;
        return mid(minId, maxId - minId + 1);
    }


    BrushPolyline mid(int minId, int maxId) const {
        minId = clamp(minId, 0, mPts.count() - 1);
        maxId = clamp(maxId, 0, mPts.count() - 1);
        if(minId > maxId) return BrushPolyline();
        return BrushPolyline(*this, minId, maxId);
    }

    void addPtAtT(const qreal& t) {
        const QPointF closest = posAtId(idClosestToT(t));
        if(pointToLen(closest - posAtT(t)) < 1) return;
        const int idB = idBeforeT(t);
        mPts.insert(idB + 1, ptAtT(t));
    }

    PolylinePt<3> ptAtT(const qreal& t) const {
        const int idBefore = idBeforeT(t);
        const auto pB = mPts.at(idBefore);
        if(idBefore >= mPts.count() - 1) return pB;
        const int idAfter = idBefore + 1;
        const auto pA = mPts.at(idAfter);
        const qreal tB = tAtId(idBefore);
        const qreal tA = tAtId(idAfter);
        const qreal frac = (t - tB)/(tA - tB);
        return pB*(1 - frac) + pA*frac;
    }

    QPointF posAtT(const qreal& t) const {
        const int idBefore = idBeforeT(t);
        const auto pB = posAtId(idBefore);
        if(idBefore >= mPts.count() - 1) return pB;
        const int idAfter = idBefore + 1;
        const auto pA = posAtId(idAfter);
        const qreal tB = tAtId(idBefore);
        const qreal tA = tAtId(idAfter);
        const qreal frac = (t - tB)/(tA - tB);
        return (1 - frac)*pB + frac*pA;
    }

    QPointF posAtId(const int& id) const {
        return reinterpret_cast<const QPointF&>(mPts[id]);
    }

    int idAfterT(const qreal& t) const {
        return idAfterT(t, 0, mPts.size() - 1);
    }

    int idBeforeT(const qreal& t) const {
        return idBeforeT(t, 0, mPts.size() - 1);
    }

    int idClosestToT(const qreal& t) {
        const int idAfter = idAfterT(t);
        const int idBefore = idBeforeT(t);
        if(idAfter == -1) return idBefore;
        else if(idBefore == -1) return idAfter;

        const auto& ptBefore = reinterpret_cast<BrushPt&>(mPts[idBefore]);
        const auto& ptAfter = reinterpret_cast<BrushPt&>(mPts[idAfter]);
        if(qAbs(ptBefore.fT - t) > qAbs(ptAfter.fT - t)) return idAfter;
        return idBefore;
    }

    void remesh(const qreal& targetDist) {
        remesh(0, mPts.count() - 1, targetDist);
    }

    void remesh(int minId, int maxId, const qreal& targetDist) {
        minId = clamp(minId, 0, mPts.count() - 1);
        maxId = clamp(maxId, 0, mPts.count() - 1);
        if(minId > maxId && isClosed()) {
            QVector<PolylinePt<3>> newPtsAfter;
            const auto begin = mPts.begin() + maxId;
            const auto end = mPts.begin() + minId;

            QPointF prevPtPos = reinterpret_cast<QPointF&>(*begin);
            auto& prevPt = reinterpret_cast<BrushPt&>(*begin);
            newPtsAfter.append(reinterpret_cast<PolylinePt<3>&>(prevPt));
            for(auto it = begin + 1; it != mPts.end(); ++it) {
                auto pt = reinterpret_cast<BrushPt&>(it);
                const QPointF ptPos = reinterpret_cast<QPointF&>(pt);
                const QPointF change = ptPos - prevPtPos;
                const qreal dist = pointToLen(change);
                if(dist > targetDist) {
                    const qreal weight = targetDist/dist;
                    prevPt = prevPt*(1 - weight) + pt*weight;
                    newPtsAfter.append(reinterpret_cast<PolylinePt<3>&>(prevPt));
                }
            }
            QVector<PolylinePt<3>> newPtsBefore;
            for(auto it = mPts.begin(); it != end; ++it) {
                auto pt = reinterpret_cast<BrushPt&>(it);
                const QPointF ptPos = reinterpret_cast<QPointF&>(pt);
                const QPointF change = ptPos - prevPtPos;
                const qreal dist = pointToLen(change);
                if(dist > targetDist) {
                    const qreal weight = targetDist/dist;
                    prevPt = prevPt*(1 - weight) + pt*weight;
                    newPtsBefore.append(reinterpret_cast<PolylinePt<3>&>(prevPt));
                }
            }
            newPtsBefore.append(*end);

            removeAfter(minId);
            remove(0, maxId);

            mPts.swap(newPtsBefore);
            mPts.append(newPtsBefore);
            mPts.append(newPtsAfter);
            return;
        }
        if(minId + 2 > maxId) return;
        QVector<PolylinePt<3>> newPts;
        const auto begin = mPts.begin() + minId;
        const auto end = mPts.begin() + maxId;

        QPointF prevPtPos = reinterpret_cast<QPointF&>(*begin);
        auto& prevPt = reinterpret_cast<BrushPt&>(*begin);
        newPts.append(reinterpret_cast<PolylinePt<3>&>(prevPt));
        for(auto it = begin + 1; it != end; ++it) {
            auto pt = reinterpret_cast<BrushPt&>(it);
            const QPointF ptPos = reinterpret_cast<QPointF&>(pt);
            const QPointF change = ptPos - prevPtPos;
            const qreal dist = pointToLen(change);
            if(dist > targetDist) {
                const qreal weight = targetDist/dist;
                prevPt = prevPt*(1 - weight) + pt*weight;
                newPts.append(reinterpret_cast<PolylinePt<3>&>(prevPt));
            }
        }
        newPts.append(*end);
        const auto after = getAfter(maxId);
        removeAfter(minId);
        mPts.append(newPts);
        mPts.append(after);
    }

    void lineTo(const QPointF& pos, const qreal& radius,
                const qreal& pressure) {
        qreal t;
        if(mPts.isEmpty()) t = 0;
        else {
            const QPointF lastPos = reinterpret_cast<QPointF&>(mPts.last());
            t = maxT() + pointToLen(pos - lastPos);
        }
        mPts.append({pos.x(), pos.y(), radius, pressure, t});
    }

    void replace(int minId, int maxId, const BrushPolyline& src) {
        if(maxId == -1) maxId = mPts.count() - 1;
        minId = clamp(minId, 0, mPts.count() - 1);
        maxId = clamp(maxId, 0, mPts.count() - 1);
        if(minId > maxId) {
            if(!isClosed()) return;
            const qreal thisBLen = length(minId, -1);
            const qreal thisALen = length(0, maxId);
            const qreal bLenFrac = thisBLen/(thisBLen + thisALen);
            const qreal srcLen = src.length();
            const qreal srcBLen = srcLen*bLenFrac;
            const int closingId = src.idBeforeLength(srcBLen);
            replace(minId, -1, src.mid(0, closingId + 1));
            replace(0, maxId, src.mid(closingId + 1));
            return;
        }
        const qreal minT = tAtId(minId);
        const qreal maxT = tAtId(maxId);
        const auto after = getAfter(maxId);
        removeAfter(minId);
        auto mappedSrc = src;
        mappedSrc.setTRange({minT, maxT});
        mPts.append(mappedSrc.mPts);
        mPts.append(after);
    }

    QVector<PolylinePt<3>> getAfter(const int& minId) {
        if(minId >= mPts.count()) return QVector<PolylinePt<3>>();
        return mPts.mid(minId + 1);
    }

    void removeAfter(const int& minId) {
        if(minId >= mPts.count()) return;
        remove(minId, mPts.count() - minId);
    }

    void remove(const int& minId, const int& maxId) {
        if(minId >= mPts.count()) return;
        if(minId > maxId) {
            if(!isClosed()) RuntimeThrow("Invalid id range");
            removeAfter(minId);
            remove(0, maxId);
            return;
        }
        mPts.remove(minId, maxId - minId + 1);
    }

    void prepend(const BrushPolyline& src) {
        const qreal srcMaxT = src.maxT();
        const qreal srcTShift = minT() - srcMaxT - 1;
        QVector<PolylinePt<3>> newPts;
        for(const auto& pt : src) {
            PolylinePt<3> newPt = pt;
            reinterpret_cast<BrushPt&>(newPt).fT += srcTShift;
            newPts.append(newPt);
        }
        mPts.swap(newPts);
        mPts.append(newPts);
    }

    void append(const BrushPolyline& src) {
        const qreal srcMinT = src.minT();
        const qreal srcTShift = maxT() - srcMinT + 1;
        for(const auto& pt : src) {
            PolylinePt<3> newPt = pt;
            reinterpret_cast<BrushPt&>(newPt).fT += srcTShift;
            mPts.append(newPt);
        }
    }

    void setTRange(const qValueRange& newRange) {
        if(mPts.isEmpty()) return;
        if(mPts.count() == 1) {
            reinterpret_cast<BrushPt&>(mPts.first()).fT = newRange.fMin;
            return;
        }
        const auto oldRange = tRange();
        const qreal oldSpan = oldRange.fMax - oldRange.fMin;
        const qreal newSpan = newRange.fMax - newRange.fMin;
        for(auto& pt : mPts) {
            qreal& ptT = reinterpret_cast<BrushPt&>(pt).fT;
            const qreal frac = (ptT - oldRange.fMin)/oldSpan;
            ptT = frac*newSpan + newRange.fMin;
        }
    }

    int countBeforeT(const qreal& t) const {
        return idBeforeT(t) + 1;
    }

    int countAfterT(const qreal& t) const {
        return count() - countBeforeT(t);
    }

    qValueRange tRange() const {
        return {minT(), maxT()};
    }

    qreal maxT() const {
        if(mPts.isEmpty()) return 0;
        return reinterpret_cast<const BrushPt&>(mPts.last()).fT;
    }

    qreal minT() const {
        if(mPts.isEmpty()) return 0;
        return reinterpret_cast<const BrushPt&>(mPts.first()).fT;
    }
    QVector<PolylinePt<3>>::const_iterator begin() const { return mPts.begin(); }
    QVector<PolylinePt<3>>::const_iterator end() const { return mPts.end(); }

    QVector<PolylinePt<3>>::iterator begin() { return mPts.begin(); }
    QVector<PolylinePt<3>>::iterator end() { return mPts.end(); }

    qreal tAtId(const int& id) const {
        return reinterpret_cast<const BrushPt&>(mPts.at(id)).fT;
    }

    int idAfterT(const qreal& t, const int& minId, const int& maxId) const {
        if(maxId < minId) return -1;
        const int guessId = (maxId + minId)/2;
        const auto& guessPt = reinterpret_cast<const BrushPt&>(mPts[guessId]);
        if(maxId == minId) {
            if(guessPt.fT > t) return maxId;
            else return -1;
        }
        if(guessPt.fT > t) return idAfterT(t, minId, guessId);
        else return idAfterT(t, guessId + 1, maxId);
    }

    int idBeforeT(const qreal& t, const int& minId, const int& maxId) const {
        if(maxId < minId) return -1;
        const int guessId = (maxId + minId)/2;
        const auto& guessPt = reinterpret_cast<const BrushPt&>(mPts[guessId]);
        if(maxId == minId) {
            if(guessPt.fT < t) return maxId;
            else return -1;
        }
        if(guessPt.fT > t) return idBeforeT(t, minId, guessId - 1);
        else return idBeforeT(t, guessId, maxId);
    }
};
#endif // BRUSHPOLYLINE_H
