#include "smartpathcontainer.h"

void SmartPath::actionRemoveNormalNode(const int &nodeId) {
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

void SmartPath::actionAddFirstNode(const QPointF &c0,
                                   const QPointF &p1,
                                   const QPointF &c2) {
    const int insertId = mNodesList.appendNode(Node(c0, p1, c2));
    mNodesList.insertNodeAfter(insertId, Node(Node::MOVE));
}

void SmartPath::insertNodeBetween(const int& prevId,
                                  const int& nextId,
                                  const Node& nodeBlueprint) {
    if(!mNodesList.nodesConnected(prevId, nextId))
        RuntimeThrow("Cannot insert between not connected nodes");
    const int insertId = mNodesList.insertNodeAfter(prevId, nodeBlueprint);
    if(nodeBlueprint.isDissolved()) {
        Node& newNode = mNodesList.at(insertId);
        mNodesList.promoteDissolvedNodeToNormal(insertId, newNode);
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
    mNodesList.promoteDissolvedNodeToNormal(nodeId);
}

void SmartPath::actionDisconnectNodes(const int &node1Id, const int &node2Id) {
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

void SmartPath::actionConnectNodes(const int &node1Id,
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

NodeList SmartPath::interpolateNodesListWithNext(
        const qreal &nextWeight) const {
    return NodeList::sInterpolate(getNodesListForNext(),
                                  mNext->getNodesListForPrev(),
                                  nextWeight);
}

NodeList SmartPath::interpolateNodesListWithPrev(
        const qreal &prevWeight) const {
    return NodeList::sInterpolate(getNodesListForPrev(),
                                  mPrev->getNodesListForNext(),
                                  prevWeight);
}

SmartPath::SmartPath() : SmartPath(QList<Node>()) {}

SmartPath::SmartPath(const QList<Node> &list) {
    mNodesList.setNodeList(list);
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

NodeList SmartPath::getNodesListFor(SmartPath * const neighbour) const {
    const NodeList& neighNodes = neighbour->getNodesRef();
    NodeList result = mNodesList.createCopy();

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

SkPath SmartPath::getPathFor(SmartPath * const neighbour) const {
    return getNodesListFor(neighbour).toSkPath();
}
