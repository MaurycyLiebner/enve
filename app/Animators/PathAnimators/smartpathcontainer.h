#ifndef SMARTPATHCONTAINER_H
#define SMARTPATHCONTAINER_H
#include "simplemath.h"
#include "skia/skiaincludes.h"
#include "exceptions.h"
#include "pointhelpers.h"

struct Node {
    enum Type {
        NORMAL, START, END, CLOSE, DISSOLVED, DUMMY, MOVE
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

    bool isClose() const { return fType == CLOSE; }

    bool isStart() const { return fType == START; }

    bool isEnd() const { return fType == END; }

    bool isMove() const { return fType == MOVE; }

    bool isNormal() const { return fType == NORMAL; }

    bool isDummy() const { return fType == DUMMY; }

    bool isDissolved() const { return fType == DISSOLVED; }

    QPointF fC0;
    QPointF fP1;
    QPointF fC2;

    //! @brief T value for segment defined by previous and next normal node
    qreal fT;
    Type fType;
};

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
    QPointF lastC2;
    bool first = true;
    Node firstMoveNode;
    Node prevNormalNode;
    for(const Node& node : nodes) {
        if(node.isDummy()) continue;

        const bool move = first || node.fType == Node::START;
        if(first) first = false;
        if(move) {
            firstMoveNode = node;
            result.moveTo(qPointToSk(node.fP1));
            lastC2 = node.fC2;
            continue;
        }
        result.cubicTo(qPointToSk(lastC2),
                       qPointToSk(node.fC0),
                       qPointToSk(node.fP1));
        if(node.fType == Node::END) {
            result.cubicTo(qPointToSk(node.fC2),
                           qPointToSk(firstMoveNode.fC0),
                           qPointToSk(firstMoveNode.fP1));
        } else {
            lastC2 = node.fC2;
        }

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

    void actionPromoteDissolvedNodeToNormal(const int& nodeId) {
        Node &node = mNodes[nodeId];
        updateNodeValuesBasedOnT(node, nodeId);
        node.fType = Node::NORMAL;
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
        for(int i = nodeId - 1; i >= 0; i--) {
            const Node& node = mNodes.at(i);
            if(node.isNormal()) return node;
        }
        return Node();
    }

    Node nextNormal(const int& nodeId) const {
        for(int i = nodeId + 1; i < mNodes.count(); i++) {
            const Node& node = mNodes.at(i);
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
        if(node.isNormal()) return false;
        const Node::Type prevType = mPrev ? mPrev->nodeType(nodeId) : Node::DUMMY;
        const Node::Type nextType = mNext ? mNext->nodeType(nodeId) : Node::DUMMY;
        if(prevType == Node::NORMAL || nextType == Node::NORMAL) {
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

    QList<Node> getNodesForPrev() {
        if(mPrev) return getNodesFor(mPrev->getNodes());
        return QList<Node>();
    }

    QList<Node> getNodesForNext() {
        if(mNext) return getNodesFor(mNext->getNodes());
        return QList<Node>();
    }
private:
    QList<Node> getNodesFor(const QList<Node>& neighbour) const {
        QList<Node> result = mNodes;

        int iMax = neighbour.count() - 1;
        int iShift = 0;
        for(int i = 0; i <= iMax; i++) {
            Node& resultNode = result[i + iShift];
            const Node& neighbourNode = neighbour.at(i);

            if(resultNode.fType == neighbourNode.fType) continue;
            if(neighbourNode.fType == Node::NORMAL &&
                    resultNode.isDissolved()) continue;

            if(neighbourNode.isNormal() && resultNode.isDissolved()) {
                resultNode.fType = neighbourNode.fType;
                continue;
            }
            if(neighbourNode.isDummy() && resultNode.isDissolved()) {
                resultNode.fType = Node::DUMMY;
                continue;
            }
            if(resultNode.fType == Node::NORMAL &&
                    neighbourNode.isNormal()) {
                resultNode.fType = neighbourNode.fType;
            }
        }
        return result;
    }

    stdsptr<NodeIdUsage> mIdUsage;
    stdptr<SmartPath> mPrev;
    QList<Node> mNodes;
    stdptr<SmartPath> mNext;
};

#endif // SMARTPATHCONTAINER_H
