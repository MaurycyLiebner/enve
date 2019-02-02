#ifndef SMARTPATHCONTAINER_H
#define SMARTPATHCONTAINER_H
#include "simplemath.h"
#include "skia/skiaincludes.h"

class NodesHandler {
public:
    enum NodeSegType { NORMAL, DUPLICATE, SHADOW, NONE };

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

    struct DuplicatePathNode {
        DuplicatePathNode(SkPoint * const src) :
            fMainNode(src + 1) {
            const auto srcS = reinterpret_cast<SkScalar*>(src);
            fDuplBefore = srcS;
            fDuplAfter = srcS + 1;
        }

        void setDuplAfter(const int& count) const {
            *fDuplAfter = count;
        }

        void setDuplBefore(const int& count) const {
            *fDuplBefore = count;
        }

        int duplBefore() const {
            return qRound(*fDuplBefore);
        }

        int duplAfter() const {
            return qRound(*fDuplAfter);
        }

        int totalNodesCount() const {
            return duplBefore() + duplAfter() + 1;
        }

        QList<NodeValues> toNodeValues() const {
            QList<NodeValues> result;

            const int iMax = duplBefore() + duplAfter();
            for(int i = 0; i <= iMax; i++) {
                NodeValues values;
                values.fC0 = (i == 0 ? fMainNode.c0() : fMainNode.p1());
                values.fP1 = fMainNode.p1();
                values.fC2 = (i == iMax ? fMainNode.c2() : fMainNode.p1());
            }

            return result;
        }

        NormalPathNode fMainNode;
        SkScalar *fDuplBefore;
        SkScalar *fDuplAfter;
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

    void insertNormalNode(const int& nodeId,
                          const SkPoint& c0,
                          const SkPoint& p1,
                          const SkPoint& c2) {
        makeSpaceForNew(nodeId, NORMAL);
        setNormalNodeValues(nodeId, c0, p1, c2);
    }

    void prependNormalNode(const SkPoint& c0,
                           const SkPoint& p1,
                           const SkPoint& c2) {
        insertNormalNode(0, c0, p1, c2);
    }

    void appendNormalNode(const SkPoint& c0,
                          const SkPoint& p1,
                          const SkPoint& c2) {
        insertNormalNode(mNNodes, c0, p1, c2);
    }

    void insertShadowNode(const int& nodeId,
                          const SkScalar& t) {
        Q_ASSERT(nodeId > 0 && nodeId < mNNodes);
        makeSpaceForNew(nodeId, SHADOW);
        setShadowNodeValue(nodeId, t);
    }

    int prevId(const int& beforeId,
               const NodeSegType& type) const {
        for(int i = beforeId - 1; i >= 0; i--) {
            if(getType(i) == type) return i;
        }
        return -1;
    }

    int nextId(const int& afterId,
               const NodeSegType& type) const {
        for(int i = afterId + 1; i < mNNodes; i++) {
            if(getType(i) == type) return i;
        }
        return -1;
    }

    const NodeSegType& getType(const int& id) const {
        return mTypes.at(static_cast<int>(id));
    }

    const int& getNodeCount() const {
        return mNNodes;
    }

    ShadowPathNode getAsShadowNode(const int& nodeId) {
        const int prevNormalId = prevId(nodeId, NORMAL);
        const int nextNormalId = nextId(nodeId, NORMAL);
        return ShadowPathNode(getValuesForNormalNode(prevNormalId),
                              getValuesForShadowNode(nodeId),
                              getValuesForNormalNode(nextNormalId));
    }


    NormalPathNode getAsNormalNode(const int& nodeId) {
        return NormalPathNode(getValuesForNormalNode(nodeId));
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

        const NodeSegType& currentType() const {
            return mTarget.getType(i);
        }

        ShadowPathNode getCurrentAsShadowNode() {
            return mTarget.getAsShadowNode(i);
        }

        NormalPathNode getCurrentAsNormalNode() {
            return mTarget.getAsNormalNode(i);
        }

        bool hasNext() const {
            return i < mTarget.getNodeCount() - 1;
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
           mNode1Iterator(startI - 1, targetT, skipShadow),
           mNode2Iterator(startI, targetT, skipShadow) {
            updateShadowNodes();
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


        NodeSegType currentType() const {
            if(mCurrentShadowNodes.isEmpty()) return NORMAL;
            return SHADOW;
        }

        bool hasNext() const {
            return mNode2Iterator.hasNext();
        }

        bool next() {
            if(hasNext()) {
                mNode1Iterator.next();
                mNode2Iterator.next();
                updateShadowNodes();
                return true;
            }
            return false;
        }

        void updateShadowNodes() {
            mCurrentShadowNodes.clear();
            while(mNode2Iterator.currentType() == SHADOW) {
                mCurrentShadowNodes << mNode2Iterator.getCurrentAsShadowNode();
                mNode2Iterator.next();
            }
        }
    private:
        QList<ShadowPathNode> mCurrentShadowNodes;
        NodeIterator mNode1Iterator;
        NodeIterator mNode2Iterator;
    };

    SkPath toSkPath(const bool& skipShadow) {
        SkPath path;
        SegmentIterator iterator(*this, skipShadow);
        bool first = true;
        while(iterator.next()) {
            if(iterator.currentType() == NORMAL) {
                auto seg = iterator.getCurrentAsNormalSegment();
                if(first) {
                    path.moveTo(seg.fPrevNormal.p1());
                    continue;
                }
                seg.cubicTo(path);
            } else {
                auto seg = iterator.getCurrentAsShadowSegment();
                if(first) {
                    path.moveTo(seg.fPrevNormal.p1());
                    continue;
                }
                seg.cubicTo(path);
            }
        }
        return path;
    }
private:
    void sortShadowNodesStartingAt(const int& firstId) {
        int i = firstId;
        QList<SkScalar> ts;
        while(getType(firstId) == SHADOW) {
            auto node = getAsShadowNode(i++);
            ts << node.t();
        }
    }


    SkPoint * getValuesForNormalNode(const int& nodeId) {
        return reinterpret_cast<SkPoint*>(mValues + nodeId*6);
    }

    SkScalar * getValuesForShadowNode(const int& nodeId) {
        return mValues + nodeId*6;
    }

    void makeSpaceForNew(int newId, const NodeSegType& nodeType) {
        newId = qMax(0, qMin(newId, mNNodes));
        mTypes.insert(static_cast<int>(newId), nodeType);
        const int newNValues = mNValues + 6;
        const size_t newDataSize = static_cast<size_t>(newNValues)*sizeof(SkScalar);
        void * const newData = malloc(newDataSize);
        SkScalar * const newValues = static_cast<SkScalar*>(newData);
        if(mNValues == 0) {
            const int firstValId = newId*6;
            const size_t cpy1Size = static_cast<size_t>(firstValId)*
                    sizeof(SkScalar);
            if(firstValId) memcpy(newValues, mValues, cpy1Size);
            if(firstValId != mNValues) {
                const size_t cpy2Size = static_cast<size_t>(
                            mNValues - firstValId)*sizeof(SkScalar);
                memcpy(newValues + firstValId + 6,
                       mValues + firstValId,
                       cpy2Size);
            }
        }
        free(mValues);
        mValues = newValues;
        mNValues = newNValues;
        mNNodes++;
    }

    void setNormalNodeValues(const int& nodeId,
                             const SkPoint& c0,
                             const SkPoint& p1,
                             const SkPoint& c2) {
        SkPoint * dst = getValuesForNormalNode(nodeId);
        *(dst++) = c0;
        *(dst++) = p1;
        *dst = c2;
    }

    void setShadowNodeValue(const int& nodeId,
                            const SkScalar& t) {
        *getValuesForShadowNode(nodeId) = t;
    }

    QList<NodeSegType> mTypes;
    SkScalar* mValues = nullptr;
    int mNValues = 0;
    int mNNodes = 0;
};

#endif // SMARTPATHCONTAINER_H
