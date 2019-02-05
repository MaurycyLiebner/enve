#ifndef SMARTPATHCONTAINER_H
#define SMARTPATHCONTAINER_H
#include "simplemath.h"
#include "skia/skiaincludes.h"
#include "exceptions.h"
#include "pointhelpers.h"

#define EQUAL_3_OR(val, or1, or2, or3) val == or1 || val == or2 || val == or3
#define NODE_TYPE_NORMAL(type) \
    EQUAL_3_OR(type, Node::NORMAL_MIDDLE, Node::NORMAL_START, Node::NORMAL_END)
struct Node {
    enum Type {
        NORMAL_MIDDLE, NORMAL_START, NORMAL_END,
        NORMAL_START_AND_END, DISSOLVED, DUMMY
    };

    Node() {}

    Node(const QPointF& c0, const QPointF& p1, const QPointF& c2,
         const Type& type) {
        fC0 = c0;
        fP1 = p1;
        fC2 = c2;
        fType = type;
    }

    Node(const QPointF& c0, const QPointF& p1, const QPointF& c2,
         const qreal& t) : Node(c0, p1, c2, DISSOLVED) {
        fRelT = t;
    }

    bool isNormal() const {
        return NODE_TYPE_NORMAL(fType);
    }

    bool isDummy() const {
        return fType == DUMMY;
    }

    bool isDissolved() const {
        return fType == DISSOLVED;
    }

    QPointF fC0;
    QPointF fP1;
    QPointF fC2;

    //! @brief T value for segment defined by previous and next node
    qreal fRelT;
    Type fType = DUMMY;
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

        const bool move = first || node.fType == Node::NORMAL_START;
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
        if(node.fType == Node::NORMAL_END) {
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
class SmartPath : public StdSelfRef {
public:
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

    Node prevNonDummy(const int& nodeId) {
        for(int i = nodeId - 1; i >= 0; i--) {
            const Node& node = mNodes.at(i);
            if(node.fType != Node::DUMMY) return node;
        }
        return Node();
    }

    Node nextNonDummy(const int& nodeId) {
        for(int i = nodeId + 1; i < mNodes.count(); i++) {
            const Node& node = mNodes.at(i);
            if(node.fType != Node::DUMMY) return node;
        }
        return Node();
    }

    //! @brief Returns true if changed.
    bool updateNodeTypeAfterNeighbourChanged(const int& nodeId) {
        Node& node = mNodes[nodeId];
        if(node.isNormal()) return false;
        const Node::Type prevType = mPrev ? mPrev->nodeType(nodeId) : Node::DUMMY;
        const Node::Type nextType = mNext ? mNext->nodeType(nodeId) : Node::DUMMY;
        if(NODE_TYPE_NORMAL(prevType) || NODE_TYPE_NORMAL(nextType)) {
            if(node.fType != Node::DISSOLVED) {
                node.fRelT = 0.5;
                node.fType = Node::DISSOLVED;
//                auto seg = segmentFromNodes(prevNonDummy(nodeId),
//                                            nextNonDummy(nodeId));
//                auto div = seg.dividedAtT(node.fRelT);
//                const auto& first = div.first;
//                const auto& second = div.second;
//                node.fC0 = first.c2();
//                node.fP1 = first.p1();
//                node.fC2 = second.c1();

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
        for(int i = 0; i <= iMax; i++) {
            Node& resultNode = result[i];
            const Node& neighbourNode = neighbour.at(i);

            if(resultNode.fType == neighbourNode.fType) continue;
            if(neighbourNode.fType == Node::NORMAL_MIDDLE &&
                    resultNode.isDissolved()) continue;
            if(neighbourNode.isNormal() &&
                    resultNode.isDissolved()) {
                resultNode.fType = neighbourNode.fType;
                continue;
            }
            if(neighbourNode.isDummy() && resultNode.isDissolved()) {
                    resultNode.fType = Node::DUMMY;
                continue;
            }
            if(resultNode.fType == Node::NORMAL_MIDDLE &&
                    neighbourNode.isNormal()) {
                resultNode.fType = neighbourNode.fType;
            }
        }
        return result;
    }

    stdptr<SmartPath> mPrev;
    QList<Node> mNodes;
    stdptr<SmartPath> mNext;
};

#endif // SMARTPATHCONTAINER_H
