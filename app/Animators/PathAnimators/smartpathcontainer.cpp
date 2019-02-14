#include "smartpathcontainer.h"
#include "smartPointers/sharedpointerdefs.h"

void SmartPath::actionRemoveNormalNode(const int &nodeId) {
    Node& node = mNodes->at(nodeId);
    if(!node.isNormal())
        RuntimeThrow("Invalid node type. "
                     "Only normal nodes can be removed.");

    Node * currNode = &node;
    while(currNode->hasPreviousNode()) {
        const int prevId = currNode->getPrevNodeId();
        currNode = &mNodes->at(prevId);
        if(currNode->isNormal() || currNode->isMove()) break;
        if(currNode->isDissolved()) currNode->fT *= 0.5;
    }
    currNode = &node;
    while(currNode->hasNextNode()) {
        const int nextId = currNode->getNextNodeId();
        currNode = &mNodes->at(nextId);
        if(currNode->isNormal() || currNode->isMove()) break;
        if(currNode->isDissolved()) currNode->fT = currNode->fT*0.5 + 0.5;
    }
    mNodes->setNodeType(nodeId, node, Node::DUMMY);
}

void SmartPath::actionAddFirstNode(const QPointF &c0,
                                   const QPointF &p1,
                                   const QPointF &c2) {
    const int insertId = mNodes->count();
    mNodes->insertNodeToList(insertId, Node(c0, p1, c2));
    if(mPrev) mPrev->normalOrMoveNodeInsertedToNeigh(-1, NEXT);
    if(mNext) mNext->normalOrMoveNodeInsertedToNeigh(-1, PREV);
    mNodes->insertNodeAfter(insertId, Node(Node::MOVE));
    if(mPrev) mPrev->normalOrMoveNodeInsertedToNeigh(insertId, NEXT);
    if(mNext) mNext->normalOrMoveNodeInsertedToNeigh(insertId, PREV);
}

void SmartPath::insertNodeBetween(const int& prevId,
                                  const int& nextId,
                                  const Node& nodeBlueprint) {
    if(!mNodes->nodesConnected(prevId, nextId))
        RuntimeThrow("Cannot insert between not connected nodes");
    const int insertId = mNodes->insertNodeAfter(prevId, nodeBlueprint);
    if(nodeBlueprint.isDissolved()) {
        Node& newNode = mNodes->at(insertId);
        mNodes->promoteDissolvedNodeToNormal(insertId, newNode);
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
    mNodes->promoteDissolvedNodeToNormal(nodeId);
}

void SmartPath::actionDisconnectNodes(const int &node1Id, const int &node2Id) {
    Node& node1 = mNodes->at(node1Id);
    Node& node2 = mNodes->at(node2Id);
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
    Node& prevNode = mNodes->at(prevId);
    if(!prevNode.isNormal()) {
        const int prevNormalIdV = mNodes->prevNormalId(prevId);
        Node& prevNormalNode = mNodes->at(prevNormalIdV);
        int currNodeId = prevId;
        while(true) {
            if(currNodeId == -1) break;
            Node& currNode = mNodes->at(currNodeId);
            if(currNode.isNormal() || currNode.isMove()) break;
            const int prevNodeId = currNode.getPrevNodeId();
            moveNodeBefore(currNodeId, currNode,
                           prevNormalIdV, prevNormalNode);
            currNodeId = prevNodeId;
        }
    }
    Node& nextNode = mNodes->at(nextId);
    if(!nextNode.isNormal()) {
        const int nextNormalIdV = mNodes->nextNormalId(nextId);
        Node& nextNormalNode = mNodes->at(nextNormalIdV);
        int currNodeId = nextNormalNode.getPrevNodeId();
        while(true) {
            if(currNodeId == -1) break;
            Node& currNode = mNodes->at(currNodeId);
            if(currNode.isNormal() || currNode.isMove()) break;
            const int nextNodeId = currNode.getPrevNodeId();
            moveNodeAfter(currNodeId, currNode,
                          nextNormalIdV, nextNormalNode);
            currNodeId = nextNodeId;
        }
    }
    mNodes->insertNodeAfter(prevId, Node(Node::MOVE));
    if(mPrev) mPrev->normalOrMoveNodeInsertedToNeigh(prevId, NEXT);
    if(mNext) mNext->normalOrMoveNodeInsertedToNeigh(prevId, PREV);
}

void SmartPath::actionConnectNodes(const int &node1Id,
                                   const int &node2Id) {
    const int moveNode1Id = mNodes->lastSegmentNode(node1Id);
    const int moveNode2Id = mNodes->lastSegmentNode(node2Id);
    if(moveNode1Id == -1 || moveNode2Id == -1)
        RuntimeThrow("Node is not part of a segment");
    // if closing single segment
    if(moveNode1Id == moveNode2Id) {
        Node& moveNode = mNodes->at(moveNode1Id);
        if(!moveNode.isMove())
            RuntimeThrow("Trying to connect a closed segment");
        mNodes->setNodeType(moveNode1Id, moveNode, Node::DUMMY);
        const int firstNodeId = mNodes->firstSegmentNode(node1Id);
        mNodes->setNodeNextId(moveNode1Id, moveNode, firstNodeId);
        mNodes->setNodePrevId(firstNodeId, moveNode1Id);
    } else { // if connecting two seperate segments
        Node& moveNode1 = mNodes->at(moveNode1Id);
        Node& moveNode2 = mNodes->at(moveNode2Id);
        if(!moveNode1.isMove() || !moveNode2.isMove())
            RuntimeThrow("Trying to connect a closed segment");
        RuntimeThrow("Not yet finished");
    }
}

int SmartPath::dissolvedOrDummyNodeInsertedToNeigh(const int &targetNodeId,
                                                   const Neighbour& neigh) {
    int insertId;
    if(targetNodeId < 0 || targetNodeId >= mNodes->count()) {
        insertId = mNodes->count();
        mNodes->insertNodeToList(mNodes->count(), Node());
    } else {
        insertId = mNodes->insertNodeAfter(targetNodeId, Node());
    }
    if((neigh & PREV) && mNext)
        mNext->dissolvedOrDummyNodeInsertedToNeigh(targetNodeId, PREV);
    if((neigh & NEXT) && mPrev)
        mPrev->dissolvedOrDummyNodeInsertedToNeigh(targetNodeId, NEXT);

    return insertId;
}

void SmartPath::normalOrMoveNodeInsertedToNeigh(const int &targetNodeId,
                                                const Neighbour& neigh) {
    dissolvedOrDummyNodeInsertedToNeigh(targetNodeId, neigh);
}

void SmartPath::removeNodeWithIdAndTellPrevToDoSame(const int &nodeId) {
    mNodes->removeNodeFromList(nodeId);
    if(mPrev) mPrev->removeNodeWithIdAndTellPrevToDoSame(nodeId);
}

void SmartPath::removeNodeWithIdAndTellNextToDoSame(const int &nodeId) {
    mNodes->removeNodeFromList(nodeId);
    if(mNext) mNext->removeNodeWithIdAndTellNextToDoSame(nodeId);
}

void SmartPath::setPrev(SmartPath * const prev) {
    mPrev = prev;
    if(mPrev) mNodes->setNext(mNext->getNodes());
    else mNodes->setNext(nullptr);
}

void SmartPath::setNext(SmartPath * const next) {
    mNext = next;
    if(mNext) mNodes->setNext(mNext->getNodes());
    else mNodes->setNext(nullptr);
}

NodeList *SmartPath::getNodes() const {
    return mNodes.get();
}

SkPath SmartPath::getPathAt() const {
    return mNodes->toSkPath();
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

SmartPath::SmartPath(const QList<Node> &nodes) {}

SkPath SmartPath::getPathForPrev() const {
    if(mPrev) return getPathFor(mPrev);
    return mNodes->toSkPath();
}

SkPath SmartPath::getPathForNext() const {
    if(mNext) return getPathFor(mNext);
    return mNodes->toSkPath();
}

SkPath SmartPath::getPathFor(SmartPath * const neighbour) const {
    const auto& neighNodes = neighbour->getNodes();
    auto result = SPtrCreate(NodeList)(mNodes);

    int iMax = neighNodes->count() - 1;
    if(result->count() - 1 != iMax)
        RuntimeThrow("Nodes count does not match");

    int iShift = 0;
    for(int i = 0; i <= iMax; i++) {
        const int resI = i + iShift;
        Node& resultNode = result->at(resI);
        const Node& neighbourNode = neighNodes->at(i);
        const Node& thisNode = mNodes->at(i);

        // Remove nodes if not needed
        if((neighbourNode.isDummy() || neighbourNode.isDissolved()) &&
                (thisNode.isDummy() || thisNode.isDissolved())) {
            iShift--;
            result->removeNodeFromList(resI);
        }

        // Create splits for connecting/disconnecting
        if(mNodes->shouldSplitThisNode(i, thisNode, neighbourNode,
                                       mNodes.get(), neighNodes)) {
            if(thisNode.isDissolved()) {
                result->promoteDissolvedNodeToNormal(resI);
                result->splitNodeAndDisconnect(resI);
                iShift += 2;
            } else if(resultNode.isNormal()) {
                result->splitNode(resI);
                iShift++;
            }
        }
    }
    return result->toSkPath();
}
