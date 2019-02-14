#include "smartpathcontainer.h"

void SmartPath::actionRemoveNormalNode(const int &nodeId) {
    Node& node = mNodes[nodeId];
    if(!node.isNormal())
        RuntimeThrow("Invalid node type. "
                     "Only normal nodes can be removed.");

    Node * currNode = &node;
    while(currNode->hasPreviousNode()) {
        const int prevId = currNode->getPrevNodeId();
        currNode = &mNodes[prevId];
        if(currNode->isNormal() || currNode->isMove()) break;
        if(currNode->isDissolved()) currNode->fT *= 0.5;
    }
    currNode = &node;
    while(currNode->hasNextNode()) {
        const int nextId = currNode->getNextNodeId();
        currNode = &mNodes[nextId];
        if(currNode->isNormal() || currNode->isMove()) break;
        if(currNode->isDissolved()) currNode->fT = currNode->fT*0.5 + 0.5;
    }
    node.setType(Node::DUMMY);
    if(mPrev) mPrev->updateNodeTypeAfterNeighbourChanged(nodeId);
    if(mNext) mNext->updateNodeTypeAfterNeighbourChanged(nodeId);
    updateNodeTypeAfterNeighbourChanged(nodeId);
}

void SmartPath::actionAddFirstNode(const QPointF &c0,
                                   const QPointF &p1,
                                   const QPointF &c2) {
    const int insertId = mNodes.count();
    gInsertNodeToList(insertId, Node(c0, p1, c2), mNodes);
    if(mPrev) mPrev->normalOrMoveNodeInsertedToNeigh(-1, NEXT);
    if(mNext) mNext->normalOrMoveNodeInsertedToNeigh(-1, PREV);
    gInsertNodeAfter(insertId, Node(Node::MOVE), mNodes);
    if(mPrev) mPrev->normalOrMoveNodeInsertedToNeigh(insertId, NEXT);
    if(mNext) mNext->normalOrMoveNodeInsertedToNeigh(insertId, PREV);
}

void SmartPath::insertNodeBetween(const int& prevId,
                                  const int& nextId,
                                  const Node& nodeBlueprint) {
    if(!gNodesConnected(prevId, nextId, mNodes))
        RuntimeThrow("Cannot insert between not connected nodes");
    const int insertId = gInsertNodeAfter(prevId, nodeBlueprint, mNodes);
    if(nodeBlueprint.isDissolved()) {
        Node& newNode = mNodes[insertId];
        gPromoteDissolvedNodeToNormal(insertId, newNode, mNodes);
    }

    if(mPrev) mPrev->normalOrMoveNodeInsertedToNeigh(prevId, NEXT);
    if(mNext) mNext->normalOrMoveNodeInsertedToNeigh(prevId, PREV);
}

void SmartPath::actionInsertNodeBetween(const int &prevId,
                                        const int& nextId,
                                        const qreal& t) {
    insertNodeBetween(prevId, nextId, Node(t));
}

void SmartPath::actionInsertNodeBetween(const int &prevId,
                                        const int& nextId,
                                        const QPointF &c0,
                                        const QPointF &p1,
                                        const QPointF &c2) {
    insertNodeBetween(prevId, nextId, Node(c0, p1, c2));
}

void SmartPath::actionPromoteDissolvedNodeToNormal(const int &nodeId) {
    gPromoteDissolvedNodeToNormal(nodeId, mNodes);
    if(mPrev) mPrev->updateNodeTypeAfterNeighbourChanged(nodeId);
    if(mNext) mNext->updateNodeTypeAfterNeighbourChanged(nodeId);
}

void SmartPath::moveNodeAfter(const int& moveNodeId, Node& moveNode,
                              const int& afterNodeId, Node& afterNode) {
    const int movePrevId = moveNode.getPrevNodeId();
    const int moveNextId = moveNode.getNextNodeId();
    setNodeNextId(movePrevId, moveNextId);
    setNodePrevId(moveNextId, movePrevId);

    const int afterNextId = afterNode.getNextNodeId();
    setNodeNextId(afterNodeId, afterNode, moveNodeId);
    setNodePrevAndNextId(moveNodeId, moveNode, afterNodeId, afterNextId);
    setNodePrevId(afterNextId, moveNodeId);
}

void SmartPath::moveNodeBefore(const int& moveNodeId, Node& moveNode,
                               const int& beforeNodeId, Node& beforeNode) {
    const int movePrevId = moveNode.getPrevNodeId();
    const int moveNextId = moveNode.getNextNodeId();
    setNodeNextId(movePrevId, moveNextId);
    setNodePrevId(moveNextId, movePrevId);

    const int beforePrevId = beforeNode.getPrevNodeId();
    setNodePrevId(beforeNodeId, beforeNode, moveNodeId);
    setNodePrevAndNextId(moveNodeId, moveNode, beforePrevId, beforeNodeId);
    setNodeNextId(beforePrevId, moveNodeId);
}

void SmartPath::actionDisconnectNodes(const int &node1Id, const int &node2Id) {
    Node& node1 = mNodes[node1Id];
    Node& node2 = mNodes[node2Id];
    if(node1.isMove() || node2.isMove())
        RuntimeThrow("Cannot disconnect move node");
    int nextId;
    int prevId;
    if(node1.getNextNodeId() == node2Id) {
        nextId = node2Id;
        prevId = node1Id;
    } else if(node2.getNextNodeId() == node1Id) {
        nextId = node1Id;
        prevId = node2Id;
    } else {
        RuntimeThrow("Trying to disconnect not connected nodes");
    }
    Node& prevNode = mNodes[prevId];
    if(!prevNode.isNormal()) {
        const int prevNormalIdV = gPrevNormalId(prevId, mNodes);
        Node& prevNormalNode = mNodes[prevNormalIdV];
        int currNodeId = prevId;
        while(true) {
            if(currNodeId == -1) break;
            Node& currNode = mNodes[currNodeId];
            if(currNode.isNormal() || currNode.isMove()) break;
            const int prevNodeId = currNode.getPrevNodeId();
            moveNodeBefore(currNodeId, currNode,
                           prevNormalIdV, prevNormalNode);
            currNodeId = prevNodeId;
        }
    }
    Node& nextNode = mNodes[nextId];
    if(!nextNode.isNormal()) {
        const int nextNormalIdV = gNextNormalId(nextId, mNodes);
        Node& nextNormalNode = mNodes[nextNormalIdV];
        int currNodeId = nextNormalNode.getPrevNodeId();
        while(true) {
            if(currNodeId == -1) break;
            Node& currNode = mNodes[currNodeId];
            if(currNode.isNormal() || currNode.isMove()) break;
            const int nextNodeId = currNode.getPrevNodeId();
            moveNodeAfter(currNodeId, currNode,
                          nextNormalIdV, nextNormalNode);
            currNodeId = nextNodeId;
        }
    }
    gInsertNodeAfter(prevId, Node(Node::MOVE), mNodes);
    if(mPrev) mPrev->normalOrMoveNodeInsertedToNeigh(prevId, NEXT);
    if(mNext) mNext->normalOrMoveNodeInsertedToNeigh(prevId, PREV);
}

void SmartPath::actionConnectNodes(const int &node1Id,
                                   const int &node2Id) {
    const int moveNode1Id = gLastSegmentNode(node1Id, mNodes);
    const int moveNode2Id = gLastSegmentNode(node2Id, mNodes);
    if(moveNode1Id == -1 || moveNode2Id == -1)
        RuntimeThrow("Node is not part of a segment");
    // if closing single segment
    if(moveNode1Id == moveNode2Id) {
        Node& moveNode = mNodes[moveNode1Id];
        if(!moveNode.isMove())
            RuntimeThrow("Trying to connect a closed segment");
        moveNode.setType(Node::DUMMY);
        const int firstNodeId = gFirstSegmentNode(node1Id, mNodes);
        Node& firstNode = mNodes[firstNodeId];
        moveNode.setNextNodeId(firstNodeId);
        firstNode.setPrevNodeId(moveNode1Id);
    } else { // if connecting two seperate segments
        Node& moveNode1 = mNodes[moveNode1Id];
        Node& moveNode2 = mNodes[moveNode2Id];
        if(!moveNode1.isMove() || !moveNode2.isMove())
            RuntimeThrow("Trying to connect a closed segment");
        if(mPrev) mPrev->updateNodeTypeAfterNeighbourChanged(moveNode1Id);
        if(mNext) mNext->updateNodeTypeAfterNeighbourChanged(moveNode1Id);
        RuntimeThrow("Not yet finished");
    }
    if(mPrev) mPrev->updateNodeTypeAfterNeighbourChanged(moveNode2Id);
    if(mNext) mNext->updateNodeTypeAfterNeighbourChanged(moveNode2Id);
}

int SmartPath::dissolvedOrDummyNodeInsertedToNeigh(const int &targetNodeId,
                                                   const Neighbour& neigh) {
    int insertId;
    if(targetNodeId < 0 || targetNodeId >= mNodes.count()) {
        insertId = mNodes.count();
        gInsertNodeToList(mNodes.count(), Node(), mNodes);
    } else {
        insertId = gInsertNodeAfter(targetNodeId, Node(), mNodes);
    }
    if((neigh & PREV) && mNext)
        mNext->dissolvedOrDummyNodeInsertedToNeigh(targetNodeId, PREV);
    if((neigh & NEXT) && mPrev)
        mPrev->dissolvedOrDummyNodeInsertedToNeigh(targetNodeId, NEXT);

    return insertId;
}

void SmartPath::normalOrMoveNodeInsertedToNeigh(const int &targetNodeId,
                                                const Neighbour& neigh) {
    const int insertId = dissolvedOrDummyNodeInsertedToNeigh(
                targetNodeId, neigh);
    updateNodeTypeAfterNeighbourChanged(insertId);
}

void SmartPath::removeNodeWithIdAndTellPrevToDoSame(const int &nodeId) {
    gRemoveNodeFromList(nodeId, mNodes);
    if(mPrev) mPrev->removeNodeWithIdAndTellPrevToDoSame(nodeId);
}

void SmartPath::removeNodeWithIdAndTellNextToDoSame(const int &nodeId) {
    gRemoveNodeFromList(nodeId, mNodes);
    if(mNext) mNext->removeNodeWithIdAndTellNextToDoSame(nodeId);
}

const Node::Type &SmartPath::nodeType(const int &nodeId) const {
    return mNodes.at(nodeId).getType();
}

bool SmartPath::isNodeNormal(const int &nodeId) const {
    return mNodes.at(nodeId).isNormal();
}

bool SmartPath::isNodeDissolved(const int &nodeId) const {
    return nodeType(nodeId) == Node::DISSOLVED;
}

bool SmartPath::isNodeDummy(const int &nodeId) const {
    return nodeType(nodeId) == Node::DUMMY;
}

QList<int> SmartPath::updateAllNodesTypeAfterNeighbourChanged() {
    QList<int> changed;
    for(int i = 0; i < mNodes.count(); i++) {
        if(updateNodeTypeAfterNeighbourChanged(i)) changed << i;
    }
    return changed;
}

bool SmartPath::updateNodeTypeAfterNeighbourChanged(const int &nodeId) {
    Node& node = mNodes[nodeId];
    if(node.isNormal() || node.isMove()) return false;
    Node::Type prevType = Node::DUMMY;
    Node::Type nextType = Node::DUMMY;
    int prevNextId = -1;
    int prevPrevId = -1;
    if(mPrev) {
        const Node& prevNode = mPrev->getNodes().at(nodeId);
        prevType = prevNode.getType();
        prevNextId = prevNode.getNextNodeId();
        prevPrevId = prevNode.getPrevNodeId();
    }
    int nextNextId = -1;
    int nextPrevId = -1;
    if(mNext) {
        const Node& nextNode = mNext->getNodes().at(nodeId);
        nextType = nextNode.getType();
        nextNextId = nextNode.getNextNodeId();
        nextPrevId = nextNode.getPrevNodeId();
    }
    const int nodeNextId = node.getNextNodeId();
    if(prevType == Node::NORMAL || nextType == Node::NORMAL ||
            prevType == Node::MOVE || nextType == Node::MOVE ||
            ((nodeNextId != nextNextId && nodeNextId != nextPrevId) && nextType != Node::DUMMY) ||
            ((nodeNextId != prevNextId && nodeNextId != prevPrevId) && prevType != Node::DUMMY)) {
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

void SmartPath::setPrev(SmartPath * const prev) {
    mPrev = prev;
    updateAllNodesTypeAfterNeighbourChanged();
}

void SmartPath::setNext(SmartPath * const next) {
    mNext = next;
    updateAllNodesTypeAfterNeighbourChanged();
}

const QList<Node> &SmartPath::getNodes() const {
    return mNodes;
}

SkPath SmartPath::getPathAt() const {
    return gNodesToSkPath(mNodes);
}

SkPath SmartPath::interpolateWithNext(const qreal &nextWeight) const {
    if(!mNext) return getPathAt();
    SkPath result;
    getPathForNext().interpolate(mNext->getPathForPrev(),
                                 qrealToSkScalar(nextWeight),
                                 &result);
    return result;
}

SkPath SmartPath::interpolateWithPrev(const qreal &nextWeight) const {
    if(!mPrev) return getPathAt();
    SkPath result;
    getPathForPrev().interpolate(mPrev->getPathForNext(),
                                 qrealToSkScalar(nextWeight),
                                 &result);
    return result;
}

SmartPath::SmartPath() {}

SmartPath::SmartPath(const QList<Node> &nodes) :
    mNodes(nodes) {}

SkPath SmartPath::getPathForPrev() const {
    if(mPrev) return getPathFor(mPrev);
    return gNodesToSkPath(mNodes);
}

SkPath SmartPath::getPathForNext() const {
    if(mNext) return getPathFor(mNext);
    return gNodesToSkPath(mNodes);
}

SkPath SmartPath::getPathFor(SmartPath * const neighbour) const {
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
        const Node& thisNode = mNodes.at(i);

        // Remove nodes if not needed
        if((neighbourNode.isDummy() || neighbourNode.isDissolved()) &&
                (thisNode.isDummy() || thisNode.isDissolved())) {
            iShift--;
            gRemoveNodeFromList(resI, result);
        }

        // Create splits for connecting/disconnecting
        if(gShouldSplitThisNode(i, thisNode, neighbourNode,
                               mNodes, neighNodes)) {
            if(thisNode.isDissolved()) {
                gPromoteDissolvedNodeToNormal(resI, result);
                gSplitNodeAndDisconnect(resI, result);
                iShift += 2;
            } else if(resultNode.isNormal()) {
                gSplitNode(resI, result);
                iShift++;
            }
        }
    }
    return gNodesToSkPath(result);
}
