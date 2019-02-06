#ifndef SMARTPATHCONTAINER_H
#define SMARTPATHCONTAINER_H
#include "simplemath.h"
#include "skia/skiaincludes.h"
#include "exceptions.h"
#include "pointhelpers.h"

struct Node {
    enum Type : char {
        DUMMY, DISSOLVED, NORMAL, NORMAL_CLOSE_AFTER, NORMAL_WRAP_AFTER, MOVE
    };

    Node() : Node(DUMMY) {}

    Node(const Type& type) { fType = type; }

    Node(const QPointF& c0, const QPointF& p1, const QPointF& c2) {
        fC0 = c0;
        fP1 = p1;
        fC2 = c2;
        fType = NORMAL;
    }

    Node(const QPointF& c0, const QPointF& p1, const QPointF& c2,
         const qreal& t) {
        fC0 = c0;
        fP1 = p1;
        fC2 = c2;
        fT = t;
        fType = DISSOLVED;
    }

    bool isMove() const { return fType == MOVE; }

    bool closeAfter() const { return fType == NORMAL_CLOSE_AFTER; }

    bool wrapAfter() const { return fType == NORMAL_WRAP_AFTER; }

    bool isNormal() const { return fType == NORMAL ||
                                   fType == NORMAL_WRAP_AFTER ||
                                   fType == NORMAL_CLOSE_AFTER; }

    bool isDummy() const { return fType == DUMMY; }

    bool isDissolved() const { return fType == DISSOLVED; }

    QPointF fC0;
    QPointF fP1;
    QPointF fC2;

    //! @brief T value for segment defined by previous and next normal node
    qreal fT;
    Type fType;
};

bool isNormal(const Node::Type& type) {
    return type == Node::NORMAL ||
           type == Node::NORMAL_WRAP_AFTER ||
           type == Node::NORMAL_CLOSE_AFTER;
}

//! @brief Assumes node1Id is before node2Id
bool areNodesAdjecent(const int& node1Id, const int& node2Id,
                      const QList<Node>& nodes) {
    if(node1Id + 1 == node2Id) return true;
    return node1Id == nodes.count() - 1 && node2Id == 0;;
}

int fixNegativeId(const int& id, const QList<Node>& nodes) {
    if(id >= 0) return id;
    return id + nodes.count();
}

SkPath nodesToSkPath(const QList<Node>& nodes) {
    SkPath result;
    bool move = true;

    Node firstMoveNode;
    Node prevNormalNode;
    Node firstNormalNode;

    bool currentSegmentWaiting = false;
    qCubicSegment2D currentNormalSegment;
    qreal lastT = 0;
    for(const Node& node : nodes) {
        if(node.isDummy()) continue;
        if(node.isNormal() && firstNormalNode.isDummy()) {
            firstNormalNode = node;
        }
        if(move) {
            if(!node.isNormal())
                RuntimeThrow("Segment starts with an unsupported type");
            firstMoveNode = node;
            result.moveTo(qPointToSk(node.fP1));
            move = false;
        } else {
            if(node.fType == Node::DISSOLVED) {
                if(!currentSegmentWaiting)
                    RuntimeThrow("No segment to divide");
                auto div = currentNormalSegment.dividedAtT(
                            gMapTToFragment(lastT, 1, node.fT));
                const auto& first = div.first;
                result.cubicTo(qPointToSk(first.c1()),
                               qPointToSk(first.c2()),
                               qPointToSk(first.p1()));
                currentNormalSegment = div.second;
                lastT = node.fT;
            } else { // if not dissolved
                if(currentSegmentWaiting) {
                    result.cubicTo(qPointToSk(currentNormalSegment.c1()),
                                   qPointToSk(currentNormalSegment.c2()),
                                   qPointToSk(currentNormalSegment.p1()));
                    currentSegmentWaiting = false;
                }
                if(node.fType == Node::NORMAL) {
                    currentNormalSegment = qCubicSegment2D(prevNormalNode.fP1,
                                                           prevNormalNode.fC2,
                                                           node.fC0, node.fP1);
                    currentSegmentWaiting = true;
                } else if(node.fType == Node::NORMAL_CLOSE_AFTER) {
                    currentNormalSegment = qCubicSegment2D(prevNormalNode.fP1,
                                                           prevNormalNode.fC2,
                                                           firstMoveNode.fC0,
                                                           firstMoveNode.fP1);
                    currentSegmentWaiting = true;
                } else if(node.fType == Node::NORMAL_WRAP_AFTER) {
                    currentNormalSegment = qCubicSegment2D(prevNormalNode.fP1,
                                                           prevNormalNode.fC2,
                                                           firstNormalNode.fC0,
                                                           firstNormalNode.fP1);
                    currentSegmentWaiting = true;
                } else if(node.fType == Node::MOVE) {
                    move = true;
                } else {
                    RuntimeThrow("Unrecognized node type");
                }
            } // if/else dissolved
        } // if/else move

        if(node.isNormal()) prevNormalNode = node;
    } // for each node

    if(currentSegmentWaiting) {
        result.cubicTo(qPointToSk(currentNormalSegment.c1()),
                       qPointToSk(currentNormalSegment.c2()),
                       qPointToSk(currentNormalSegment.p1()));
        currentSegmentWaiting = false;
    }

    return result;
}

qCubicSegment2D segmentFromNodes(const Node& prevNode,
                                 const Node& nextNode) {
    return qCubicSegment2D(prevNode.fP1, prevNode.fC2,
                           nextNode.fC0, nextNode.fP1);
}
#include "framerange.h"
#include "smartPointers/stdselfref.h"
#include "smartPointers/stdpointer.h"

class NodeIdUsage : public StdSelfRef {
public:
    //! @brief Increments and returns current usage.
    const int& incUsage(const int& id) {
        int& usage = mUsage[id];
        return ++usage;
    }

    //! @brief Decrements and returns current usage.
    const int& decUsage(const int& id) {
        int& usage = mUsage[id];
        if(usage == 0)
            RuntimeThrow("Node id usage reached negative value");
        return --usage;
    }

    void remove(const int& id) {
        if(mUsage.takeAt(id) != 0)
            RuntimeThrow("Non-zero usage removed");
    }

    void insertFirstUsage(const int& id) {
        mUsage.insert(id, 1);
    }
private:
    QList<int> mUsage;
};

class SmartPath : public StdSelfRef {
    enum Neighbour { NONE, NEXT, PREV, BOTH = NEXT | PREV };
public:
    void actionRemoveNormalNode(const int& nodeId) {
        Node& node = mNodes[nodeId];
        if(!node.isNormal())
            RuntimeThrow("Invalid node type. "
                         "Only normal nodes can be removed.");

        if(mIdUsage->decUsage(nodeId) == 0) {
            mIdUsage->remove(nodeId);
            if(mPrev) mPrev->removeNodeWithIdAndTellPrevToDoSame(nodeId);
            if(mNext) mNext->removeNodeWithIdAndTellNextToDoSame(nodeId);
        } else {
            node.fType = Node::DUMMY;
            if(mPrev) mPrev->updateNodeTypeAfterNeighbourChanged(nodeId);
            if(mNext) mNext->updateNodeTypeAfterNeighbourChanged(nodeId);
            updateNodeTypeAfterNeighbourChanged(nodeId);
        }
    }

    void updateNodeValuesBasedOnT(const int& nodeId) {
        Node& node = mNodes[nodeId];
        updateNodeValuesBasedOnT(node, nodeId);
    }

    void updateNodeValuesBasedOnT(Node& node, const int& nodeId) {
        auto seg = segmentFromNodes(prevNormal(nodeId), nextNormal(nodeId));
        auto div = seg.dividedAtT(node.fT);
        const auto& first = div.first;
        const auto& second = div.second;
        node.fC0 = first.c2();
        node.fP1 = first.p1();
        node.fC2 = second.c1();
    }

    void actionInsertNormalNode(const int& nodeId, const qreal& t) {
        mNodes.insert(nodeId, Node());
        Node &node = mNodes[nodeId];
        node.fT = t;
        updateNodeValuesBasedOnT(node, nodeId);
        node.fType = Node::NORMAL;
        if(mPrev) mPrev->normalNodeInsertedToNext(nodeId);
        if(mNext) mNext->normalNodeInsertedToPrev(nodeId);
    }

    void actionInsertNormalNode(const int& nodeId,
                                const QPointF& c0,
                                const QPointF& p1,
                                const QPointF& c2) {
        mNodes.insert(nodeId, Node(c0, p1, c2, Node::NORMAL));
        if(mPrev) mPrev->normalNodeInsertedToNext(nodeId);
        if(mNext) mNext->normalNodeInsertedToPrev(nodeId);
    }

    void promoteDissolvedNodeToNormal(const int& nodeId,
                                      QList<Node>& nodes) const {
        Node& node = nodes[nodeId];
        promoteDissolvedNodeToNormal(nodeId, node, nodes);
    }

    void promoteDissolvedNodeToNormal(const int& nodeId,
                                      Node& node,
                                      QList<Node>& nodes) const {
        const int prevNormalIdV = prevNormalId(nodeId, nodes);
        const int nextNormalIdV = nextNormalId(nodeId, nodes);
        Node& prevNormal = nodes[prevNormalIdV];
        Node& nextNormal = nodes[nextNormalIdV];

        auto seg = segmentFromNodes(prevNormal, nextNormal);
        auto div = seg.dividedAtT(node.fT);
        const auto& first = div.first;
        const auto& second = div.second;
        prevNormal.fC2 = first.c1();
        node.fC0 = first.c2();
        node.fP1 = first.p1();
        node.fC2 = second.c1();
        node.fType = Node::NORMAL;
        nextNormal.fC0 = second.c2();
        for(int i = prevNormalIdV + 1; i < nodeId; i++) {
            Node& iNode = nodes[i];
            if(iNode.isDissolved()) {
                iNode.fT = gMapTToFragment(0, node.fT, iNode.fT);
            }
        }
        for(int i = nodeId + 1; i < nextNormalIdV; i++) {
            Node& iNode = nodes[i];
            if(iNode.isDissolved()) {
                iNode.fT = gMapTToFragment(node.fT, 1, iNode.fT);
            }
        }
    }

    void splitNode(const int& nodeId, QList<Node>& nodes) const {
        Node& node = nodes[nodeId];
        splitNode(node, nodeId, nodes);
    }

    void splitNode(Node& node, const int& nodeId, QList<Node>& nodes) const {
        Node newNode = node;
        node.fC2 = node.fP1;
        newNode.fC0 = newNode.fP1;
        nodes.insert(nodeId + 1, newNode);
    }

    void splitNodeAndDisconnect(const int& nodeId,
                                QList<Node>& nodes) const {
        splitNode(nodeId, nodes);
        nodes.insert(nodeId + 1, Node(Node::MOVE));
    }

    void actionPromoteDissolvedNodeToNormal(const int& nodeId) {
        promoteDissolvedNodeToNormal(nodeId, mNodes);
        if(mPrev) mPrev->updateNodeTypeAfterNeighbourChanged(nodeId);
        if(mNext) mNext->updateNodeTypeAfterNeighbourChanged(nodeId);
    }

//    Neighbour nodeDissolvedFor(const int& nodeId) {
//        Node& node = mNodes[nodeId];
//        if(!node.isDissolved())
//            RuntimeThrow("Accepts only nodes with type DISSOLVED");
//        Neighbour neigh = NONE;
//        if(mPrev) {
//            if(NODE_TYPE_NORMAL(mPrev->nodeType(nodeId))) {
//                neigh = static_cast<Neighbour>(neigh | PREV);
//            }
//        }
//        if(mNext) {
//            if(NODE_TYPE_NORMAL(mNext->nodeType(nodeId))) {
//                neigh = static_cast<Neighbour>(neigh | NEXT);
//            }
//        }
//        return neigh;
//    }

    void normalNodeInsertedToPrev(const int& insertedNodeId) {
        mNodes.insert(insertedNodeId, Node());
        updateNodeTypeAfterNeighbourChanged(insertedNodeId);
        if(mNext) mNext->dissolvedNodeInsertedToPrev(insertedNodeId);
    }

    void dissolvedNodeInsertedToPrev(const int& insertedNodeId) {
        mNodes.insert(insertedNodeId, Node());
        if(mNext) mNext->dummyNodeInsertedToPrev(insertedNodeId);
    }

    void dummyNodeInsertedToPrev(const int& insertedNodeId) {
        mNodes.insert(insertedNodeId, Node());
        if(mNext) mNext->dummyNodeInsertedToPrev(insertedNodeId);
    }

    void normalNodeInsertedToNext(const int& insertedNodeId) {
        mNodes.insert(insertedNodeId, Node());
        updateNodeTypeAfterNeighbourChanged(insertedNodeId);
        if(mPrev) mPrev->dissolvedNodeInsertedToNext(insertedNodeId);
    }

    void dissolvedNodeInsertedToNext(const int& insertedNodeId) {
        mNodes.insert(insertedNodeId, Node());
        if(mPrev) mPrev->dummyNodeInsertedToNext(insertedNodeId);
    }

    void dummyNodeInsertedToNext(const int& insertedNodeId) {
        mNodes.insert(insertedNodeId, Node());
        if(mPrev) mPrev->dummyNodeInsertedToNext(insertedNodeId);
    }

    void removeNodeWithIdAndTellPrevToDoSame(const int& nodeId) {
        mNodes.removeAt(nodeId);
        if(mPrev) mPrev->removeNodeWithIdAndTellPrevToDoSame(nodeId);
    }

    void removeNodeWithIdAndTellNextToDoSame(const int& nodeId) {
        mNodes.removeAt(nodeId);
        if(mNext) mNext->removeNodeWithIdAndTellNextToDoSame(nodeId);
    }

    Node::Type nodeType(const int& nodeId) const {
        return mNodes.at(nodeId).fType;
    }

    bool isNodeNormal(const int& nodeId) const {
        return mNodes.at(nodeId).isNormal();
    }

    bool isNodeDissolved(const int& nodeId) const {
        return nodeType(nodeId) == Node::DISSOLVED;
    }

    bool isNodeDummy(const int& nodeId) const {
        return nodeType(nodeId) == Node::DUMMY;
    }

    QList<int> updateAllNodesTypeAfterNeighbourChanged() {
        QList<int> changed;
        for(int i = 0; i < mNodes.count(); i++) {
            if(updateNodeTypeAfterNeighbourChanged(i)) changed << i;
        }
        return changed;
    }

    Node prevNormal(const int& nodeId) const {
        return prevNormal(nodeId, mNodes);
    }

    Node nextNormal(const int& nodeId) const {
        return nextNormal(nodeId, mNodes);
    }

    int prevNormalId(const int& nodeId, const QList<Node>& nodes) const {
        for(int i = nodeId - 1; i >= 0; i--) {
            const Node& node = nodes.at(i);
            if(node.isNormal()) return i;
        }
        return -1;
    }

    int nextNormalId(const int& nodeId, const QList<Node>& nodes) const {
        for(int i = nodeId + 1; i < nodes.count(); i++) {
            const Node& node = nodes.at(i);
            if(node.isNormal()) return i;
        }
        return -1;
    }

    Node prevNormal(const int& nodeId, const QList<Node>& nodes) const {
        for(int i = nodeId - 1; i >= 0; i--) {
            const Node& node = nodes.at(i);
            if(node.isNormal()) return node;
        }
        return Node();
    }

    Node nextNormal(const int& nodeId, const QList<Node>& nodes) const {
        for(int i = nodeId + 1; i < nodes.count(); i++) {
            const Node& node = nodes.at(i);
            if(node.isNormal()) return node;
        }
        return Node();
    }

    Node prevNonDummy(const int& nodeId) const {
        for(int i = nodeId - 1; i >= 0; i--) {
            const Node& node = mNodes.at(i);
            if(node.fType != Node::DUMMY) return node;
        }
        return Node();
    }

    Node nextNonDummy(const int& nodeId) const {
        for(int i = nodeId + 1; i < mNodes.count(); i++) {
            const Node& node = mNodes.at(i);
            if(node.fType != Node::DUMMY) return node;
        }
        return Node();
    }

    qreal prevT(const int& nodeId) const {
        Node node = prevNonDummy(nodeId);
        if(node.isNormal()) return 0;
        return node.fT;
    }

    qreal nextT(const int& nodeId) const {
        Node node = nextNonDummy(nodeId);
        if(node.isNormal()) return 1;
        return node.fT;
    }

    //! @brief Returns true if changed.
    bool updateNodeTypeAfterNeighbourChanged(const int& nodeId) {
        Node& node = mNodes[nodeId];
        if(node.isNormal() || node.isMove()) return false;
        const Node::Type prevType = mPrev ? mPrev->nodeType(nodeId) : Node::DUMMY;
        const Node::Type nextType = mNext ? mNext->nodeType(nodeId) : Node::DUMMY;
        if(isNormal(prevType) || isNormal(nextType) ||
                prevType == Node::MOVE || nextType == Node::MOVE) {
            if(node.fType != Node::DISSOLVED) {
                node.fT = 0.5*(prevT(nodeId) + nextT(nodeId));
                updateNodeValuesBasedOnT(nodeId);
                node.fType = Node::DISSOLVED;
                return true;
            }
            return false;
        }
        if(node.fType != Node::DUMMY) {
            node.fType = Node::DUMMY;
            return true;
        }
        return false;
    }

    void setPrev(SmartPath * const prev) {
        mPrev = prev;
        updateAllNodesTypeAfterNeighbourChanged();
    }

    void setNext(SmartPath * const next) {
        mNext = next;
        updateAllNodesTypeAfterNeighbourChanged();
    }

    const QList<Node>& getNodes() const {
        return mNodes;
    }

    SkPath getPathForPrev() const {
        if(mPrev) return getPathFor(mPrev);
        return nodesToSkPath(mNodes);
    }

    SkPath getPathForNext() const {
        if(mNext) return getPathFor(mNext);
        return nodesToSkPath(mNodes);
    }
    SkPath getPathAt() const {
        return nodesToSkPath(mNodes);
    }

    SkPath interpolateWithNext(const qreal& nextWeight) const {
        if(!mNext) return getPathAt();
        SkPath result;
        getPathForNext().interpolate(mNext->getPathForPrev(),
                                     qrealToSkScalar(nextWeight),
                                     &result);
        return result;
    }

    SkPath interpolateWithPrev(const qreal& nextWeight) const {
        if(!mPrev) return getPathAt();
        SkPath result;
        getPathForPrev().interpolate(mPrev->getPathForNext(),
                                     qrealToSkScalar(nextWeight),
                                     &result);
        return result;
    }
private:
    SkPath getPathFor(SmartPath * const neighbour) const {
        const auto& neighNodes = neighbour->getNodes();
        QList<Node> result = mNodes;

        int iMax = neighNodes.count() - 1;
        if(result.count() - 1 != iMax)
            RuntimeThrow("Nodes count does not match");

        int iShift = 0;
        bool duplicateNext = false;
        int prevNormalId = -1;
        for(int i = 0; i <= iMax; i++) {
            const int resI = i + iShift;
            Node& resultNode = result[resI];
            const Node& neighbourNode = neighNodes.at(i);

            if((neighbourNode.isDummy() || neighbourNode.isDissolved()) &&
                    (resultNode.isDummy() || resultNode.isDissolved())) {
                iShift--;
                result.removeAt(resI);
            } else if(neighbourNode.isDissolved() && resultNode.isMove()) {
                splitNode(prevNormalId, result);
                iShift++;
                duplicateNext = true;
            } else if(resultNode.isDissolved() && neighbourNode.isMove()) {
                promoteDissolvedNodeToNormal(resI, result);
                splitNodeAndDisconnect(resI, result);
                iShift += 2;
            } else if(resultNode.isNormal()) {
                prevNormalId = resI;
                if(duplicateNext) {
                    splitNode(resultNode, resI, result);
                    iShift++;
                    duplicateNext = false;
                }
            }
        }
        return nodesToSkPath(result);
    }

    stdsptr<NodeIdUsage> mIdUsage;
    stdptr<SmartPath> mPrev;
    QList<Node> mNodes;
    stdptr<SmartPath> mNext;
};

#endif // SMARTPATHCONTAINER_H
