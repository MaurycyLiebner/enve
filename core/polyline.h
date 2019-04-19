#ifndef POLYLINE_H
#define POLYLINE_H
#include "pointhelpers.h"
#include "exceptions.h"
#include "basicreadwrite.h"

template <uchar PROPS, uchar VALS = 2 + PROPS>
struct PolylinePt {
    qreal fVals[VALS];

    PolylinePt<PROPS> operator+(const PolylinePt<PROPS>& other) const {
        auto newP = *this;
        for(int i = 0; i < VALS; i++) newP.fVals[i] += other.fVals[i];
        return newP;
    }

    PolylinePt<PROPS> operator*(const qreal& val) const {
        auto newP = *this;
        for(int i = 0; i < VALS; i++) newP.fVals[i] *= val;
        return newP;
    }
};

class Gaussian {
public:
    Gaussian(const qreal& a, const qreal& c) {
        mA = a;
        m2Csqr = 2*c*c;
    }

    static Gaussian sFromRadius(const qreal& a, const qreal& radius) {
        const qreal c = 0.465991*radius; // tenth of height at radius
        return Gaussian(a, c);
    }

    qreal eval(const qreal& x) const {
        return mA*exp(-x*x/m2Csqr);
    }

    qreal xForValue(const qreal& val) const {
        const qreal expArg = log(val/mA);
        return sqrt(-expArg*m2Csqr);
    }
private:
    qreal mA;
    qreal m2Csqr;
};

template <uchar PROPS>
class Polyline {
public:
    Polyline() {}
    Polyline(const QVector<PolylinePt<PROPS>>& src) : mPts(src) {}
    int count() const {
        return mPts.count();
    }

    qreal length() const {
        return length(0, mPts.count() - 1);
    }

    qreal length(int minId, int maxId) const {
        if(mPts.isEmpty()) return 0;
        if(maxId == -1) maxId = mPts.count() - 1;
        minId = clamp(minId, 0, mPts.count() - 1);
        maxId = clamp(maxId, 0, mPts.count() - 1);
        if(minId > maxId) {
            if(mClosed) {
                return length(minId, mPts.count() - 1) + length(0, maxId);
            } else return 0;
        }
        qreal len = 0;
        const auto begin = mPts.begin() + minId;
        const auto end = mPts.begin() + maxId + 1;

        QPointF lastPos = reinterpret_cast<const QPointF&>(begin);
        for(auto it = begin + 1; it != end; ++it) {
            const QPointF ptPos = reinterpret_cast<QPointF&>(it);
            len += pointToLen(lastPos - ptPos);
        }
        return len;
    }

    int idBeforeLength(const qreal& len) const {
        qreal currLen = 0;
        QPointF lastPos = reinterpret_cast<const QPointF&>(*mPts.begin());
        for(auto it = mPts.begin() + 1; it != mPts.end(); ++it) {
            const QPointF ptPos = reinterpret_cast<QPointF&>(it);
            currLen += pointToLen(lastPos - ptPos);
            if(currLen > len) return static_cast<int>(it - mPts.begin()) - 1;
        }
        return mPts.count() - 1;
    }

    void smoothyProperty(const int& propId, const QPointF& pos,
                         const Gaussian& weightGauss, const int& windowSize) {
        const qreal distCutoff = weightGauss.xForValue(0.00001);
        const int valId = 2 + propId;
        int i = -1;
        const int maxI = mPts.count() - 1;
        for(auto it = mPts.begin(); it != mPts.end(); ++it) {
            i++;
            const qreal dist = pointToLen(pos - reinterpret_cast<QPointF&>(it));
            if(dist > distCutoff) continue;
            const int lWinSize = qMin(i, windowSize);
            const int rWinSize = qMin(i, maxI - i);
            const qreal prop = it.fVals[valId];
            const qreal weight = CLAMP01(weightGauss.eval(dist));
            qreal targetVal = prop;
            auto lIt = it;
            for(int l = 0; l < lWinSize; l++) {
                lIt++;
                targetVal += lIt.fVals[valId];
            }
            auto rIt = it;
            for(int r = 0; r < rWinSize; r++) {
                rIt++;
                targetVal += rIt.fVals[valId];
            }
            targetVal /= lWinSize + 1 + rWinSize;
            it.fVals[valId] = prop*(1 - weight) + targetVal*weight;
        }
    }

    void smoothyPos(const int& propId, const QPointF& pos,
                    const Gaussian& weightGauss, const int& windowSize) {
        const qreal distCutoff = weightGauss.xForValue(0.00001);
        const int valId = 2 + propId;
        int i = -1;
        const int maxI = mPts.count() - 1;
        for(auto it = mPts.begin(); it != mPts.end(); ++it) {
            i++;
            const auto& itPos = reinterpret_cast<QPointF&>(it);
            const qreal dist = pointToLen(pos - itPos);
            if(dist > distCutoff) continue;
            const int lWinSize = qMin(i, windowSize);
            const int rWinSize = qMin(i, maxI - i);
            const qreal weight = CLAMP01(weightGauss.eval(dist));
            QPointF targetPos = itPos;
            auto lIt = it;
            for(int l = 0; l < lWinSize; l++) {
                lIt++;
                targetPos += reinterpret_cast<QPointF&>(lIt);
            }
            auto rIt = it;
            for(int r = 0; r < rWinSize; r++) {
                rIt++;
                targetPos += reinterpret_cast<QPointF&>(rIt);
            }
            targetPos /= lWinSize + 1 + rWinSize;
            it.fVals[valId] = itPos*(1 - weight) + targetPos*weight;
        }
    }

    void changePropBy(const int& propId, const QPointF& pos,
                      const qreal& changeBy, const Gaussian& weightGauss) {
        const int valId = 2 + propId;
        const qreal distCutoff = weightGauss.xForValue(0.00001);
        for(auto& pt : mPts) {
            const QPointF& ptPos = reinterpret_cast<QPointF&>(pt);
            const qreal dist = pointToLen(pos - ptPos);
            if(dist > distCutoff) continue;
            const qreal weight = CLAMP01(weightGauss.eval(dist));
            auto& val = pt.fVals[valId];
            val = CLAMP01(val + changeBy*weight);
        }
    }

    void changePropTo(const int& propId, const QPointF& pos,
                      const qreal& changeTo, const Gaussian& weightGauss) {
        const int valId = 2 + propId;
        const qreal distCutoff = weightGauss.xForValue(0.00001);
        for(auto& pt : mPts) {
            const QPointF& ptPos = reinterpret_cast<QPointF&>(pt);
            const qreal dist = pointToLen(pos - ptPos);
            if(dist > distCutoff) continue;
            const qreal weight = CLAMP01(weightGauss.eval(dist));
            auto& val = pt.fVals[valId];
            val = CLAMP01(val*(1 - weight) + changeTo*weight);
        }
    }

    void changePos(const QPointF& pos, const QPointF& changeBy,
                   const Gaussian& weightGauss) {
        const qreal distCutoff = weightGauss.xForValue(0.00001);
        for(auto& pt : mPts) {
            const QPointF& ptPos = reinterpret_cast<QPointF&>(pt);
            const qreal dist = pointToLen(pos - ptPos);
            if(dist > distCutoff) continue;
            const qreal weight = CLAMP01(weightGauss.eval(dist));
            reinterpret_cast<QPointF&>(pt) += changeBy*weight;
        }
    }

    void save() {
        mSavedPts.clear();
        mSavedPts.append(mPts);
    }

    void restore() {
        if(mSavedPts.count() == mPts.count()) {
            auto sIt = mSavedPts.begin();
            for(auto& pt : mPts) pt = *(sIt++);
        } else {
            mPts.swap(mSavedPts);
        }
        mSavedPts.clear();
    }

    bool isClosed() const { return mClosed; }

    auto begin() { return mPts.begin(); }
    auto end() { return mPts.end(); }

    auto begin() const { return mPts.begin(); }
    auto end() const { return mPts.end(); }

    bool read(QIODevice * src) {
        int size;
        src->read(rcChar(&size), sizeof(int));
        mPts.clear();
        mPts.reserve(size);
        for(int i = 0; i < size; i++) {
            PolylinePt<PROPS> pt;
            src->read(rcChar(&pt), sizeof(PolylinePt<PROPS>));
            mPts.append(pt);
        }
        src->read(rcChar(&mClosed), sizeof(bool));
        return true;
    }

    bool write(QIODevice * dst) const {
        const int size = mPts.count();
        dst->write(rcConstChar(&size), sizeof(int));
        for(int i = 0; i < size; i++) {
            PolylinePt<PROPS> node;
            dst->write(rcConstChar(&mPts.at(i)), sizeof(PolylinePt<PROPS>));
        }
        dst->write(rcConstChar(&mClosed), sizeof(bool));
        return true;
    }
protected:
    QVector<PolylinePt<PROPS>> mPts;
private:
    QVector<PolylinePt<PROPS>> mSavedPts;
    bool mClosed = false;
};


#endif // POLYLINE_H
