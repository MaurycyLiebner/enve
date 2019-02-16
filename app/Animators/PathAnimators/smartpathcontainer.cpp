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
    const int insertId = mNodes->appendNode(Node(c0, p1, c2));
    mNodes->insertNodeAfter(insertId, Node(Node::MOVE));
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
    int prevId;
    int nextId;
    if(node1.getNextNodeId() == node2Id) {
        prevId = node1Id;
        nextId = node2Id;
    } else if(node2.getNextNodeId() == node1Id) {
        prevId = node2Id;
        nextId = node1Id;
    } else {
        RuntimeThrow("Trying to disconnect not connected nodes");
    }
    mNodes->insertNodeAfter(prevId, Node(Node::MOVE));
    if(nextId > prevId) nextId++;

    Node& prevNode = mNodes->at(prevId);
    if(!prevNode.isNormal()) {
        const int prevNormalIdV = mNodes->prevNormalId(prevId);
        Node& prevNormalNode = mNodes->at(prevNormalIdV);
        mNodes->setNodeType(prevId, prevNode, Node::NORMAL);
        prevNode.fC0 = prevNormalNode.fC0;
        prevNode.fP1 = prevNormalNode.fP1;
        prevNode.fC2 = prevNormalNode.fC2;

        int currNodeId = prevNode.getPrevNodeId();
        while(true) {
            if(currNodeId == -1) break;
            Node& currNode = mNodes->at(currNodeId);
            if(currNode.isNormal() || currNode.isMove()) break;
            mNodes->setNodeType(currNodeId, currNode, Node::DUMMY);
            currNodeId = currNode.getPrevNodeId();
        }
        mNodes->setNodeType(prevNormalIdV, prevNormalNode, Node::DUMMY);
    }
    Node& nextNode = mNodes->at(nextId);
    if(!nextNode.isNormal()) {
        const int nextNormalIdV = mNodes->nextNormalId(nextId);
        Node& nextNormalNode = mNodes->at(nextNormalIdV);
        mNodes->setNodeType(nextId, nextNode, Node::NORMAL);
        nextNode.fC0 = nextNormalNode.fC0;
        nextNode.fP1 = nextNormalNode.fP1;
        nextNode.fC2 = nextNormalNode.fC2;

        int currNodeId = nextNode.getNextNodeId();
        while(true) {
            if(currNodeId == -1) break;
            Node& currNode = mNodes->at(currNodeId);
            if(currNode.isNormal() || currNode.isMove()) break;
            mNodes->setNodeType(currNodeId, currNode, Node::DUMMY);
            currNodeId = currNode.getNextNodeId();
        }
        mNodes->setNodeType(nextNormalIdV, nextNormalNode, Node::DUMMY);
    }
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
        const int firstNodeId = mNodes->firstSegmentNode(node1Id);
        mNodes->setNodeType(moveNode1Id, moveNode, Node::DUMMY);
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
    if(mPrev) mNodes->setPrev(mPrev->getNodes());
    else mNodes->setPrev(nullptr);
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
    const SkPath thisPath = getPathForNext();
    const SkPath nextPath = mNext->getPathForPrev();
    const bool ok = thisPath.interpolate(nextPath,
                                         qrealToSkScalar(nextWeight),
                                         &result);
    if(!ok) RuntimeThrow("Cannot interpolate paths with different verb count");
    return result;
}

SkPath SmartPath::interpolateWithPrev(const qreal &prevWeight) const {
    if(!mPrev) return getPathAt();
    SkPath result;
    const SkPath thisPath = getPathForPrev();
    const SkPath prevPath = mPrev->getPathForNext();
    const bool ok = thisPath.interpolate(prevPath,
                                         qrealToSkScalar(prevWeight),
                                         &result);
    if(!ok) RuntimeThrow("Cannot interpolate paths with different verb count");
    return result;
}

SmartPath::SmartPath() {
    mNodes = SPtrCreate(NodeList)();
}

SmartPath::SmartPath(const NodeList * const nodes) {
    mNodes = SPtrCreate(NodeList)(nodes);
}

SkPath SmartPath::getPathForPrev() const {
    if(mPrev) return getPathFor(mPrev);
    return mNodes->toSkPath();
}

SkPath SmartPath::getPathForNext() const {
    if(mNext) return getPathFor(mNext);
    return mNodes->toSkPath();
}

bool isEndPointAndShouldBeSplit(const int& nodeId,
                         const Node& thisNode,
                         const Node& neighNode,
                         const NodeList * const thisNodes,
                         const NodeList * const neighNodes) {
    if(!thisNodes->segmentClosed(nodeId)) {
        const int thisLastId = thisNodes->lastSegmentNode(nodeId);
        const int neighLastId = neighNodes->lastSegmentNode(nodeId);
        if(thisLastId == thisNode.getNextNodeId() &&
           neighLastId != neighNode.getNextNodeId()) return true;
        const int thisFirstId = thisNodes->firstSegmentNode(nodeId);
        const int neighFirstId = neighNodes->firstSegmentNode(nodeId);
        if(thisFirstId == nodeId && neighFirstId != nodeId) return true;
    }
    return false;
}

bool shouldSplitThisNode(const int& nodeId,
                         const Node& thisNode,
                         const Node& neighNode,
                         const NodeList * const thisNodes,
                         const NodeList * const neighNodes) {
    if(thisNode.isNormal()) {
        return isEndPointAndShouldBeSplit(nodeId, thisNode, neighNode,
                                          thisNodes, neighNodes);
    } else if(thisNode.isDissolved()) {
        return isEndPointAndShouldBeSplit(nodeId, neighNode, thisNode,
                                          neighNodes, thisNodes);
    }
    return false;
}

SkPath SmartPath::getPathFor(SmartPath * const neighbour) const {
    const auto& neighNodes = neighbour->getNodes();
    auto result = SPtrCreate(NodeList)(mNodes.get(), true);

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
        if(shouldSplitThisNode(i, thisNode, neighbourNode,
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
