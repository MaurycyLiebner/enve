#ifndef SMARTPATHCONTAINER_H
#define SMARTPATHCONTAINER_H
#include "simplemath.h"
#include "skia/skiaincludes.h"
#define VALUES_PER_VERB 7

class NodesHandler {
public:
    ~NodesHandler() {
        if(mValues) free(mValues);
    }
    enum VerbType { NORMAL, SHADOW, MOVE, NONE };

    struct TRange {
        SkScalar fMin;
        SkScalar fMax;

        SkScalar middle() const {
            return (fMin + fMax)*0.5f;
        }
    };

    struct NodeValues {
        SkPoint fC0;
        SkPoint fP1;
        SkPoint fC2;
    };

    struct ShadowNodeValues {
        SkPoint fPrevC2;
        SkPoint fC0;
        SkPoint fP1;
        SkPoint fC2;
        SkPoint fNextC0;
    };

    struct NormalPathNode {
        NormalPathNode(SkPoint * const src) :
            fC0(src), fP1(src + 1), fC2(src + 2),
            fDuplCount(reinterpret_cast<SkScalar*>(src + 3)) {}

        const SkPoint& c0() const {
            return *fC0;
        }

        const SkPoint& p1() const {
            return *fP1;
        }

        const SkPoint& c2() const {
            return *fC2;
        }

        void setC0(const SkScalar& c0x, const SkScalar& c0y) const {
            setC0(SkPoint::Make(c0x, c0y));
        }

        void setP1(const SkScalar& p1x, const SkScalar& p1y) const {
            setP1(SkPoint::Make(p1x, p1y));
        }

        void setC2(const SkScalar& c2x, const SkScalar& c2y) const {
            setC2(SkPoint::Make(c2x, c2y));
        }

        void setC0(const SkPoint& c0V) const {
            *fC0 = c0V;
        }

        void setP1(const SkPoint& p1V) const {
            *fP1 = p1V;
        }

        void setC2(const SkPoint& c2V) const {
            *fC2 = c2V;
        }

        int duplCount() const {
            return qRound(*fDuplCount);
        }

        int totalNodesCount() const {
            return duplCount() + 1;
        }

        void setDuplCount(const int& count) {
            *fDuplCount = count;
        }

        NodeValues toNodeValues() const {
            return {c0(), p1(), c2()};
        }
//        QList<NodeValues> toNodeValues() const {
//            QList<NodeValues> result;

//            const int iMax = duplCount();
//            for(int i = 0; i <= iMax; i++) {
//                NodeValues values;
//                values.fC0 = (i == 0 ? c0() : p1());
//                values.fP1 = p1();
//                values.fC2 = (i == iMax ? c2() : p1());
//            }

//            return result;
//        }

        SkPoint* fC0;
        SkPoint* fP1;
        SkPoint* fC2;
        SkScalar* fDuplCount;
    };

    struct ShadowPathNode {
        ShadowPathNode(const NormalPathNode& prevNormal,
                       SkScalar * const src,
                       const NormalPathNode& nextNormal,
                       const bool& previousIsShadow) :
            fPrevNormal(prevNormal),
            fT(src), fDuplCount(src + 6),
            fNextNormal(nextNormal) {
            if(!previousIsShadow) {
                fPrevNodeT = 0;
                fPrevNodeP1 = prevNormal.p1();
                fPrevNodeC2 = prevNormal.c2();
                return;
            }
            const auto prevSrc = src - VALUES_PER_VERB;
            fPrevNodeT = *prevSrc;
            auto prevSNode = ShadowPathNode(prevNormal, prevSrc,
                                            nextNormal, false);
            auto prevNodeVals = prevSNode.toNodeValues();
            fPrevNodeP1 = prevNodeVals.fP1;
            fPrevNodeC2 = prevNodeVals.fC2;
        }

        const SkScalar& t() const {
            return *fT;
        }

        void setT(const SkScalar& tV) const {
            *fT = tV;
        }

        ShadowNodeValues toNodeValues() const {
            qreal qt = CLAMP(static_cast<qreal>(t()), 0, 1);

            QPointF p0 = skPointToQ(fPrevNodeP1);
            QPointF c1 = skPointToQ(fPrevNodeC2);
            QPointF c2 = skPointToQ(fNextNormal.c0());
            QPointF p1 = skPointToQ(fNextNormal.p1());

            qreal oneMinusT = 1 - qt;
            QPointF P0_1 = p0*oneMinusT + c1*qt;
            QPointF P1_2 = c1*oneMinusT + c2*qt;
            QPointF P2_3 = c2*oneMinusT + p1*qt;

            QPointF P01_12 = P0_1*oneMinusT + P1_2*qt;
            QPointF P12_23 = P1_2*oneMinusT + P2_3*qt;

            QPointF P0112_1223 = P01_12*oneMinusT + P12_23*qt;

            //qCubicSegment2D seg1(p0, P0_1, P01_12, P0112_1223);
            //qCubicSegment2D seg2(P0112_1223, P12_23, P2_3, p1);

            return {qPointToSk(P0_1), qPointToSk(P01_12),
                    qPointToSk(P0112_1223), qPointToSk(P12_23),
                    qPointToSk(P2_3)};
        }


        int duplCount() const {
            return qRound(*fDuplCount);
        }

        int totalNodesCount() const {
            return duplCount() + 1;
        }

        void setDuplCount(const int& count) {
            *fDuplCount = count;
        }

        NormalPathNode fPrevNormal;
        SkPoint fPrevNodeP1;
        SkPoint fPrevNodeC2;
        SkScalar fPrevNodeT;
        SkScalar* fT;
        SkScalar* fDuplCount;
        NormalPathNode fNextNormal;
    };

    struct NormalSegment {
        NormalSegment(const NormalPathNode& prev,
                      const NormalPathNode& next) :
            fPrevNormal(prev), fNextNormal(next) {}

        void cubicTo(SkPath &path,
                     const bool& skipPrevDuplicates,
                     const bool& skipNextDuplicates) {
            if(!skipPrevDuplicates) {
                for(int i = 0; i < fPrevNormal.duplCount(); i++) {
                    path.cubicTo(fPrevNormal.p1(), fPrevNormal.p1(),
                                 fPrevNormal.p1());
                }
            }
            path.cubicTo(fPrevNormal.c2(), fNextNormal.c0(),
                         fNextNormal.p1());
            if(!skipNextDuplicates) {
                for(int i = 0; i < fNextNormal.duplCount(); i++) {
                    path.cubicTo(fNextNormal.p1(), fNextNormal.p1(),
                                 fNextNormal.p1());
                }
            }
        }

        NormalPathNode fPrevNormal;
        NormalPathNode fNextNormal;
    };

    struct ShadowSegment {
        ShadowSegment(const NormalPathNode& prev,
                      const QList<ShadowPathNode>& middle,
                      const NormalPathNode& next) :
            fPrevNormal(prev), fShadowNodes(middle), fNextNormal(next) {}

        void cubicTo(SkPath &path,
                     const bool& skipPrevDuplicates,
                     const bool& skipNextDuplicates) {
            if(!skipPrevDuplicates) {
                for(int i = 0; i < fPrevNormal.duplCount(); i++) {
                    path.cubicTo(fPrevNormal.p1(), fPrevNormal.p1(),
                                 fPrevNormal.p1());
                }
            }
            ShadowNodeValues lastPtValues =
            { fPrevNormal.c0(), fPrevNormal.c0(),
              fPrevNormal.p1(), fPrevNormal.c2(),
              fNextNormal.c0()};
            const int iMax = fShadowNodes.count() - 1;
            for(int i = 0; i <= iMax; i++) {
                auto& sNode = fShadowNodes.at(i);
                auto ptValues = sNode.toNodeValues();
                path.cubicTo(ptValues.fPrevC2, ptValues.fC0, ptValues.fP1);
                for(int j = 0; j < sNode.duplCount(); j++) {
                    path.cubicTo(ptValues.fP1, ptValues.fP1, ptValues.fP1);
                }
                if(i == iMax) lastPtValues = ptValues;
            }
            path.cubicTo(lastPtValues.fC2, lastPtValues.fNextC0, fNextNormal.p1());
            if(!skipNextDuplicates) {
                for(int i = 0; i < fNextNormal.duplCount(); i++) {
                    path.cubicTo(fNextNormal.p1(), fNextNormal.p1(),
                                 fNextNormal.p1());
                }
            }
        }

        NormalPathNode fPrevNormal;
        QList<ShadowPathNode> fShadowNodes;
        NormalPathNode fNextNormal;
    };

    void replaceWithNormalNode(const int& verbId,
                               const SkPoint& c0,
                               const SkPoint& p1,
                               const SkPoint& c2,
                               const int& nDupl = 0) {
        mVerbTypes.replace(verbId, NORMAL);
        setNormalNodeValues(verbId, c0, p1, c2, nDupl);
    }

    void replaceWithShadowNode(const int& verbId,
                               const SkScalar& t,
                               const int& nDupl = 0) {
        mVerbTypes.replace(verbId, SHADOW);
        setShadowNodeValues(verbId, t, nDupl);
    }

    void replaceWithMoveVerb(const int& verbId) {
        mVerbTypes.replace(verbId, MOVE);
    }

    void insertNormalNode(const int& verbId,
                          const SkPoint& c0,
                          const SkPoint& p1,
                          const SkPoint& c2,
                          const int& nDupl = 0) {
        makeSpaceForNew(verbId, NORMAL);
        setNormalNodeValues(verbId, c0, p1, c2, nDupl);
    }

    void prependNormalNode(const SkPoint& c0,
                           const SkPoint& p1,
                           const SkPoint& c2,
                           const int& nDupl = 0) {
        insertNormalNode(0, c0, p1, c2, nDupl);
    }

    void appendNormalNode(const SkPoint& c0,
                          const SkPoint& p1,
                          const SkPoint& c2,
                          const int& nDupl = 0) {
        insertNormalNode(mNVerbs, c0, p1, c2, nDupl);
    }

    void insertShadowNode(const int& verbId, const SkScalar& t,
                          const int& nDupl = 0) {
        makeSpaceForNew(verbId, SHADOW);
        setShadowNodeValues(verbId, t, nDupl);
    }

    void insertShadowNodeInTheMiddle(const int& verbId,
                                     const int& nDupl = 0) {
        makeSpaceForNew(verbId, SHADOW);
        setShadowNodeValues(verbId, getShadowNodeTRange(verbId).middle(),
                           nDupl);
    }

    void convertShadowNodeToNormalNode(const int& verbId) {
        auto vals = getAsShadowNode(verbId).toNodeValues();
        mVerbTypes.replace(verbId, NORMAL);
        setNormalNodeValues(verbId, vals.fC0, vals.fP1, vals.fC2);
    }

    void setNumberOfDuplicates(const int& verbId, const int& nDupl) {
        *(getValuesAsSkScalar(verbId) + 6) = nDupl;
    }

    void appendShadowNode(const SkScalar& t,
                          const int& nDupl = 0) {
        insertShadowNode(mNVerbs, t, nDupl);
    }

    void prependShadowNode(const SkScalar& t,
                           const int& nDupl = 0) {
        insertShadowNode(0, t, nDupl);
    }

    void removeVerb(const int& verbId) {
        if(verbId < 0) return;
        if(verbId >= mNVerbs) return;
        mVerbTypes.removeAt(verbId);
        const int newNValues = mNValues - VALUES_PER_VERB;
        const size_t newDataSize = static_cast<size_t>(newNValues)*sizeof(SkScalar);
        void * const newData = malloc(newDataSize);
        SkScalar * const newValues = static_cast<SkScalar*>(newData);
        if(mNValues == 0) {
            const int firstValId = verbId*VALUES_PER_VERB;
            const size_t cpy1Size = static_cast<size_t>(firstValId)*
                    sizeof(SkScalar);
            if(firstValId) memcpy(newValues, mValues, cpy1Size);
            if(firstValId != mNValues - VALUES_PER_VERB) {
                const size_t cpy2Size = static_cast<size_t>(
                            mNValues - firstValId - VALUES_PER_VERB)*sizeof(SkScalar);
                memcpy(newValues + firstValId,
                       mValues + firstValId + VALUES_PER_VERB,
                       cpy2Size);
            }
        }
        free(mValues);
        mValues = newValues;
        mNValues = newNValues;
        mNVerbs--;
    }

    int prevId(const int& beforeId, const VerbType& type) const {
        for(int i = beforeId - 1; i >= 0; i--) {
            if(getType(i) == type) return i;
        }
        return -1;
    }

    int nextId(const int& afterId, const VerbType& type) const {
        for(int i = afterId + 1; i < mNVerbs; i++) {
            if(getType(i) == type) return i;
        }
        return -1;
    }

    const VerbType& getType(const int& id) const {
        return mVerbTypes.at(static_cast<int>(id));
    }

    const int& getVerbCount() const {
        return mNVerbs;
    }

    ShadowPathNode getAsShadowNode(const int& verbId) {
        const int prevNormalId = prevId(verbId, NORMAL);
        const int nextNormalId = nextId(verbId, NORMAL);
        return ShadowPathNode(getValuesAsSkPoint(prevNormalId),
                              getValuesAsSkScalar(verbId),
                              getValuesAsSkPoint(nextNormalId),
                              prevNormalId < (verbId - 1));
    }

    NormalPathNode getAsNormalNode(const int& verbId) {
        return NormalPathNode(getValuesAsSkPoint(verbId));
    }

    TRange getShadowNodeTRange(const int& verbId) const {
        if(verbId == 0) return {1, 1};
        if(verbId == mNVerbs - 1) return {0, 0};
        SkScalar min;
        SkScalar max;
        auto prevType = getType(verbId - 1);
        if(prevType == SHADOW) {
            min = *getValuesAsSkScalar(verbId - 1);
        } else if(prevType == MOVE) {
            return {1, 1};
        } else {
            min = 0;
        }

        auto nextType = getType(verbId + 1);
        if(nextType == SHADOW) {
            max = *getValuesAsSkScalar(verbId + 1);
        } else if(nextType == MOVE) {
            return {0, 0};
        } else {
            max = 1;
        }
        return {min, max};
    }

    struct NodeIterator {
        NodeIterator(const int& startI,
                     NodesHandler& targetT,
                     const bool& skipShadow) :
            i(startI - 1), mTarget(targetT), mSkipShadow(skipShadow) {
            if(mSkipShadow) {
                if(currentType() == SHADOW) {
                    next();
                }
            }
        }

        NodeIterator(NodesHandler& targetT,
                     const bool& skipShadow) :
            NodeIterator(-1, targetT, skipShadow) {

        }

        const VerbType& currentType() const {
            return mTarget.getType(i);
        }

        ShadowPathNode getCurrentAsShadowNode() {
            return mTarget.getAsShadowNode(i);
        }

        NormalPathNode getCurrentAsNormalNode() {
            return mTarget.getAsNormalNode(i);
        }

        bool hasNext() const {
            return i < mTarget.getVerbCount() - 1;
        }

        bool next() {
            if(hasNext()) {
                i++;
                if(mSkipShadow) {
                    if(currentType() == SHADOW) {
                        return next();
                    }
                }
                return true;
            }
            return false;
        }

        NodeIterator peekNext() {
            return NodeIterator(i + 1, mTarget, mSkipShadow);
        }
    private:
        int i;
        NodesHandler& mTarget;
        bool mSkipShadow;
    };

    struct SegmentIterator {
        SegmentIterator(const int& startI, NodesHandler& targetT,
                        const bool& skipShadow) :
           mNode1Iterator(startI - 1, targetT, true),
           mNode2Iterator(startI, targetT, skipShadow) {
            updateCurrentType();
        }

        SegmentIterator(NodesHandler& targetT,
                        const bool& skipShadow) :
            SegmentIterator(-1, targetT, skipShadow) {}


        ShadowSegment getCurrentAsShadowSegment() {
            return ShadowSegment(mNode1Iterator.getCurrentAsNormalNode(),
                                 mCurrentShadowNodes,
                                 mNode2Iterator.getCurrentAsNormalNode());
        }

        NormalSegment getCurrentAsNormalSegment() {
            return NormalSegment(mNode1Iterator.getCurrentAsNormalNode(),
                                 mNode2Iterator.getCurrentAsNormalNode());
        }

        VerbType currentType() const {
            return mCurrentType;
        }

        bool hasNext() const {
            return mNode2Iterator.hasNext();
        }

        bool next() {
            if(hasNext()) {
                mNode1Iterator.next();
                mNode2Iterator.next();
                updateCurrentType();
                return true;
            }
            return false;
        }
    private:
        void updateCurrentType() {
            mCurrentShadowNodes.clear();
            mCurrentType = mNode2Iterator.currentType();
            if(mCurrentType == MOVE) {
                mNode1Iterator.next();
                mNode2Iterator.next();
            }
            while(mNode2Iterator.currentType() == SHADOW) {
                mCurrentShadowNodes << mNode2Iterator.getCurrentAsShadowNode();
                mNode2Iterator.next();
            }
        }

        VerbType mCurrentType = NONE;
        QList<ShadowPathNode> mCurrentShadowNodes;
        NodeIterator mNode1Iterator;
        NodeIterator mNode2Iterator;
    };

    NodesHandler interpolate(const NodesHandler& src1,
                             const NodesHandler& src2,
                             const SkScalar& src2Weight) {
        if(isZero6Dec(src2Weight)/* || src2Weight < 0*/) return src1;
        if(isOne6Dec(src2Weight)/* || src2Weight > 1*/) return src2;
        const SkScalar src1Weigth = 1 - src2Weight;
    }

    SkPath toSkPath(const bool& skipShadow) {
        SkPath path;
        SegmentIterator iterator(*this, skipShadow);
        bool move = true;
        while(iterator.next()) {
            if(iterator.currentType() == NORMAL) {
                auto seg = iterator.getCurrentAsNormalSegment();
                const bool moved = move;
                if(move) {
                    move = false;
                    path.moveTo(seg.fPrevNormal.p1());
                    continue;
                }
                seg.cubicTo(path, skipShadow || !moved, false);
            } else if(iterator.currentType() == SHADOW) {
                auto seg = iterator.getCurrentAsShadowSegment();
                const bool moved = move;
                if(move) {
                    move = false;
                    path.moveTo(seg.fPrevNormal.p1());
                    continue;
                }
                seg.cubicTo(path, skipShadow || !moved, false);
            } else if(iterator.currentType() == MOVE) {
                move = true;
            } else {
                Q_ASSERT(false);
            }
        }
        return path;
    }
private:
//    void sortShadowNodesStartingAt(const int& firstId) {
//        int i = firstId;
//        QList<SkScalar> ts;
//        while(getType(i) == SHADOW) {
//            auto node = getAsShadowNode(i++);
//            ts << node.t();
//        }
//        std::sort(ts.begin(), ts.end());
//        i = firstId;
//        while(getType(i) == SHADOW) {
//            setShadowNodeValue(i, ts.at(i));
//        }
//    }

    SkPoint * getValuesAsSkPoint(const int& verbId) const {
        return reinterpret_cast<SkPoint*>(mValues) + verbId;
    }

    SkScalar * getValuesAsSkScalar(const int& verbId) const {
        return mValues + verbId*VALUES_PER_VERB;
    }

    void makeSpaceForNew(int newId, const VerbType& nodeType) {
        newId = qMax(0, qMin(newId, mNVerbs));
        mVerbTypes.insert(static_cast<int>(newId), nodeType);
        const int newNValues = mNValues + VALUES_PER_VERB;
        const size_t newDataSize = static_cast<size_t>(newNValues)*sizeof(SkScalar);
        void * const newData = malloc(newDataSize);
        SkScalar * const newValues = static_cast<SkScalar*>(newData);
        if(mNValues == 0) {
            const int firstValId = newId*VALUES_PER_VERB;
            const size_t cpy1Size = static_cast<size_t>(firstValId)*
                    sizeof(SkScalar);
            if(firstValId) memcpy(newValues, mValues, cpy1Size);
            if(firstValId != mNValues) {
                const size_t cpy2Size = static_cast<size_t>(
                            mNValues - firstValId)*sizeof(SkScalar);
                memcpy(newValues + firstValId + VALUES_PER_VERB,
                       mValues + firstValId,
                       cpy2Size);
            }
        }
        free(mValues);
        mValues = newValues;
        mNValues = newNValues;
        mNVerbs++;
    }

    void setNormalNodeValues(const int& verbId,
                             const SkPoint& c0,
                             const SkPoint& p1,
                             const SkPoint& c2,
                             const int& nDupl = 0) {
        SkPoint * dst = getValuesAsSkPoint(verbId);
        *(dst++) = c0;
        *(dst++) = p1;
        *(dst++) = c2;
        *reinterpret_cast<SkScalar*>(dst) = nDupl;
    }

    void setShadowNodeValues(const int& verbId,
                             const SkScalar& t,
                             const int& nDupl = 0) {
        SkScalar * const dst = getValuesAsSkScalar(verbId);
        *dst = t;
        *(dst + 6) = nDupl;
    }

    QList<VerbType> mVerbTypes;
    SkScalar* mValues = nullptr;
    int mNValues = 0;
    int mNVerbs = 0;
};

#endif // SMARTPATHCONTAINER_H
