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

    Node(const qreal& t) {
        fT = t;
        mType = DISSOLVED;
    }

    bool isMove() const { return mType == MOVE; }

    bool isNormal() const { return mType == NORMAL; }

    bool isDummy() const { return mType == DUMMY; }

    bool isDissolved() const { return mType == DISSOLVED; }

    void switchPrevAndNext() {
        const int prevT = mPrevNodeId;
        mPrevNodeId = mNextNodeId;
        mNextNodeId = prevT;
    }

    int getNextNodeId() const {
        if(isDummy() || isMove()) return -1;
        return mNextNodeId;
    }
    int getPrevNodeId() const {
        if(isDummy() || isMove()) return -1;
        return mPrevNodeId;
    }

    bool hasPreviousNode() const {
        return getPrevNodeId() >= 0;
    }

    bool hasNextNode() const {
        return getNextNodeId() >= 0;
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

int firstSegmentNode(const int& nodeId, const QList<Node>& nodes) {
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

int lastSegmentNode(const int& nodeId, const QList<Node>& nodes) {
    if(nodeId < 0) return -1;
    if(nodeId >= nodes.count()) return -1;
    const Node * currNode = &nodes.at(nodeId);
    if(currNode->isDummy() || currNode->isMove()) return -1;
    int smallestId = nodeId;
    int currId = nodeId;
    while(true) {
        if(currNode->isDummy())
            RuntimeThrow("Dummy used as a next node(should be skipped)");
        //if(currNode->isDissolved() || currNode->isNormal())
        const int nextId = currNode->getNextNodeId();
        if(nextId == currId)
            RuntimeThrow("Node points to itself");
        if(!nextId) return nodeId;
        if(nextId == smallestId) return nodeId;
        smallestId = qMin(nextId, smallestId);
        currNode = &nodes.at(nextId);
        currId = nextId;
    }
}

int nodesInSameSagment(const int& node1Id,
                       const int& node2Id,
                       const QList<Node>& nodes) {
    const int firstSegment1 = firstSegmentNode(node1Id, nodes);
    const int firstSegment2 = firstSegmentNode(node2Id, nodes);
    return firstSegment1 && firstSegment1 == firstSegment2;
}

void reverseSegment(const int& nodeId, QList<Node>& nodes) {
    const int firstNodeId = firstSegmentNode(nodeId, nodes);
    if(!firstNodeId) return;
    Node& firstNode = nodes[firstNodeId];
    int nextSegId = firstNode.getNextNodeId();
    firstNode.switchPrevAndNext();
    while(const int currNodeId = nextSegId) {
        if(currNodeId == firstNodeId) break;
        Node& currNode = nodes[nextSegId];
        nextSegId = currNode.getNextNodeId();
        currNode.switchPrevAndNext();
    }
}

bool segmentClosed(const int& nodeId, const QList<Node>& nodes) {
    const int firstNodeId = firstSegmentNode(nodeId, nodes);
    if(!firstNodeId) return false;
    return nodes.at(firstNodeId).hasPreviousNode();
}

QList<Node> sortNodeListAccoringToConnetions(const QList<Node>& srcList) {
    QList<Node> result;
    QList<int> srcIds;
    for(int i = 0; i < srcList.count(); i++) {
        srcIds << i;
    }
    while(!srcIds.isEmpty()) {
        const int firstSrcId = firstSegmentNode(srcIds.first(), srcList);
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

void cubicTo(const Node& prevNode, const Node& nextNode,
             QList<qreal>& dissolvedTs, SkPath& result) {
    qCubicSegment2D seg(prevNode.fP1, prevNode.fC2,
                        nextNode.fC0, nextNode.fP1);
    qreal lastT = 0;
    for(const qreal& t : dissolvedTs) {
        const qreal mappedT = gMapTToFragment(lastT, 1, t);
        auto div = seg.dividedAtT(mappedT);
        const auto& first = div.first;
        result.cubicTo(qPointToSk(first.c1()),
                       qPointToSk(first.c2()),
                       qPointToSk(first.p1()));
        seg = div.second;
        lastT = t;
    }
    result.cubicTo(qPointToSk(seg.c1()),
                   qPointToSk(seg.c2()),
                   qPointToSk(seg.p1()));
    dissolvedTs.clear();
}

SkPath nodesToSkPath(const QList<Node>& nodes) {
    const auto sortedNodes = sortNodeListAccoringToConnetions(nodes);
    SkPath result;
    bool move = true;

    bool close = false;
    const Node * firstNode = nullptr;
    const Node * prevNormalNode = nullptr;

    QList<qreal> dissolvedTs;

    for(int i = 0; i < sortedNodes.count(); i++) {
        const Node& node = sortedNodes.at(i);
        if(node.isDummy()) continue;
        else if(node.isDissolved()) dissolvedTs << node.fT;
        else if(node.isMove()) move = true;
        else if(node.isNormal()) {
            if(move) {
                if(close) cubicTo(*prevNormalNode, *firstNode,
                                  dissolvedTs, result);
                firstNode = &node;
                close = firstNode->getPrevNodeId();
                result.moveTo(qPointToSk(node.fP1));
                move = false;
            } else {
                cubicTo(*prevNormalNode, node,
                        dissolvedTs, result);
            }
            prevNormalNode = &node;
        } else {
            RuntimeThrow("Unrecognized node type");
        }
    } // for each node
    if(close) {
        cubicTo(*prevNormalNode, *firstNode,
                dissolvedTs, result);
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

    void actionRemoveNormalNode(const int& nodeId) {
        Node& node = mNodes[nodeId];
        if(!node.isNormal())
            RuntimeThrow("Invalid node type. "
                         "Only normal nodes can be removed.");

        int nextPrevId = node.getPrevNodeId();
        while(const int prevId = nextPrevId) {
            Node &prevNode = mNodes[prevId];
            if(prevNode.isNormal()) break;
            prevNode.fT *= 0.5;
            nextPrevId = prevNode.getPrevNodeId();
        }
        int nextNextId = node.getNextNodeId();
        while(const int nextId = nextNextId) {
            Node &nextNode = mNodes[nextId];
            if(nextNode.isNormal()) break;
            nextNode.fT = nextNode.fT*0.5 + 0.5;
            nextPrevId = nextNode.getNextNodeId();
        }
        node.setType(Node::DUMMY);
        if(mPrev) mPrev->updateNodeTypeAfterNeighbourChanged(nodeId);
        if(mNext) mNext->updateNodeTypeAfterNeighbourChanged(nodeId);
        updateNodeTypeAfterNeighbourChanged(nodeId);
    }

    void actionAddFirstNode(const QPointF& c0,
                            const QPointF& p1,
                            const QPointF& c2) {
        int insertId = mNodes.count();
        if(!mNodes.isEmpty()) {
            insertNodeToList(insertId, Node(Node::MOVE));
            if(mPrev) mPrev->normalOrMoveNodeInsertedToNext(insertId);
            if(mNext) mNext->normalOrMoveNodeInsertedToPrev(insertId);
            insertId++;
        }
        insertNodeToList(insertId, Node(c0, p1, c2));
        if(mPrev) mPrev->normalOrMoveNodeInsertedToNext(insertId);
        if(mNext) mNext->normalOrMoveNodeInsertedToPrev(insertId);
    }

    void actionInsertNormalNode(const int& nodeId, const qreal& t) {
        Node &node = insertNodeToList(nodeId, Node(t));
        promoteDissolvedNodeToNormal(nodeId, node, mNodes);
        if(mPrev) mPrev->normalOrMoveNodeInsertedToNext(nodeId);
        if(mNext) mNext->normalOrMoveNodeInsertedToPrev(nodeId);
    }

    void actionAddNormalNodeAtEnd(const int& nodeId,
                                  const QPointF& c0,
                                  const QPointF& p1,
                                  const QPointF& c2) {
        Node& endNode = mNodes[nodeId];
        bool isNext;
        if(!endNode.getNextNodeId()) {
            endNode.setNextNodeId(nodeId);
            isNext = true;
        } else if(!endNode.getPrevNodeId()) {
            endNode.setPrevNodeId(nodeId);
            isNext = false;
        } else RuntimeThrow("Assumption not met: Node is not and end node.");
        const int insertId = isNext ? nodeId + 1 : nodeId;
        Node& newNode = insertNodeToList(insertId, Node(c0, p1, c2));
        if(isNext) {
            newNode.setPrevNodeId(nodeId);
        } else {
            newNode.setNextNodeId(nodeId + 1);
        }
        if(mPrev) mPrev->normalOrMoveNodeInsertedToNext(insertId);
        if(mNext) mNext->normalOrMoveNodeInsertedToPrev(insertId);
    }

    void actionPromoteDissolvedNodeToNormal(const int& nodeId) {
        promoteDissolvedNodeToNormal(nodeId, mNodes);
        if(mPrev) mPrev->updateNodeTypeAfterNeighbourChanged(nodeId);
        if(mNext) mNext->updateNodeTypeAfterNeighbourChanged(nodeId);
    }

    void actionDisconnectNodes(const int& node1Id, const int& node2Id) {
        Node& node1 = mNodes[node1Id];
        Node& node2 = mNodes[node2Id];
        int moveInsertId;
        if(node1.getNextNodeId() == node2Id) {
            node1.setNextNodeId(-1);
            node2.setPrevNodeId(-1);
            moveInsertId = node1Id + 1;
        } else if(node2.getNextNodeId() == node1Id) {
            node1.setPrevNodeId(-1);
            node2.setNextNodeId(-1);
            moveInsertId = node2Id + 1;
        } else {
            RuntimeThrow("Trying to disconnect not connected nodes");
        }
        insertNodeToList(moveInsertId, Node(Node::MOVE));
        if(mPrev) mPrev->normalOrMoveNodeInsertedToNext(moveInsertId);
        if(mNext) mNext->normalOrMoveNodeInsertedToPrev(moveInsertId);
    }

    void actionConnectNodes(const int& node1Id, const int& node2Id) {
        Node& node1 = mNodes[node1Id];
        Node& node2 = mNodes[node2Id];
        if(!node1.hasNextNode() && !node2.hasPreviousNode()) {
            node1.setNextNodeId(node2Id);
            node2.setPrevNodeId(node1Id);
        } else if(!node1.hasPreviousNode() && !node2.hasNextNode()) {
            node1.setPrevNodeId(node2Id);
            node2.setNextNodeId(node1Id);
        } else if(!node1.hasPreviousNode() && !node2.hasPreviousNode()) {
            reverseSegment(node1Id, mNodes);
            node1.setNextNodeId(node2Id);
            node2.setPrevNodeId(node1Id);
        } else if(!node1.hasNextNode() && !node2.hasNextNode()) {
            reverseSegment(node1Id, mNodes);
            node1.setPrevNodeId(node2Id);
            node2.setNextNodeId(node1Id);
        } else {
            RuntimeThrow("Trying to connect nodes "
                         "that already have two connections");
        }
    }

    void normalOrMoveNodeInsertedToPrev(const int& insertedNodeId) {
        insertNodeToList(insertedNodeId, Node());
        updateNodeTypeAfterNeighbourChanged(insertedNodeId);
        if(mNext) mNext->dissolvedOrDummyNodeInsertedToPrev(insertedNodeId);
    }

    void dissolvedOrDummyNodeInsertedToPrev(const int& insertedNodeId) {
        insertNodeToList(insertedNodeId, Node());
        if(mNext) mNext->dissolvedOrDummyNodeInsertedToPrev(insertedNodeId);
    }

    void normalOrMoveNodeInsertedToNext(const int& insertedNodeId) {
        insertNodeToList(insertedNodeId, Node());
        updateNodeTypeAfterNeighbourChanged(insertedNodeId);
        if(mPrev) mPrev->dissolvedOrDummyNodeInsertedToNext(insertedNodeId);
    }

    void dissolvedOrDummyNodeInsertedToNext(const int& insertedNodeId) {
        insertNodeToList(insertedNodeId, Node());
        if(mPrev) mPrev->dissolvedOrDummyNodeInsertedToNext(insertedNodeId);
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

    qreal prevT(const int& nodeId) const {
        const int &prevId = prevNonDummyId(nodeId, mNodes);
        const Node& node = mNodes.at(prevId);
        if(node.isNormal()) return 0;
        return node.fT;
    }

    qreal nextT(const int& nodeId) const {
        const int &nextId = nextNonDummyId(nodeId, mNodes);
        const Node& node = mNodes.at(nextId);
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
protected:
    SmartPath() {}
    SmartPath(SmartPath * const prev,
              SmartPath * const next,
              const QList<Node>& nodes) :
        mPrev(prev), mNext(next), mNodes(nodes) {}
private:
    int prevNormalId(const int& nodeId, const QList<Node>& nodes) const {
        const Node * currNode = &nodes.at(nodeId);
        int nextNodeId = currNode->getPrevNodeId();
        while(nextNodeId) {
            currNode = &nodes.at(nextNodeId);
            if(currNode->isNormal()) return nextNodeId;
            nextNodeId = currNode->getPrevNodeId();
        }
        return -1;
    }

    int nextNormalId(const int& nodeId, const QList<Node>& nodes) const {
        const Node * currNode = &nodes.at(nodeId);
        int nextNodeId = currNode->getNextNodeId();
        while(nextNodeId) {
            currNode = &nodes.at(nextNodeId);
            if(currNode->isNormal()) return nextNodeId;
            nextNodeId = currNode->getNextNodeId();
        }
        return -1;
    }

    int prevNonDummyId(const int& nodeId, const QList<Node>& nodes) const {
        const Node * currNode = &nodes.at(nodeId);
        int nextNodeId = currNode->getPrevNodeId();
        while(nextNodeId) {
            currNode = &nodes.at(nextNodeId);
            if(!currNode->isDummy()) return nextNodeId;
            nextNodeId = currNode->getPrevNodeId();
        }
        return -1;
    }

    int nextNonDummyId(const int& nodeId, const QList<Node>& nodes) const {
        const Node * currNode = &nodes.at(nodeId);
        int nextNodeId = currNode->getNextNodeId();
        while(nextNodeId) {
            currNode = &nodes.at(nextNodeId);
            if(!currNode->isDummy()) return nextNodeId;
            nextNodeId = currNode->getNextNodeId();
        }
        return -1;
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

    void splitNodeAndDisconnect(const int& nodeId, QList<Node>& nodes) const {
        Node& node = nodes[nodeId];
        if(!node.isNormal())
            RuntimeThrow("Can only disconnect normal nodes.");
        splitNode(node, nodeId, nodes);
        nodes.insert(nodeId + 1, Node(Node::MOVE));
    }

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

            // Remove nodes if not needed
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

    stdptr<SmartPath> mPrev;
    stdptr<SmartPath> mNext;
    QList<Node> mNodes;
};

#endif // SMARTPATHCONTAINER_H
