#include "smartpathcontainer.h"

SmartPath::SmartPath() :
  mNodesList(false) {}

SmartPath::SmartPath(const SmartPath &other) :
    SmartPath() {
    mNodesList.setNodeList(other.getNodesRef().getList());
}

void SmartPath::actionRemoveNormalNode(const int &nodeId) {
    Node * const node = mNodesList.at(nodeId);
    if(!node->isNormal())
        RuntimeThrow("Invalid node type. "
                     "Only normal nodes can be removed.");

    Node * currNode = node;
    while(currNode->hasPreviousNode()) {
        const int prevId = currNode->getPrevNodeId();
        currNode = mNodesList.at(prevId);
        if(currNode->isNormal() || currNode->isMove()) break;
        if(currNode->isDissolved()) currNode->fT *= 0.5;
    }
    currNode = node;
    while(currNode->hasNextNode()) {
        const int nextId = currNode->getNextNodeId();
        currNode = mNodesList.at(nextId);
        if(currNode->isNormal() || currNode->isMove()) break;
        if(currNode->isDissolved()) currNode->fT = currNode->fT*0.5 + 0.5;
    }
    mNodesList.setNodeType(nodeId, node, Node::DUMMY);
}

int SmartPath::actionAddFirstNode(const QPointF &c0,
                                  const QPointF &p1,
                                  const QPointF &c2) {
    const int insertId = mNodesList.appendNode(Node(c0, p1, c2));
    mNodesList.insertNodeAfter(insertId, Node(Node::MOVE));
    return insertId;
}

int SmartPath::actionAppendNodeAtEndNode(const int &endNodeId,
                                         const NodePointValues &values) {
    Node * const endNode = mNodesList.at(endNodeId);
    if(!endNode->isNormal())
        RuntimeThrow("Invalid node type. "
                     "End nodes should always be NORMAL.");
    const Node * const nextNode = mNodesList.at(endNode->getNextNodeId());
    if(nextNode->isMove()) {
        const Node nodeBlueprint(values.fC0, values.fP1, values.fC2);
        return mNodesList.insertNodeAfter(endNodeId, nodeBlueprint);
    } else {
        const Node nodeBlueprint = *endNode;
        endNode->fC0 = values.fC0;
        endNode->fP1 = values.fP1;
        endNode->fC2 = values.fC2;
        return mNodesList.insertNodeAfter(endNodeId, nodeBlueprint);
    }
}

int SmartPath::insertNodeBetween(const int& prevId,
                                 const int& nextId,
                                 const Node& nodeBlueprint) {
    if(!mNodesList.nodesConnected(prevId, nextId))
        RuntimeThrow("Cannot insert between not connected nodes");
    return mNodesList.insertNodeAfter(prevId, nodeBlueprint);
}

int SmartPath::actionInsertNodeBetween(const int &prevId,
                                      const int& nextId,
                                      const qreal& t) {
    return insertNodeBetween(prevId, nextId, Node(t));
}

int SmartPath::actionInsertNodeBetween(const int &prevId,
                                      const int& nextId,
                                      const QPointF &c0,
                                      const QPointF &p1,
                                      const QPointF &c2) {
    return insertNodeBetween(prevId, nextId, Node(c0, p1, c2));
}

void SmartPath::actionPromoteDissolvedNodeToNormal(const int &nodeId) {
    mNodesList.promoteDissolvedNodeToNormal(nodeId);
}

void SmartPath::actionMoveNodeBetween(const int& movedNodeId,
                                      const int& prevNodeId,
                                      const int& nextNodeId) {
    Node * const prevNode = mNodesList.at(prevNodeId);
    if(prevNode->getNextNodeId() != nextNodeId)
        RuntimeThrow("Trying to move between not connected nodes");
    mNodesList.moveNodeAfter(movedNodeId, mNodesList.at(movedNodeId),
                             prevNodeId, prevNode);
}

void SmartPath::actionDisconnectNodes(const int &node1Id, const int &node2Id) {
    Node * const node1 = mNodesList.at(node1Id);
    Node * const node2 = mNodesList.at(node2Id);
    if(node1->isMove() || node2->isMove())
        RuntimeThrow("Cannot disconnect move node");
    int prevId;
    int nextId;
    if(node1->getNextNodeId() == node2Id) {
        prevId = node1Id;
        nextId = node2Id;
    } else if(node2->getNextNodeId() == node1Id) {
        prevId = node2Id;
        nextId = node1Id;
    } else {
        RuntimeThrow("Trying to disconnect not connected nodes");
    }
    mNodesList.insertNodeAfter(prevId, Node(Node::MOVE));
    if(nextId > prevId) nextId++;

    Node * const prevNode = mNodesList.at(prevId);
    if(!prevNode->isNormal()) {
        const int prevNormalIdV = mNodesList.prevNormalId(prevId);
        Node * const prevNormalNode = mNodesList.at(prevNormalIdV);
        mNodesList.setNodeType(prevId, prevNode, Node::NORMAL);
        prevNode->fC0 = prevNormalNode->fC0;
        prevNode->fP1 = prevNormalNode->fP1;
        prevNode->fC2 = prevNormalNode->fC2;

        int currNodeId = prevNode->getPrevNodeId();
        while(true) {
            if(currNodeId == -1) break;
            Node * const currNode = mNodesList.at(currNodeId);
            if(currNode->isNormal() || currNode->isMove()) break;
            mNodesList.setNodeType(currNodeId, currNode, Node::DUMMY);
            currNodeId = currNode->getPrevNodeId();
        }
        mNodesList.setNodeType(prevNormalIdV, prevNormalNode, Node::DUMMY);
    }
    Node * const nextNode = mNodesList.at(nextId);
    if(!nextNode->isNormal()) {
        const int nextNormalIdV = mNodesList.nextNormalId(nextId);
        Node * const nextNormalNode = mNodesList.at(nextNormalIdV);
        mNodesList.setNodeType(nextId, nextNode, Node::NORMAL);
        nextNode->fC0 = nextNormalNode->fC0;
        nextNode->fP1 = nextNormalNode->fP1;
        nextNode->fC2 = nextNormalNode->fC2;

        int currNodeId = nextNode->getNextNodeId();
        while(true) {
            if(currNodeId == -1) break;
            Node * const currNode = mNodesList.at(currNodeId);
            if(currNode->isNormal() || currNode->isMove()) break;
            mNodesList.setNodeType(currNodeId, currNode, Node::DUMMY);
            currNodeId = currNode->getNextNodeId();
        }
        mNodesList.setNodeType(nextNormalIdV, nextNormalNode, Node::DUMMY);
    }
}

void SmartPath::actionConnectNodes(const int &node1Id,
                                  const int &node2Id) {
    const int moveNode1Id = mNodesList.lastSegmentNode(node1Id);
    const int moveNode2Id = mNodesList.lastSegmentNode(node2Id);
    if(moveNode1Id == -1 || moveNode2Id == -1)
        RuntimeThrow("Node is not part of a segment");
    // if closing single segment
    if(moveNode1Id == moveNode2Id) {
        Node * const moveNode = mNodesList.at(moveNode1Id);
        if(!moveNode->isMove())
            RuntimeThrow("Trying to connect a closed segment");
        const int firstNodeId = mNodesList.firstSegmentNode(node1Id);
        mNodesList.setNodeType(moveNode1Id, moveNode, Node::DUMMY);
        mNodesList.setNodeNextId(moveNode1Id, moveNode, firstNodeId);
        mNodesList.setNodePrevId(firstNodeId, moveNode1Id);
    } else { // if connecting two seperate segments
        Node * const moveNode1 = mNodesList.at(moveNode1Id);
        Node * const moveNode2 = mNodesList.at(moveNode2Id);
        if(!moveNode1->isMove() || !moveNode2->isMove())
            RuntimeThrow("Trying to connect a closed segment");
        RuntimeThrow("Not yet finished");
    }
}

void SmartPath::removeNodeWithIdAndTellPrevToDoSame(const int &nodeId) {
    mNodesList.removeNodeFromList(nodeId);
    if(mPrev) mPrev->removeNodeWithIdAndTellPrevToDoSame(nodeId);
}

void SmartPath::removeNodeWithIdAndTellNextToDoSame(const int &nodeId) {
    mNodesList.removeNodeFromList(nodeId);
    if(mNext) mNext->removeNodeWithIdAndTellNextToDoSame(nodeId);
}

void SmartPath::setPrev(SmartPath * const prev) {
    mPrev = prev;
    if(mPrev) mNodesList.setPrev(mPrev->getNodesPtr());
    else mNodesList.setPrev(nullptr);
}

void SmartPath::setNext(SmartPath * const next) {
    mNext = next;
    if(mNext) mNodesList.setNext(mNext->getNodesPtr());
    else mNodesList.setNext(nullptr);
}

NodeList *SmartPath::getNodesPtr() {
    return &mNodesList;
}

SkPath SmartPath::getPathAt() const {
    return mNodesList.toSkPath();
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

ValueRange SmartPath::dissolvedTRange(const int &nodeId) {
    return {mNodesList.prevT(nodeId), mNodesList.nextT(nodeId)};
}

NodeList SmartPath::interpolateNodesListWithNext(
        const qreal &nextWeight, const bool& simplify) const {
    return NodeList::sInterpolate(getNodesListForNext(simplify),
                                  mNext->getNodesListForPrev(simplify),
                                  nextWeight);
}

NodeList SmartPath::interpolateNodesListWithPrev(
        const qreal &prevWeight, const bool &simplify) const {
    return NodeList::sInterpolate(getNodesListForPrev(simplify),
                                  mPrev->getNodesListForNext(simplify),
                                  prevWeight);
}

SkPath SmartPath::getPathForPrev() const {
    if(mPrev) return getPathFor(mPrev);
    return mNodesList.toSkPath();
}

SkPath SmartPath::getPathForNext() const {
    if(mNext) return getPathFor(mNext);
    return mNodesList.toSkPath();
}

bool isEndPointAndShouldBeSplit(const int& nodeId,
                                const Node * const thisNode,
                                const Node * const neighNode,
                                const NodeList &thisNodes,
                                const NodeList &neighNodes) {
    Q_UNUSED(neighNode);
    if(!thisNodes.segmentClosed(nodeId)) {
        if(neighNodes.segmentClosed(nodeId)) {
            const int thisFirstId = thisNodes.firstSegmentNode(nodeId);
            if(thisFirstId == nodeId) return true;
            const int thisLastId = thisNodes.lastSegmentNode(nodeId);
            if(thisLastId == thisNode->getNextNodeId()) return true;
        }
    }
    return false;
}

bool shouldSplitThisNode(const int& nodeId,
                         const Node * const thisNode,
                         const Node * const neighNode,
                         const NodeList &thisNodes,
                         const NodeList &neighNodes) {
    if(thisNode->isNormal()) {
        return isEndPointAndShouldBeSplit(nodeId, thisNode, neighNode,
                                          thisNodes, neighNodes);
    } else if(thisNode->isDissolved()) {
        return neighNode->isMove();
//        return isEndPointAndShouldBeSplit(nodeId, neighNode, thisNode,
//                                          neighNodes, thisNodes);
    }
    return false;
}

void SmartPath::prepareForNewNeighBetweenThisAnd(
        SmartPath * const neighbour) {
    NodeList& neighNodes = neighbour->getNodesRef();
    NodeList result = mNodesList.createCopy(true);
    const bool nextNeigh = neighbour == mNext;
    if(nextNeigh) {
        neighNodes.setPrev(&result);
        mNodesList.setNext(&result);
        result.setNext(const_cast<NodeList*>(&neighNodes));
        result.setPrev(&mNodesList);
    } else {
        neighNodes.setNext(&result);
        mNodesList.setPrev(&result);
        result.setNext(&mNodesList);
        result.setPrev(const_cast<NodeList*>(&neighNodes));
    }

    const int iMax = neighNodes.count() - 1;
    if(result.count() - 1 != iMax)
        RuntimeThrow("Nodes count does not match");

    int iShift = 0;
    for(int i = 0; i <= iMax; i++) {
        const int resI = i + iShift;
        Node * const resultNode = result.at(resI);
        const Node * const neighbourNode = neighNodes.at(resI);
        const Node * const thisNode = mNodesList.at(resI);

        if(shouldSplitThisNode(i, thisNode, neighbourNode,
                               mNodesList, neighNodes)) {
            if(thisNode->isDissolved()) {
                result.promoteDissolvedNodeToNormal(resI, resultNode);
                result.splitNodeAndDisconnect(resI);
                iShift += 2;
            } else if(resultNode->isNormal()) {
                result.splitNode(resI);
                iShift++;
            }
        }
    }
    if(nextNeigh) {
        neighNodes.setPrev(&mNodesList);
        mNodesList.setNext(&neighNodes);
    } else {
        neighNodes.setNext(&mNodesList);
        mNodesList.setPrev(&neighNodes);
    }
}

NodeList SmartPath::getNodesListFor(const SmartPath * const neighbour,
                                    const bool &simplify) const {
    const NodeList& neighNodes = neighbour->getNodesRef();
    NodeList result = mNodesList.createCopy(true);

    const int iMax = neighNodes.count() - 1;
    if(result.count() - 1 != iMax)
        RuntimeThrow("Nodes count does not match");

    int iShift = 0;
    for(int i = 0; i <= iMax; i++) {
        const int resI = i + iShift;
        Node * const resultNode = result.at(resI);
        const Node * const neighbourNode = neighNodes.at(i);
        const Node * const thisNode = mNodesList.at(i);

        // Remove nodes if not needed
        if((neighbourNode->isDummy() || neighbourNode->isDissolved()) &&
           (thisNode->isDummy() || thisNode->isDissolved())) {
            if(simplify) {
                iShift--;
                result.removeNodeFromList(resI);
            } else if(thisNode->isDummy() && neighbourNode->isDissolved()) {
                resultNode->fT = 0.5;
                result.setNodeType(resI, resultNode, Node::DISSOLVED);
            }
        } else if(thisNode->isDissolved() && neighbourNode->isNormal()) {
            result.promoteDissolvedNodeToNormal(resI, resultNode);
        } else if(shouldSplitThisNode(i, thisNode, neighbourNode,
                                      mNodesList, neighNodes)) {
            if(thisNode->isDissolved()) {
                result.promoteDissolvedNodeToNormal(resI, resultNode);
                result.splitNodeAndDisconnect(resI);
                iShift += 2;
            } else if(resultNode->isNormal()) {
                result.splitNode(resI);
                iShift++;
            }
        }
    }
    return result;
}

SkPath SmartPath::getPathFor(const SmartPath * const neighbour) const {
    return getNodesListFor(neighbour, true).toSkPath();
}
