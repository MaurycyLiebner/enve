#ifndef PATHCONTAINER_H
#define PATHCONTAINER_H
#include <QtCore>
#include "skia/skiaincludes.h"
#include "simplemath.h"
struct NodeSettings;
enum CtrlsMode : short;


class NodesHandler {
public:
    enum NodeSegType { NORMAL, SHADOW, NONE };

    struct NodeValues {
        SkPoint fC0;
        SkPoint fP1;
        SkPoint fC2;
    };

    struct NormalPathNode {
        NormalPathNode(SkPoint * const src) :
            fC0(*src), fP1(*(src + 1)), fC2(*(src + 2)) {}

        SkPoint &fC0;
        SkPoint &fP1;
        SkPoint &fC2;
    };

    struct ShadowPathNode {
        ShadowPathNode(const NormalPathNode& prev,
                       SkScalar * const src,
                       const NormalPathNode& next) :
            fPrevNormal(prev), fT(*src), fNextNormal(next) {}

        NodeValues getNodeValues() const {
            qreal t = CLAMP(static_cast<qreal>(fT), 0, 1);
            if(isZero6Dec(t)) {
                return {fPrevNormal.fP1, fPrevNormal.fP1, fPrevNormal.fP1};
            }
            if(isZero6Dec(t - 1)) {
                return {fNextNormal.fP1, fNextNormal.fP1, fNextNormal.fP1};
            }
            QPointF p0 = skPointToQ(fPrevNormal.fP1);
            QPointF c1 = skPointToQ(fPrevNormal.fC2);
            QPointF c2 = skPointToQ(fNextNormal.fC0);
            QPointF p1 = skPointToQ(fNextNormal.fP1);

            qreal oneMinusT = 1 - t;
            QPointF P0_1 = p0*oneMinusT + c1*t;
            QPointF P1_2 = c1*oneMinusT + c2*t;
            QPointF P2_3 = c2*oneMinusT + p1*t;

            QPointF P01_12 = P0_1*oneMinusT + P1_2*t;
            QPointF P12_23 = P1_2*oneMinusT + P2_3*t;

            QPointF P0112_1223 = P01_12*oneMinusT + P12_23*t;

            //qCubicSegment2D seg1(p0, P0_1, P01_12, P0112_1223);
            //qCubicSegment2D seg2(P0112_1223, P12_23, P2_3, p1);

            return {qPointToSk(P01_12), qPointToSk(P0112_1223),
                        qPointToSk(P12_23)};
        }

        NormalPathNode fPrevNormal;
        SkScalar &fT;
        NormalPathNode fNextNormal;
    };

    struct NormalSegment {
        NormalSegment(const NormalPathNode& prev,
                      const NormalPathNode& next) :
            fPrevNormal(prev), fNextNormal(next) {}

        void cubicTo(SkPath &path) {
            path.cubicTo(fPrevNormal.fC2, fNextNormal.fC0, fNextNormal.fP1);
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
                path.cubicTo(fPrevNormal.fC2, fNextNormal.fC0, fNextNormal.fP1);
                return;
            }
            auto lastPtValues = fShadowNodes.first().getNodeValues();
            path.cubicTo(fPrevNormal.fC2, lastPtValues.fC0, lastPtValues.fP1);
            for(int i = 1; i < fShadowNodes.count(); i++) {
                auto ptValues = fShadowNodes.first().getNodeValues();
                path.cubicTo(lastPtValues.fC2, ptValues.fC0, ptValues.fP1);
                lastPtValues = ptValues;
            }
            path.cubicTo(lastPtValues.fC2, fNextNormal.fC0, fNextNormal.fP1);
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
        makeSpaceForNew(nodeId, SHADOW);
        setShadowNodeValue(nodeId, t);
    }

    void prependShadowNode(const SkScalar& t) {
        insertShadowNode(0, t);
    }

    void appendShadowNode(const SkScalar& t) {
        insertShadowNode(mNNodes, t);
    }

    SkPoint * getValuesForNormalNode(const int& nodeId) {
        return reinterpret_cast<SkPoint*>(mValues + nodeId*6);
    }

    SkScalar * getValuesForShadowNode(const int& nodeId) {
        return mValues + nodeId*6;
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

    struct NodeIterator {
        NodeIterator(const int& startI,
                     NodesHandler& targetT,
                     const bool& skipShadow) :
            i(startI), mTarget(targetT), mSkipShadow(skipShadow) {
            if(mSkipShadow) {
                if(currentType() == SHADOW) {
                    next();
                }
            }
        }

        NodeIterator(NodesHandler& targetT,
                     const bool& skipShadow) :
            NodeIterator(0, targetT, skipShadow) {

        }

        const NodeSegType& currentType() const {
            return mTarget.getType(i);
        }

        ShadowPathNode getCurrentAsShadowNode() {
            const int prevNormalId = mTarget.prevId(i, NORMAL);
            const int nextNormalId = mTarget.nextId(i, NORMAL);
            return ShadowPathNode(mTarget.getValuesForNormalNode(prevNormalId),
                                  mTarget.getValuesForShadowNode(i),
                                  mTarget.getValuesForNormalNode(nextNormalId));
        }

        NormalPathNode getCurrentAsNormalNode() {
            return NormalPathNode(mTarget.getValuesForNormalNode(i));
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
            return NodeIterator(i + 1, mTarget);
        }
    private:
        int i;
        NodesHandler& mTarget;
        bool mSkipShadow;
    };

    struct SegmentIterator {
        SegmentIterator(const int& startI, NodesHandler& targetT,
                        const bool& skipShadow) :
           mNode1Iterator(startI, targetT, skipShadow),
           mNode2Iterator(startI + 1, targetT, skipShadow) {
            updateShadowNodes();
        }

        SegmentIterator(NodesHandler& targetT,
                        const bool& skipShadow) :
            SegmentIterator(0, targetT, skipShadow) {}


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
private:
    void makeSpaceForNew(int newId, const NodeSegType& nodeType) {
        newId = qMax(0, qMin(newId, mNNodes));
        mTypes.insert(static_cast<int>(newId), nodeType);
        const int newNValues = mNValues + 6;
        void * const newData = malloc(newNValues*sizeof(SkScalar));
        SkScalar * const newValues = static_cast<SkScalar*>(newData);
        const int firstValId = newId*6;
        if(firstValId) memcpy(newValues, mValues, firstValId*sizeof(SkScalar));
        if(firstValId != mNValues) {
            memcpy(newValues + firstValId + 6,
                   mValues + firstValId,
                   (mNValues - firstValId)*sizeof(SkScalar));
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

class PathContainer {
public:
    PathContainer() {}
    virtual ~PathContainer() {}

    const QList<SkPoint> &getElementsPosList() const {
        return mElementsPos;
    }
    const SkPoint &getElementPos(const int &index) const;
    virtual void setElementPos(const int &index,
                               const SkPoint &pos);
    void addNodeElements(int startPtIndex,
                 const SkPoint &startPos,
                 const SkPoint &pos,
                 const SkPoint &endPos);
    void removeElementPosAt(const int &index);
    const SkPath &getPath();
    void updatePath();
    virtual void setPathClosed(const bool &bT);
    virtual void setElementsFromSkPath(const SkPath &path);
    void clearElements();

    void addNewPointAtTBetweenPts(const SkScalar &tVal,
                                  const int &id1,
                                  const int &id2,
                                  const bool &newPtSmooth);

    virtual NodeSettings *getNodeSettingsForPtId(const int &ptId) = 0;
    void setCurrentPath(const SkPath &path) {
        mPath = path;
    }

    int pointIdToNodeId(const int &ptId) {
        return ptId/3;
    }

    int nodeIdToPointId(const int &nodeId) {
        return nodeId*3 + 1;
    }

    virtual void removeNodeAt(const int &nodeId) {
        int nodePtId = nodeIdToPointId(nodeId);
        removeNodeElements(nodePtId);
    }

    virtual void removeNodeAtAndApproximate(const int &nodeId) {
        int nodePtId = nodeIdToPointId(nodeId);
        bool isLast = nodePtId == 1 ||
                (nodePtId == mElementsPos.count() - 2 && !mPathClosed);
        if(!isLast) {
            int prevCtrlId = nodePtId - 2;
            if(prevCtrlId < 0) prevCtrlId = mElementsPos.count() - 1;
            int nextCtrlId = nodePtId + 2;
            if(nextCtrlId >= mElementsPos.count()) nextCtrlId = 0;
            // make approximate
        }
        removeNodeElements(nodePtId - 1);
    }

    virtual void setCtrlsModeForNode(const int &nodeId,
                                     const CtrlsMode &mode);

    virtual void moveElementPosSubset(
            int firstId,
            int count,
            int targetId) {
        if(firstId == targetId) return;
        if(targetId == -1) {
            targetId = mElementsPos.count() - count;
        }
        if(count == -1) {
            count = mElementsPos.count() - firstId;
        }
        int lastId = firstId + count - 1;
        if(targetId > firstId) {
            int takeId = lastId + 1;
            int moveTo = firstId;
            for(int i = 0; i < targetId - firstId; i++) {
                mElementsPos.move(takeId, moveTo);
                takeId++;
                moveTo++;
            }
        } else {
            int takeId = firstId - 1;
            for(int i = 0; i < firstId - targetId; i++) {
                mElementsPos.move(takeId, lastId + 1);
                takeId--;
            }
        }
    }

    virtual void applyTransformToPoints(const QMatrix &transform);

    virtual void revertElementPosSubset(
            const int &firstId,
            int count) {
        if(count == -1) {
            count = mElementsPos.count() - firstId;
        }
        int lastId = firstId + count - 1;
        for(int i = 0; i < count; i++) {
            mElementsPos.move(lastId, firstId + i);
        }
    }

    QList<SkPoint> takeElementsPosSubset(const int &firstId,
                                         int count) {
        if(count == -1) {
            count = mElementsPos.count() - firstId;
        }
        QList<SkPoint> elements;
        for(int i = 0; i < count; i++) {
            elements << mElementsPos.takeAt(firstId);
        }
        setPathClosed(false);
        return elements;
    }

    void readPathContainer(QIODevice *target);
    void writePathContainer(QIODevice * const target) const;
    static QList<SkPoint> extractElementsFromSkPath(const SkPath &path);

    virtual void shiftAllPoints(const int &by) {
        if(by == 0) return;
        if(mPathClosed) {
            for(int i = 0; i < by*3; i++) {
                mElementsPos.prepend(mElementsPos.takeLast());
            }
            for(int i = 0; i < -by*3; i++) {
                mElementsPos.append(mElementsPos.takeFirst());
            }
            mPathUpdateNeeded = true;
        }
    }

    void schedulePathUpdate() {
        mPathUpdateNeeded = true;
    }

    virtual void revertAllPoints() {
        revertElementPosSubset(0, -1);
        PathContainer::shiftAllPoints(1);
        mPathUpdateNeeded = true;
    }

    virtual void startPathChange() {
        if(mPathChanged) return;
        mPathChanged = true;
        mSavedElementsPos = mElementsPos;
        return;
    }

    virtual void cancelPathChange() {
        mPathChanged = false;
    }

    virtual void finishedPathChange();

    void setElementsPos(const QList<SkPoint> &newElementsPos) {
        mElementsPos = newElementsPos;
    }
    void removeNodeElements(const int &startPtIndex);

    virtual void updateAfterChangedFromInside() = 0;

    virtual void mergeNodes(const int &nodeId1,
                            const int &nodeId2);
protected:
    bool mPathChanged = false;
    bool mPathClosed = false;
    bool mPathUpdateNeeded = false;
    SkPath mPath;
    QList<SkPoint> mElementsPos;
    QList<SkPoint> mSavedElementsPos;
};

#endif // PATHCONTAINER_H
