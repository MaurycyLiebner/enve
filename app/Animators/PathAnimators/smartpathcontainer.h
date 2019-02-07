#ifndef SMARTPATHCONTAINER_H
#define SMARTPATHCONTAINER_H
#include "simplemath.h"
#include "skia/skiaincludes.h"
#include "exceptions.h"
#include "pointhelpers.h"

struct Node {
    enum Type : char {
        DUMMY, DISSOLVED, NORMAL, MOVE
    };

    Node() : Node(DUMMY) {}

    Node(const Type& type) { mType = type; }

    Node(const QPointF& c0, const QPointF& p1, const QPointF& c2) {
        fC0 = c0;
        fP1 = p1;
        fC2 = c2;
        mType = NORMAL;
    }

    Node(const QPointF& c0, const QPointF& p1, const QPointF& c2,
         const qreal& t) {
        fC0 = c0;
        fP1 = p1;
        fC2 = c2;
        fT = t;
        mType = DISSOLVED;
    }

    bool isMove() const { return mType == MOVE; }

    bool isNormal() const { return mType == NORMAL; }

    bool isDummy() const { return mType == DUMMY; }

    bool isDissolved() const { return mType == DISSOLVED; }

    int getNextNodeId() const {
        if(isDummy() || isMove()) return -1;
        return mNextNodeId;
    }
    int getPrevNodeId() const {
        if(isDummy() || isMove()) return -1;
        return mPrevNodeId;
    }

    void setNextNodeId(const int& nextNodeId) {
        mNextNodeId = nextNodeId;
    }

    void setPrevNodeId(const int& prevNodeId) {
        mPrevNodeId = prevNodeId;
    }

    QPointF fC0;
    QPointF fP1;
    QPointF fC2;

    //! @brief T value for segment defined by previous and next normal node
    qreal fT;

    void setType(const Type& type) {
        mType = type;
        if(isDummy() || isMove()) {
            mNextNodeId = -1;
            mPrevNodeId = -1;
        }
    }

    const Type& getType() const { return mType; }

    void shiftIdsGreaterThan(const int& greater, const int& shiftBy) {
        if(mNextNodeId) if(mNextNodeId > greater) mNextNodeId += shiftBy;
        if(mPrevNodeId) if(mPrevNodeId > greater) mPrevNodeId += shiftBy;
    }

    void shiftIdsSmallerThan(const int& smaller, const int& shiftBy) {
        if(mNextNodeId) if(mNextNodeId < smaller) mNextNodeId += shiftBy;
        if(mPrevNodeId) if(mPrevNodeId < smaller) mPrevNodeId += shiftBy;
    }
private:
    Type mType;

    //! @brief Next connected node id in the list.
    //! Used with dissolved node and all normal nodes
    int mNextNodeId = -1;

    //! @brief Previous connected node id in the list.
    //! Used with dissolved node and all normal nodes
    int mPrevNodeId = -1;
};

//class NodesListHandler {

//    void in
//public:
//    QList<Node> mNodes;
//};

int firstConnectedNodeId(const int& nodeId, const QList<Node>& nodes) {
    if(nodeId < 0) return -1;
    if(nodeId >= nodes.count()) return -1;
    const Node * currNode = &nodes.at(nodeId);
    if(currNode->isDummy() || currNode->isMove()) return -1;
    int smallestId = nodeId;
    int currId = nodeId;
    while(true) {
        if(currNode->isDummy())
            RuntimeThrow("Dummy used as a previous node(should be skipped)");
        //if(currNode->isDissolved() || currNode->isNormal())
        const int prevId = currNode->getPrevNodeId();
        if(prevId == currId)
            RuntimeThrow("Node points to itself");
        if(!prevId) return nodeId;
        if(prevId == smallestId) return smallestId;
        smallestId = qMin(prevId, smallestId);
        currNode = &nodes.at(prevId);
        currId = prevId;
    }
}

QList<Node> sortNodeListAccoringToConnetions(const QList<Node>& srcList) {
    QList<Node> result;
    QList<int> srcIds;
    for(int i = 0; i < srcList.count(); i++) {
        srcIds << i;
    }
    while(!srcIds.isEmpty()) {
        const int firstSrcId = firstConnectedNodeId(srcIds.first(), srcList);
        if(!firstSrcId) {
            srcIds.removeFirst();
            continue;
        }
        const int firstResultId = result.count();
        int nextSrcId = firstSrcId;
        bool first = true;
        while(true) {
            srcIds.removeOne(nextSrcId);
            const Node& srcNode = srcList.at(nextSrcId);
            Node newNode = srcNode;
            if(first) first = false;
            else newNode.setPrevNodeId(result.count() - 1);

            if(newNode.getNextNodeId())
                newNode.setNextNodeId(result.count() + 1);

            result << newNode;

            nextSrcId = srcNode.getNextNodeId();
            if(!nextSrcId) break;
            if(nextSrcId == firstSrcId) {
                result[result.count() - 1].setNextNodeId(firstResultId);
                result[firstResultId].setPrevNodeId(result.count() - 1);
                break;
            }
        }
    }
    return result;
}

SkPath nodesToSkPath(const QList<Node>& nodes) {
    QList<Node> sortedNodes = sortNodeListAccoringToConnetions(nodes);
    SkPath result;
    bool move = true;

    bool close = false;
    Node firstNode;
    Node prevNormalNode;

    qCubicSegment2D currentNormalSegment;
    QList<qreal> dissolvedTs;

    for(int i = 0; i < sortedNodes.count(); i++) {
        const Node& node = sortedNodes.at(i);
        if(node.isDummy()) continue;
        if(move) {
            if(close) {
                qCubicSegment2D currentNormalSegment(prevNormalNode.fP1,
                                                     prevNormalNode.fC2,
                                                     firstNode.fC0,
                                                     firstNode.fP1);
                qreal lastT = 0;
                for(const qreal& t : dissolvedTs) {
                    auto div = currentNormalSegment.dividedAtT(
                                gMapTToFragment(lastT, 1, t));
                    const auto& first = div.first;
                    result.cubicTo(qPointToSk(first.c1()),
                                   qPointToSk(first.c2()),
                                   qPointToSk(first.p1()));
                    currentNormalSegment = div.second;
                    lastT = t;
                }
                result.cubicTo(qPointToSk(currentNormalSegment.c1()),
                               qPointToSk(currentNormalSegment.c2()),
                               qPointToSk(currentNormalSegment.p1()));
                dissolvedTs.clear();
            }
            if(!node.isNormal())
                RuntimeThrow("Segment starts with an unsupported type");
            firstNode = node;
            prevNormalNode = node;
            close = firstNode.getPrevNodeId();
            result.moveTo(qPointToSk(node.fP1));
            move = false;
        } else {
            if(node.getType() == Node::DISSOLVED) {
                dissolvedTs << node.fT;
            } else { // if not dissolved
                if(node.isNormal()) {
                    qCubicSegment2D currentNormalSegment(prevNormalNode.fP1,
                                                         prevNormalNode.fC2,
                                                         node.fC0, node.fP1);
                    qreal lastT = 0;
                    for(const qreal& t : dissolvedTs) {
                        auto div = currentNormalSegment.dividedAtT(
                                    gMapTToFragment(lastT, 1, t));
                        const auto& first = div.first;
                        result.cubicTo(qPointToSk(first.c1()),
                                       qPointToSk(first.c2()),
                                       qPointToSk(first.p1()));
                        currentNormalSegment = div.second;
                        lastT = t;
                    }
                    result.cubicTo(qPointToSk(currentNormalSegment.c1()),
                                   qPointToSk(currentNormalSegment.c2()),
                                   qPointToSk(currentNormalSegment.p1()));
                    dissolvedTs.clear();
                } else if(node.getType() == Node::MOVE) {
                    move = true;
                } else {
                    RuntimeThrow("Unrecognized node type");
                }
            } // if/else dissolved
        } // if/else move

        if(node.isNormal()) prevNormalNode = node;
    } // for each node

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
    void removeNodeFromList(const int& nodeId) {
        mNodes.removeAt(nodeId);
        for(int i = nodeId; i < mNodes.count(); i++) {
            Node& iNode = mNodes[i];
            iNode.shiftIdsGreaterThan(nodeId, -1);
        }
    }

    Node& insertNodeToList(const int& nodeId, const Node& node) {
        mNodes.insert(nodeId, node);
        for(int i = nodeId; i < mNodes.count(); i++) {
            Node& iNode = mNodes[i];
            iNode.shiftIdsGreaterThan(nodeId - 1, 1);
        }
        return mNodes[nodeId];
    }

    Node& prependNodeToList(const Node& node) {
        return insertNodeToList(0, node);
    }

    Node& appendNodeToList(const Node& node) {
        return insertNodeToList(mNodes.count(), node);
    }

    void actionRemoveNormalNode(const int& nodeId) {
        Node& node = mNodes[nodeId];
        if(!node.isNormal())
            RuntimeThrow("Invalid node type. "
                         "Only normal nodes can be removed.");

        removeNodeFromList(nodeId);

        if(mIdUsage->decUsage(nodeId) == 0) {
            mIdUsage->remove(nodeId);
            if(mPrev) mPrev->removeNodeWithIdAndTellPrevToDoSame(nodeId);
            if(mNext) mNext->removeNodeWithIdAndTellNextToDoSame(nodeId);
        } else {
            node.setType(Node::DUMMY);
            if(mPrev) mPrev->updateNodeTypeAfterNeighbourChanged(nodeId);
            if(mNext) mNext->updateNodeTypeAfterNeighbourChanged(nodeId);
            updateNodeTypeAfterNeighbourChanged(nodeId);
        }
    }

    void actionInsertNormalNode(const int& nodeId, const qreal& t) {
        Node &node = insertNodeToList(nodeId, Node());
        node.fT = t;
        node.setType(Node::DISSOLVED);
        promoteDissolvedNodeToNormal(nodeId, node, mNodes);
        if(mPrev) mPrev->normalNodeInsertedToNext(nodeId);
        if(mNext) mNext->normalNodeInsertedToPrev(nodeId);
    }

    void actionAddNormalNodeAtEnd(const int& nodeId,
                                  const QPointF& c0,
                                  const QPointF& p1,
                                  const QPointF& c2) {
        insertNodeToList(nodeId, Node(c0, p1, c2, Node::NORMAL));
        if(mPrev) mPrev->normalNodeInsertedToNext(nodeId);
        if(mNext) mNext->normalNodeInsertedToPrev(nodeId);
    }

    void actionPromoteDissolvedNodeToNormal(const int& nodeId) {
        promoteDissolvedNodeToNormal(nodeId, mNodes);
        if(mPrev) mPrev->updateNodeTypeAfterNeighbourChanged(nodeId);
        if(mNext) mNext->updateNodeTypeAfterNeighbourChanged(nodeId);
    }

    void actionDisconnectNodes(const int& node1Id, const int& node2Id) {
        Node& node1 = mNodes[node1Id];
        Node& node2 = mNodes[node2Id];
        if(node1.getNextNodeId() == node2Id) {
            node1.setNextNodeId(-1);
            node2.setPrevNodeId(-1);
        } else if(node2.getNextNodeId() == node1Id) {
            node1.setPrevNodeId(-1);
            node2.setNextNodeId(-1);
        } else {
            RuntimeThrow("Trying to disconnect not connected nodes");
        }

        insertNodeToList(node1Id + 1, Node(Node::MOVE));
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
        node.setType(Node::NORMAL);
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
        if(node.isNormal()) {
            node.fC2 = node.fP1;
            newNode.fC0 = newNode.fP1;
            nodes.insert(nodeId + 1, newNode);
        } else if(node.isDissolved()) {
            nodes.insert(nodeId + 1, newNode);
        }
    }

    void splitNodeAndDisconnect(const int& nodeId,
                                QList<Node>& nodes) const {
        Node& node = nodes[nodeId];
        if(!node.isNormal())
            RuntimeThrow("Can only disconnect normal nodes.");
        splitNode(node, nodeId, nodes);
        nodes.insert(nodeId + 1, Node(Node::MOVE));
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
        insertNodeToList(insertedNodeId, Node());
        updateNodeTypeAfterNeighbourChanged(insertedNodeId);
        if(mNext) mNext->dissolvedNodeInsertedToPrev(insertedNodeId);
    }

    void dissolvedNodeInsertedToPrev(const int& insertedNodeId) {
        insertNodeToList(insertedNodeId, Node());
        if(mNext) mNext->dummyNodeInsertedToPrev(insertedNodeId);
    }

    void dummyNodeInsertedToPrev(const int& insertedNodeId) {
        insertNodeToList(insertedNodeId, Node());
        if(mNext) mNext->dummyNodeInsertedToPrev(insertedNodeId);
    }

    void normalNodeInsertedToNext(const int& insertedNodeId) {
        insertNodeToList(insertedNodeId, Node());
        updateNodeTypeAfterNeighbourChanged(insertedNodeId);
        if(mPrev) mPrev->dissolvedNodeInsertedToNext(insertedNodeId);
    }

    void dissolvedNodeInsertedToNext(const int& insertedNodeId) {
        insertNodeToList(insertedNodeId, Node());
        if(mPrev) mPrev->dummyNodeInsertedToNext(insertedNodeId);
    }

    void dummyNodeInsertedToNext(const int& insertedNodeId) {
        insertNodeToList(insertedNodeId, Node());
        if(mPrev) mPrev->dummyNodeInsertedToNext(insertedNodeId);
    }

    void removeNodeWithIdAndTellPrevToDoSame(const int& nodeId) {
        removeNodeFromList(nodeId);
        if(mPrev) mPrev->removeNodeWithIdAndTellPrevToDoSame(nodeId);
    }

    void removeNodeWithIdAndTellNextToDoSame(const int& nodeId) {
        removeNodeFromList(nodeId);
        if(mNext) mNext->removeNodeWithIdAndTellNextToDoSame(nodeId);
    }

    const Node::Type& nodeType(const int& nodeId) const {
        return mNodes.at(nodeId).getType();
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
            if(node.getType() != Node::DUMMY) return node;
        }
        return Node();
    }

    Node nextNonDummy(const int& nodeId) const {
        for(int i = nodeId + 1; i < mNodes.count(); i++) {
            const Node& node = mNodes.at(i);
            if(node.getType() != Node::DUMMY) return node;
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
        Node::Type prevType = Node::DUMMY;
        Node::Type nextType = Node::DUMMY;
        int prevNextId = -1;
        int nextNextId = -1;
        if(mPrev) {
            const Node& prevNode = mPrev->getNodes().at(nodeId);
            prevType = prevNode.getType();
            prevNextId = prevNode.getNextNodeId();
        }
        if(mNext) {
            const Node& nextNode = mNext->getNodes().at(nodeId);
            nextType = nextNode.getType();
            nextNextId = nextNode.getNextNodeId();
        }
        if(prevType == Node::NORMAL || nextType == Node::NORMAL ||
           prevType == Node::MOVE || nextType == Node::MOVE ||
           (node.getNextNodeId() != nextNextId && nextType != Node::DUMMY) ||
           (node.getNextNodeId() != prevNextId && prevType != Node::DUMMY)) {
            if(node.getType() != Node::DISSOLVED) {
                node.fT = 0.5*(prevT(nodeId) + nextT(nodeId));
                node.setType(Node::DISSOLVED);
                return true;
            }
            return false;
        }
        if(node.getType() != Node::DUMMY) {
            node.setType(Node::DUMMY);
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
    SkPath getPathForPrev() const {
        if(mPrev) return getPathFor(mPrev);
        return nodesToSkPath(mNodes);
    }

    SkPath getPathForNext() const {
        if(mNext) return getPathFor(mNext);
        return nodesToSkPath(mNodes);
    }

    bool shouldSplitThisNode(const Node& thisNode, const Node& neighNode) const {
        const bool prevDiffers = thisNode.getPrevNodeId() !=
                neighNode.getPrevNodeId();
        const bool nextDiffers = thisNode.getNextNodeId() !=
                neighNode.getNextNodeId();
        // if node is normal
        if(thisNode.isNormal()) {
            // if node is in the middle(has both previous and next node)
            if(thisNode.getNextNodeId() && thisNode.getPrevNodeId()) {
                // split node only if both nodes differ
                return prevDiffers && nextDiffers;
            }
        }
        // if node is not normal and in the middle
        // split if previous or next node differs
        return prevDiffers || nextDiffers;
    }

    SkPath getPathFor(SmartPath * const neighbour) const {
        const auto& neighNodes = neighbour->getNodes();
        QList<Node> result = mNodes;

        int iMax = neighNodes.count() - 1;
        if(result.count() - 1 != iMax)
            RuntimeThrow("Nodes count does not match");

        int iShift = 0;
        for(int i = 0; i <= iMax; i++) {
            const int resI = i + iShift;
            Node& resultNode = result[resI];
            const Node& neighbourNode = neighNodes.at(i);

            // Remove nodes is not needed
            if((neighbourNode.isDummy() || neighbourNode.isDissolved()) &&
               (resultNode.isDummy() || resultNode.isDissolved())) {
                iShift--;
                result.removeAt(resI);
            }

            // Create splits for connecting/disconnecting
            if(shouldSplitThisNode(resultNode, neighbourNode)) {
                if(resultNode.isDissolved()) {
                    promoteDissolvedNodeToNormal(resI, result);
                    splitNodeAndDisconnect(resI, result);
                    iShift += 2;
                } else if(resultNode.isNormal()) {
                    splitNode(resultNode, resI, result);
                    iShift++;
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
