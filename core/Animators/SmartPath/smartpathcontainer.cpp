#include "smartpathcontainer.h"

void PathBase::actionRemoveNormalNode(const int &nodeId) {
    Node& node = mNodesList.at(nodeId);
    if(!node.isNormal())
        RuntimeThrow("Invalid node type. "
                     "Only normal nodes can be removed.");

    Node * currNode = &node;
    while(currNode->hasPreviousNode()) {
        const int prevId = currNode->getPrevNodeId();
        currNode = &mNodesList.at(prevId);
        if(currNode->isNormal() || currNode->isMove()) break;
        if(currNode->isDissolved()) currNode->fT *= 0.5;
    }
    currNode = &node;
    while(currNode->hasNextNode()) {
        const int nextId = currNode->getNextNodeId();
        currNode = &mNodesList.at(nextId);
        if(currNode->isNormal() || currNode->isMove()) break;
        if(currNode->isDissolved()) currNode->fT = currNode->fT*0.5 + 0.5;
    }
    mNodesList.setNodeType(nodeId, node, Node::DUMMY);
}

int PathBase::actionAddFirstNode(const QPointF &c0,
                                 const QPointF &p1,
                                 const QPointF &c2) {
    const int insertId = mNodesList.appendNode(Node(c0, p1, c2));
    mNodesList.insertNodeAfter(insertId, Node(Node::MOVE));
    return insertId;
}

int PathBase::actionAppendNodeAtEndNode(const int &endNodeId,
                                        const NodePointValues &values) {
    Node& endNode = mNodesList.at(endNodeId);
    if(!endNode.isNormal())
        RuntimeThrow("Invalid node type. "
                     "End nodes should always be NORMAL.");
    const Node& moveNode = endNode.getNextNodeId();
    if(!moveNode.isMove())
        RuntimeThrow("Invalid node type. "
                     "End nodes should have MOVE node as next.");
    return mNodesList.insertNodeAfter(endNodeId,
                                      Node(values.fC0, values.fP1, values.fC2));
}

int PathBase::insertNodeBetween(const int& prevId,
                                const int& nextId,
                                const Node& nodeBlueprint) {
    if(!mNodesList.nodesConnected(prevId, nextId))
        RuntimeThrow("Cannot insert between not connected nodes");
    return mNodesList.insertNodeAfter(prevId, nodeBlueprint);
}

int PathBase::actionInsertNodeBetween(const int &prevId,
                                      const int& nextId,
                                      const qreal& t) {
    return insertNodeBetween(prevId, nextId, Node(t));
}

int PathBase::actionInsertNodeBetween(const int &prevId,
                                      const int& nextId,
                                      const QPointF &c0,
                                      const QPointF &p1,
                                      const QPointF &c2) {
    return insertNodeBetween(prevId, nextId, Node(c0, p1, c2));
}

void PathBase::actionPromoteDissolvedNodeToNormal(const int &nodeId) {
    mNodesList.promoteDissolvedNodeToNormal(nodeId);
}

void PathBase::actionMoveNodeBetween(const int& movedNodeId,
                                     const int& prevNodeId,
                                     const int& nextNodeId) {
    Node& prevNode = mNodesList.at(prevNodeId);
    if(prevNode.getNextNodeId() != nextNodeId)
        RuntimeThrow("Trying to move between not connected nodes");
    mNodesList.moveNodeAfter(movedNodeId, mNodesList.at(movedNodeId),
                             prevNodeId, prevNode);
}

void PathBase::actionDisconnectNodes(const int &node1Id, const int &node2Id) {
    Node& node1 = mNodesList.at(node1Id);
    Node& node2 = mNodesList.at(node2Id);
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
    mNodesList.insertNodeAfter(prevId, Node(Node::MOVE));
    if(nextId > prevId) nextId++;

    Node& prevNode = mNodesList.at(prevId);
    if(!prevNode.isNormal()) {
        const int prevNormalIdV = mNodesList.prevNormalId(prevId);
        Node& prevNormalNode = mNodesList.at(prevNormalIdV);
        mNodesList.setNodeType(prevId, prevNode, Node::NORMAL);
        prevNode.fC0 = prevNormalNode.fC0;
        prevNode.fP1 = prevNormalNode.fP1;
        prevNode.fC2 = prevNormalNode.fC2;

        int currNodeId = prevNode.getPrevNodeId();
        while(true) {
            if(currNodeId == -1) break;
            Node& currNode = mNodesList.at(currNodeId);
            if(currNode.isNormal() || currNode.isMove()) break;
            mNodesList.setNodeType(currNodeId, currNode, Node::DUMMY);
            currNodeId = currNode.getPrevNodeId();
        }
        mNodesList.setNodeType(prevNormalIdV, prevNormalNode, Node::DUMMY);
    }
    Node& nextNode = mNodesList.at(nextId);
    if(!nextNode.isNormal()) {
        const int nextNormalIdV = mNodesList.nextNormalId(nextId);
        Node& nextNormalNode = mNodesList.at(nextNormalIdV);
        mNodesList.setNodeType(nextId, nextNode, Node::NORMAL);
        nextNode.fC0 = nextNormalNode.fC0;
        nextNode.fP1 = nextNormalNode.fP1;
        nextNode.fC2 = nextNormalNode.fC2;

        int currNodeId = nextNode.getNextNodeId();
        while(true) {
            if(currNodeId == -1) break;
            Node& currNode = mNodesList.at(currNodeId);
            if(currNode.isNormal() || currNode.isMove()) break;
            mNodesList.setNodeType(currNodeId, currNode, Node::DUMMY);
            currNodeId = currNode.getNextNodeId();
        }
        mNodesList.setNodeType(nextNormalIdV, nextNormalNode, Node::DUMMY);
    }
}

void PathBase::actionConnectNodes(const int &node1Id,
                                  const int &node2Id) {
    const int moveNode1Id = mNodesList.lastSegmentNode(node1Id);
    const int moveNode2Id = mNodesList.lastSegmentNode(node2Id);
    if(moveNode1Id == -1 || moveNode2Id == -1)
        RuntimeThrow("Node is not part of a segment");
    // if closing single segment
    if(moveNode1Id == moveNode2Id) {
        Node& moveNode = mNodesList.at(moveNode1Id);
        if(!moveNode.isMove())
            RuntimeThrow("Trying to connect a closed segment");
        const int firstNodeId = mNodesList.firstSegmentNode(node1Id);
        mNodesList.setNodeType(moveNode1Id, moveNode, Node::DUMMY);
        mNodesList.setNodeNextId(moveNode1Id, moveNode, firstNodeId);
        mNodesList.setNodePrevId(firstNodeId, moveNode1Id);
    } else { // if connecting two seperate segments
        Node& moveNode1 = mNodesList.at(moveNode1Id);
        Node& moveNode2 = mNodesList.at(moveNode2Id);
        if(!moveNode1.isMove() || !moveNode2.isMove())
            RuntimeThrow("Trying to connect a closed segment");
        RuntimeThrow("Not yet finished");
    }
}

void PathBase::removeNodeWithIdAndTellPrevToDoSame(const int &nodeId) {
    mNodesList.removeNodeFromList(nodeId);
    if(mPrev) mPrev->removeNodeWithIdAndTellPrevToDoSame(nodeId);
}

void PathBase::removeNodeWithIdAndTellNextToDoSame(const int &nodeId) {
    mNodesList.removeNodeFromList(nodeId);
    if(mNext) mNext->removeNodeWithIdAndTellNextToDoSame(nodeId);
}

void PathBase::setPrev(PathBase * const prev) {
    mPrev = prev;
    if(mPrev) mNodesList.setPrev(mPrev->getNodesPtr());
    else mNodesList.setPrev(nullptr);
}

void PathBase::setNext(PathBase * const next) {
    mNext = next;
    if(mNext) mNodesList.setNext(mNext->getNodesPtr());
    else mNodesList.setNext(nullptr);
}

NodeList *PathBase::getNodesPtr() {
    return &mNodesList;
}

SkPath PathBase::getPathAt() const {
    return mNodesList.toSkPath();
}

SkPath PathBase::interpolateWithNext(const qreal &nextWeight) const {
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

SkPath PathBase::interpolateWithPrev(const qreal &prevWeight) const {
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

ValueRange PathBase::dissolvedTRange(const int &nodeId) {
    return {mNodesList.prevT(nodeId), mNodesList.nextT(nodeId)};
}

NodeList PathBase::interpolateNodesListWithNext(
        const qreal &nextWeight) const {
    return NodeList::sInterpolate(getNodesListForNext(),
                                  mNext->getNodesListForPrev(),
                                  nextWeight);
}

NodeList PathBase::interpolateNodesListWithPrev(
        const qreal &prevWeight) const {
    return NodeList::sInterpolate(getNodesListForPrev(),
                                  mPrev->getNodesListForNext(),
                                  prevWeight);
}

PathBase::PathBase(const NodeList::Type &type) :
    PathBase(QList<Node>(), type) {}

PathBase::PathBase(const QList<Node> &list,
                   const NodeList::Type &type) :
    mType(type), mNodesList(type, false) {
    mNodesList.setNodeList(list);
}

SkPath PathBase::getPathForPrev() const {
    if(mPrev && mType == NodeList::SMART) return getPathFor(mPrev);
    return mNodesList.toSkPath();
}

SkPath PathBase::getPathForNext() const {
    if(mNext && mType == NodeList::SMART) return getPathFor(mNext);
    return mNodesList.toSkPath();
}

bool isEndPointAndShouldBeSplit(const int& nodeId,
                                const Node& thisNode,
                                const Node& neighNode,
                                const NodeList &thisNodes,
                                const NodeList &neighNodes) {
    if(!thisNodes.segmentClosed(nodeId)) {
        const int thisLastId = thisNodes.lastSegmentNode(nodeId);
        const int neighLastId = neighNodes.lastSegmentNode(nodeId);
        if(thisLastId == thisNode.getNextNodeId() &&
           neighLastId != neighNode.getNextNodeId()) return true;
        const int thisFirstId = thisNodes.firstSegmentNode(nodeId);
        const int neighFirstId = neighNodes.firstSegmentNode(nodeId);
        if(thisFirstId == nodeId && neighFirstId != nodeId) return true;
    }
    return false;
}

bool shouldSplitThisNode(const int& nodeId,
                         const Node& thisNode,
                         const Node& neighNode,
                         const NodeList &thisNodes,
                         const NodeList &neighNodes) {
    if(thisNode.isNormal()) {
        return isEndPointAndShouldBeSplit(nodeId, thisNode, neighNode,
                                          thisNodes, neighNodes);
    } else if(thisNode.isDissolved()) {
        return neighNode.isMove();
//        return isEndPointAndShouldBeSplit(nodeId, neighNode, thisNode,
//                                          neighNodes, thisNodes);
    }
    return false;
}

NodeList PathBase::getNodesListFor(PathBase * const neighbour) const {
    const NodeList& neighNodes = neighbour->getNodesRef();
    NodeList result = mNodesList.createCopy(true);

    int iMax = neighNodes.count() - 1;
    if(result.count() - 1 != iMax)
        RuntimeThrow("Nodes count does not match");

    int iShift = 0;
    for(int i = 0; i <= iMax; i++) {
        const int resI = i + iShift;
        Node& resultNode = result.at(resI);
        const Node& neighbourNode = neighNodes.at(i);
        const Node& thisNode = mNodesList.at(i);

        // Remove nodes if not needed
        if((neighbourNode.isDummy() || neighbourNode.isDissolved()) &&
           (thisNode.isDummy() || thisNode.isDissolved())) {
            iShift--;
            result.removeNodeFromList(resI);
        }

        // Create splits for connecting/disconnecting
        if(shouldSplitThisNode(i, thisNode, neighbourNode,
                               mNodesList, neighNodes)) {
            if(thisNode.isDissolved()) {
                result.promoteDissolvedNodeToNormal(resI);
                result.splitNodeAndDisconnect(resI);
                iShift += 2;
            } else if(resultNode.isNormal()) {
                result.splitNode(resI);
                iShift++;
            }
        }
    }
    return result;
}

SkPath PathBase::getPathFor(PathBase * const neighbour) const {
    return getNodesListFor(neighbour).toSkPath();
}
