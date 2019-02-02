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
    enum VerbType { NORMAL, DUPLICATE, SHADOW, MOVE, NONE };

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

    struct NormalPathNode {
        NormalPathNode(SkPoint * const src) :
            fC0(src), fP1(src + 1), fC2(src + 2) {}

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

        SkPoint* fC0;
        SkPoint* fP1;
        SkPoint* fC2;
    };

    struct DuplicatePathNode : public NormalPathNode {
        DuplicatePathNode(SkPoint * const src) :
            NormalPathNode(src) {
            const auto srcS = reinterpret_cast<SkScalar*>(src) + 6;
            fNDuplicates = srcS;
        }

        void setDuplCount(const int& count) const {
            *fNDuplicates = count;
        }

        int duplCount() const {
            return qRound(*fNDuplicates);
        }

        int totalNodesCount() const {
            return duplCount() + 1;
        }

        QList<NodeValues> toNodeValues() const {
            QList<NodeValues> result;

            const int iMax = duplCount();
            for(int i = 0; i <= iMax; i++) {
                NodeValues values;
                values.fC0 = (i == 0 ? c0() : p1());
                values.fP1 = p1();
                values.fC2 = (i == iMax ? c2() : p1());
            }

            return result;
        }

        SkScalar *fNDuplicates;
    };

    struct ShadowPathNode {
        ShadowPathNode(const NormalPathNode& prev,
                       SkScalar * const src,
                       const NormalPathNode& next) :
            fPrevNormal(prev), fT(src), fNextNormal(next) {}

        const SkScalar& t() const {
            return *fT;
        }

        void setT(const SkScalar& tV) const {
            *fT = tV;
        }

        NodeValues toNodeValues() const {
            qreal qt = CLAMP(static_cast<qreal>(t()), 0, 1);
            if(isZero6Dec(qt)) {
                SkPoint val = fPrevNormal.p1();
                return {val, val, val};
            }
            if(isZero6Dec(qt - 1)) {
                SkPoint val = fNextNormal.p1();
                return {val, val, val};
            }
            QPointF p0 = skPointToQ(fPrevNormal.p1());
            QPointF c1 = skPointToQ(fPrevNormal.c2());
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

            return {qPointToSk(P01_12), qPointToSk(P0112_1223),
                        qPointToSk(P12_23)};
        }

        NormalPathNode fPrevNormal;
        SkScalar* fT;
        NormalPathNode fNextNormal;
    };

    struct NormalSegment {
        NormalSegment(const NormalPathNode& prev,
                      const NormalPathNode& next) :
            fPrevNormal(prev), fNextNormal(next) {}

        void cubicTo(SkPath &path) {
            path.cubicTo(fPrevNormal.c2(), fNextNormal.c0(), fNextNormal.p1());
        }

        NormalPathNode fPrevNormal;
        NormalPathNode fNextNormal;
    };

    struct DuplicateSegment {
        DuplicateSegment(const DuplicatePathNode& prev,
                         const DuplicatePathNode& next) :
            fPrevDuplicate(prev), fNextDuplicate(next) {}

        void cubicTo(SkPath &path) {
            for(int i = 0; i < fPrevDuplicate.duplCount(); i++) {
                path.cubicTo(fPrevDuplicate.p1(), fPrevDuplicate.p1(),
                             fPrevDuplicate.p1());
            }
            path.cubicTo(fPrevDuplicate.c2(), fNextDuplicate.c0(),
                         fNextDuplicate.p1());
            for(int i = 0; i < fNextDuplicate.duplCount(); i++) {
                path.cubicTo(fNextDuplicate.p1(), fNextDuplicate.p1(),
                             fNextDuplicate.p1());
            }
        }

        DuplicatePathNode fPrevDuplicate;
        DuplicatePathNode fNextDuplicate;
    };

    // should also handle multiple shadow nodes
    struct ShadowSegment {
        ShadowSegment(const NormalPathNode& prev,
                      const QList<ShadowPathNode>& middle,
                      const NormalPathNode& next) :
            fPrevNormal(prev), fShadowNodes(middle), fNextNormal(next) {}

        void cubicTo(SkPath &path, const bool& skipShadowNodes = false) {
            if(fShadowNodes.isEmpty() || skipShadowNodes) {
                path.cubicTo(fPrevNormal.c2(), fNextNormal.c0(), fNextNormal.p1());
                return;
            }
            auto lastPtValues = fShadowNodes.first().toNodeValues();
            path.cubicTo(fPrevNormal.c2(), lastPtValues.fC0, lastPtValues.fP1);
            for(int i = 1; i < fShadowNodes.count(); i++) {
                auto ptValues = fShadowNodes.first().toNodeValues();
                path.cubicTo(lastPtValues.fC2, ptValues.fC0, ptValues.fP1);
                lastPtValues = ptValues;
            }
            path.cubicTo(lastPtValues.fC2, fNextNormal.c0(), fNextNormal.p1());
        }

        NormalPathNode fPrevNormal;
        QList<ShadowPathNode> fShadowNodes;
        NormalPathNode fNextNormal;
    };

    void replaceWithNormalNode(const int& verbId,
                               const SkPoint& c0,
                               const SkPoint& p1,
                               const SkPoint& c2) {
        mVerbTypes.replace(verbId, NORMAL);
        setNormalNodeValues(verbId, c0, p1, c2);
    }

    void replaceWithDuplicateNode(const int& verbId,
                                  const SkPoint& c0,
                                  const SkPoint& p1,
                                  const SkPoint& c2,
                                  const int& nDupli) {
        mVerbTypes.replace(verbId, DUPLICATE);
        setDuplicateNodeValues(verbId, c0, p1, c2, nDupli);
    }

    void replaceWithShadowNode(const int& verbId,
                               const SkScalar& t) {
        mVerbTypes.replace(verbId, SHADOW);
        setShadowNodeValue(verbId, t);
    }

    void replaceWithMoveVerb(const int& verbId,
                             const SkPoint& moveTo) {
        mVerbTypes.replace(verbId, MOVE);
        setMoveVerbValue(verbId, moveTo);
    }

    void insertNormalNode(const int& verbId,
                          const SkPoint& c0,
                          const SkPoint& p1,
                          const SkPoint& c2) {
        makeSpaceForNew(verbId, NORMAL);
        setNormalNodeValues(verbId, c0, p1, c2);
    }

    void prependNormalNode(const SkPoint& c0,
                           const SkPoint& p1,
                           const SkPoint& c2) {
        insertNormalNode(0, c0, p1, c2);
    }

    void appendNormalNode(const SkPoint& c0,
                          const SkPoint& p1,
                          const SkPoint& c2) {
        insertNormalNode(mNVerbs, c0, p1, c2);
    }

    void insertDuplicateNode(const int& verbId,
                             const SkPoint& c0,
                             const SkPoint& p1,
                             const SkPoint& c2,
                             const int& nDupl) {
        makeSpaceForNew(verbId, DUPLICATE);
        setDuplicateNodeValues(verbId, c0, p1, c2, nDupl);
    }

    void prependDuplicateNode(const SkPoint& c0,
                              const SkPoint& p1,
                              const SkPoint& c2,
                              const int& nDupl) {
        insertDuplicateNode(0, c0, p1, c2, nDupl);
    }

    void appendDuplicateNode(const SkPoint& c0,
                             const SkPoint& p1,
                             const SkPoint& c2,
                             const int& nDupl) {
        insertDuplicateNode(mNVerbs, c0, p1, c2, nDupl);
    }

    void insertShadowNode(const int& verbId, const SkScalar& t) {
        makeSpaceForNew(verbId, SHADOW);
        setShadowNodeValue(verbId, t);
    }

    void insertShadowNodeInTheMiddle(const int& verbId) {
        makeSpaceForNew(verbId, SHADOW);
        setShadowNodeValue(verbId, getShadowNodeTRange(verbId).middle());
    }

    void convertShadowNodeToNormalNode(const int& verbId) {
        auto vals = getAsShadowNode(verbId).toNodeValues();
        mVerbTypes.replace(verbId, NORMAL);
        setNormalNodeValues(verbId, vals.fC0, vals.fP1, vals.fC2);
    }

    void setNumberOfDuplicates(const int& verbId, const int& duplicates) {
        auto vals = getAsShadowNode(verbId).toNodeValues();
        mVerbTypes.replace(verbId, NORMAL);
        setNormalNodeValues(verbId, vals.fC0, vals.fP1, vals.fC2);
    }

    void appendShadowNode(const SkScalar& t) {
        insertShadowNode(mNVerbs, t);
    }

    void prependShadowNode(const SkScalar& t) {
        insertShadowNode(0, t);
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

    const int& getNodeCount() const {
        return mNVerbs;
    }

    ShadowPathNode getAsShadowNode(const int& verbId) {
        const int prevNormalId = prevId(verbId, NORMAL);
        const int nextNormalId = nextId(verbId, NORMAL);
        return ShadowPathNode(getValuesAsSkPoint(prevNormalId),
                              getValuesAsSkScalar(verbId),
                              getValuesAsSkPoint(nextNormalId));
    }

    NormalPathNode getAsNormalNode(const int& verbId) {
        return NormalPathNode(getValuesAsSkPoint(verbId));
    }

    DuplicatePathNode getAsDuplicateNode(const int& verbId) {
        return DuplicatePathNode(getValuesAsSkPoint(verbId));
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
                     const bool& simplified) :
            i(startI - 1), mTarget(targetT), mSimplified(simplified) {
            if(mSimplified) {
                if(currentType() == SHADOW) {
                    next();
                }
            }
        }

        NodeIterator(NodesHandler& targetT,
                     const bool& simplified) :
            NodeIterator(-1, targetT, simplified) {

        }

        VerbType currentType() const {
            const VerbType& type = mTarget.getType(i);
            if(mSimplified) { // if simplified treat duplicates as normal
                if(type == DUPLICATE) return NORMAL;
            }
            return type;
        }

        ShadowPathNode getCurrentAsShadowNode() {
            return mTarget.getAsShadowNode(i);
        }

        NormalPathNode getCurrentAsNormalNode() {
            return mTarget.getAsNormalNode(i);
        }

        DuplicatePathNode getCurrentAsDuplicateNode() {
            return mTarget.getAsDuplicateNode(i);
        }

        bool hasNext() const {
            return i < mTarget.getNodeCount() - 1;
        }

        bool next() {
            if(hasNext()) {
                i++;
                if(mSimplified) {
                    if(currentType() == SHADOW) {
                        return next();
                    }
                }
                return true;
            }
            return false;
        }

        NodeIterator peekNext() {
            return NodeIterator(i + 1, mTarget, mSimplified);
        }
    private:
        int i;
        NodesHandler& mTarget;
        bool mSimplified;
    };

    struct SegmentIterator {
        SegmentIterator(const int& startI, NodesHandler& targetT,
                        const bool& simplified) :
           mNode1Iterator(startI - 1, targetT, simplified),
           mNode2Iterator(startI, targetT, simplified) {
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

        DuplicateSegment getCurrentAsDuplicateSegment() {
            return DuplicateSegment(mNode2Iterator.getCurrentAsDuplicateNode(),
                                    mNode1Iterator.getCurrentAsDuplicateNode());
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

    SkPath toSkPath(const bool& simplified) {
        SkPath path;
        SegmentIterator iterator(*this, simplified);
        bool first = true;
        while(iterator.next()) {
            if(iterator.currentType() == NORMAL) {
                auto seg = iterator.getCurrentAsNormalSegment();
                if(first) {
                    path.moveTo(seg.fPrevNormal.p1());
                    continue;
                }
                seg.cubicTo(path);
            } else if(iterator.currentType() == DUPLICATE) {
                auto seg = iterator.getCurrentAsDuplicateSegment();
                if(first) {
                    path.moveTo(seg.fPrevDuplicate.p1());
                    continue;
                }
                seg.cubicTo(path);
            } else if(iterator.currentType() == SHADOW) {
                auto seg = iterator.getCurrentAsShadowSegment();
                if(first) {
                    path.moveTo(seg.fPrevNormal.p1());
                    continue;
                }
                seg.cubicTo(path);
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
                             const SkPoint& c2) {
        setDuplicateNodeValues(verbId, c0, p1, c2, 0);
    }

    void setDuplicateNodeValues(const int& verbId,
                                const SkPoint& c0,
                                const SkPoint& p1,
                                const SkPoint& c2,
                                const int& nDupl) {
        SkPoint * dst = getValuesAsSkPoint(verbId);
        *(dst++) = c0;
        *(dst++) = p1;
        *(dst++) = c2;
        *reinterpret_cast<SkScalar*>(dst) = nDupl;
    }

    void setShadowNodeValue(const int& verbId,
                            const SkScalar& t) {
        *getValuesAsSkScalar(verbId) = t;
    }

    void setMoveVerbValue(const int& verbId,
                          const SkPoint& moveTo) {
        *getValuesAsSkPoint(verbId) = moveTo;
    }

    QList<VerbType> mVerbTypes;
    SkScalar* mValues = nullptr;
    int mNValues = 0;
    int mNVerbs = 0;
};

#endif // SMARTPATHCONTAINER_H
