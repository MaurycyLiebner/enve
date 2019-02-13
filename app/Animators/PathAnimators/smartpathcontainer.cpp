#include "smartpathcontainer.h"

void gRemoveNodeFromList(const int &nodeId, QList<Node>& nodes) {
    const Node& nodeToRemove = nodes.at(nodeId);
    if(nodeToRemove.hasPreviousNode()) {
        Node& prevNode = nodes[nodeToRemove.getPrevNodeId()];
        prevNode.setNextNodeId(nodeToRemove.getNextNodeId());
    }
    if(nodeToRemove.hasNextNode()) {
        Node& nextNode = nodes[nodeToRemove.getNextNodeId()];
        nextNode.setPrevNodeId(nodeToRemove.getPrevNodeId());
    }
    nodes.removeAt(nodeId);
    for(int i = 0; i < nodes.count(); i++) {
        Node& iNode = nodes[i];
        iNode.shiftIdsGreaterThan(nodeId, -1);
    }
}

Node &gInsertNodeToList(const int &nodeId, const Node &node,
                        QList<Node>& nodes) {
    nodes.insert(nodeId, node);
    for(int i = 0; i < nodes.count(); i++) {
        Node& iNode = nodes[i];
        iNode.shiftIdsGreaterThan(nodeId - 1, 1);
    }
    return nodes[nodeId];
}

int gFirstSegmentNode(const int& nodeId, const QList<Node>& nodes) {
    if(nodeId < 0) return -1;
    if(nodeId >= nodes.count()) return -1;
    const Node * currNode = &nodes.at(nodeId);
    int smallestId = nodeId;
    int currId = nodeId;
    while(true) {
        if(!currNode->hasPreviousNode()) return currId;
        const int prevId = currNode->getPrevNodeId();
        if(prevId == currId)
            RuntimeThrow("Node points to itself");
        if(prevId == smallestId) return smallestId;
        smallestId = qMin(prevId, smallestId);
        currNode = &nodes.at(prevId);
        if(currNode->isMove()) return currId;
        currId = prevId;
    }
}

int gLastSegmentNode(const int& nodeId, const QList<Node>& nodes) {
    if(nodeId < 0) return -1;
    if(nodeId >= nodes.count()) return -1;
    const Node * currNode = &nodes.at(nodeId);
    int smallestId = nodeId;
    int currId = nodeId;
    while(true) {
        if(currNode->isMove()) return currId;
        const int nextId = currNode->getNextNodeId();
        if(nextId == currId)
            RuntimeThrow("Node points to itself");
        if(nextId == smallestId) return currId;
        smallestId = qMin(nextId, smallestId);
        currNode = &nodes.at(nextId);
        currId = nextId;
    }
}

int gNodesInSameSagment(const int& node1Id,
                        const int& node2Id,
                        const QList<Node>& nodes) {
    const int firstSegment1 = gFirstSegmentNode(node1Id, nodes);
    const int firstSegment2 = gFirstSegmentNode(node2Id, nodes);
    return firstSegment1 && firstSegment1 == firstSegment2;
}

void gReverseSegment(const int& nodeId, QList<Node>& nodes) {
    const int firstNodeId = gFirstSegmentNode(nodeId, nodes);
    if(firstNodeId == -1) return;
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

bool gSegmentClosed(const int& nodeId, const QList<Node>& nodes) {
    const int firstNodeId = gFirstSegmentNode(nodeId, nodes);
    if(firstNodeId == -1) return false;
    return nodes.at(firstNodeId).hasPreviousNode();
}

int gInsertNodeBefore(const int& nextId,
                      const Node& nodeBlueprint,
                      QList<Node>& nodes) {
    const int insertId = nextId;
    Node& insertedNode = gInsertNodeToList(insertId, nodeBlueprint, nodes);
    const int shiftedNextId = nextId + 1;
    Node& nextNode = nodes[shiftedNextId];
    const int prevId = nextNode.getPrevNodeId();
    if(prevId != -1) {
        Node& prevNode = nodes[prevId];
        prevNode.setNextNodeId(insertId);
    }
    nextNode.setPrevNodeId(insertId);
    insertedNode.setPrevNodeId(prevId);
    insertedNode.setNextNodeId(shiftedNextId);
    return insertId;
}

int gInsertNodeAfter(const int& prevId,
                     const Node& nodeBlueprint,
                     QList<Node>& nodes) {
    Node& prevNode = nodes[prevId];
    if(prevNode.isMove()) return gInsertNodeBefore(prevId + 1, nodeBlueprint,
                                                  nodes);
    const int insertId = prevId + 1;
    Node& insertedNode = gInsertNodeToList(insertId, nodeBlueprint, nodes);
    const int nextId = prevNode.getNextNodeId();
    if(nextId != -1) {
        Node& nextNode = nodes[nextId];
        if(nodeBlueprint.isMove()) nextNode.setPrevNodeId(-1);
        else nextNode.setPrevNodeId(insertId);
    }
    prevNode.setNextNodeId(insertId);
    insertedNode.setPrevNodeId(prevId);
    if(nodeBlueprint.isMove()) insertedNode.setNextNodeId(-1);
    else insertedNode.setNextNodeId(nextId);

    return insertId;
}

QList<QList<Node>> gSortNodeListAccoringToConnetions(const QList<Node>& srcList) {
    QList<QList<Node>> result;
    QList<Node> segment;
    QList<int> srcIds;
    for(int i = 0; i < srcList.count(); i++) {
        srcIds << i;
    }
    while(!srcIds.isEmpty()) {
        const int firstSrcId = gFirstSegmentNode(srcIds.first(), srcList);
        if(firstSrcId == -1) {
            srcIds.removeFirst();
            continue;
        }
        const int firstResultId = segment.count();
        int nextSrcId = firstSrcId;
        bool first = true;
        while(true) {
            srcIds.removeOne(nextSrcId);
            const Node& srcNode = srcList.at(nextSrcId);
            Node newNode = srcNode;
            if(first) first = false;
            else newNode.setPrevNodeId(segment.count() - 1);

            if(newNode.hasNextNode())
                newNode.setNextNodeId(segment.count() + 1);

            if(!newNode.isMove() && !newNode.isDummy()) segment << newNode;

            if(!srcNode.hasNextNode()) break;
            nextSrcId = srcNode.getNextNodeId();
            if(nextSrcId == firstSrcId) {
                segment[segment.count() - 1].setNextNodeId(firstResultId);
                segment[firstResultId].setPrevNodeId(segment.count() - 1);
                break;
            }
        }
        if(!segment.isEmpty()) {
            result << segment;
            segment.clear();
        }
    }
    if(!segment.isEmpty()) result << segment;
    return result;
}

void gCubicTo(const Node& prevNode, const Node& nextNode,
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
        qDebug() << "Cubic to:" << first.p1();
        seg = div.second;
        lastT = t;
    }
    result.cubicTo(qPointToSk(seg.c1()),
                   qPointToSk(seg.c2()),
                   qPointToSk(seg.p1()));
    qDebug() << "Cubic to:" << seg.p1();
    dissolvedTs.clear();
}

SkPath gNodesToSkPath(const QList<Node>& nodes) {
    const auto segments = gSortNodeListAccoringToConnetions(nodes);
    SkPath result;

    const Node * firstNode = nullptr;
    const Node * prevNormalNode = nullptr;

    QList<qreal> dissolvedTs;

    for(const auto& segment : segments) {
        SkPath currPath;
        bool move = true;
        bool close = false;
        for(int i = 0; i < segment.count(); i++) {
            const Node& node = segment.at(i);
            if(node.isDummy()) continue;
            else if(node.isDissolved()) dissolvedTs << node.fT;
            else if(node.isMove())
                RuntimeThrow("No MOVE node should have gotten here");
            else if(node.isNormal()) {
                if(move) {
                    firstNode = &node;
                    close = firstNode->hasPreviousNode();
                    currPath.moveTo(qPointToSk(node.fP1));
                    qDebug() << "Move to:" << node.fP1;
                    move = false;
                } else {
                    gCubicTo(*prevNormalNode, node,
                            dissolvedTs, currPath);
                }
                prevNormalNode = &node;
            } else {
                RuntimeThrow("Unrecognized node type");
            }
        } // for each node
        if(close) {
            gCubicTo(*prevNormalNode, *firstNode,
                    dissolvedTs, currPath);
            currPath.close();
        }
        result.addPath(currPath);
    }
    qDebug() << "";
    return result;
}

qCubicSegment2D gSegmentFromNodes(const Node& prevNode,
                                  const Node& nextNode) {
    return qCubicSegment2D(prevNode.fP1, prevNode.fC2,
                           nextNode.fC0, nextNode.fP1);
}

int gPrevNormalId(const int& nodeId, const QList<Node>& nodes) {
    const Node * currNode = &nodes.at(nodeId);
    while(currNode->hasPreviousNode()) {
        const int currId = currNode->getPrevNodeId();
        currNode = &nodes.at(currId);
        if(currNode->isNormal()) return currId;
    }
    return -1;
}

int gNextNormalId(const int& nodeId, const QList<Node>& nodes) {
    const Node * currNode = &nodes.at(nodeId);
    while(currNode->hasNextNode()) {
        const int currId = currNode->getNextNodeId();
        currNode = &nodes.at(currId);
        if(currNode->isNormal()) return currId;
    }
    return -1;
}

int gPrevNonDummyId(const int& nodeId, const QList<Node>& nodes) {
    const Node * currNode = &nodes.at(nodeId);
    while(currNode->hasPreviousNode()) {
        const int currId = currNode->getPrevNodeId();
        currNode = &nodes.at(currId);
        if(!currNode->isDummy()) return currId;
    }
    return -1;
}

int gNextNonDummyId(const int& nodeId, const QList<Node>& nodes) {
    const Node * currNode = &nodes.at(nodeId);
    while(currNode->hasNextNode()) {
        const int currId = currNode->getNextNodeId();
        currNode = &nodes.at(currId);
        if(!currNode->isDummy()) return currId;
    }
    return -1;
}

void gPromoteDissolvedNodeToNormal(const int& nodeId,
                                   Node& node,
                                   QList<Node>& nodes) {
    const int prevNormalIdV = gPrevNormalId(nodeId, nodes);
    const int nextNormalIdV = gNextNormalId(nodeId, nodes);
    Node& prevNormal = nodes[prevNormalIdV];
    Node& nextNormal = nodes[nextNormalIdV];

    auto seg = gSegmentFromNodes(prevNormal, nextNormal);
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

void gPromoteDissolvedNodeToNormal(const int& nodeId,
                                  QList<Node>& nodes) {
    Node& node = nodes[nodeId];
    gPromoteDissolvedNodeToNormal(nodeId, node, nodes);
}

void gSplitNode(const int& nodeId, QList<Node>& nodes) {
    Node& node = nodes[nodeId];
    Node newNode = node;
    if(node.isNormal()) {
        node.fC2 = node.fP1;
        newNode.fC0 = newNode.fP1;
    }
    gInsertNodeAfter(nodeId, newNode, nodes);
}

void gSplitNodeAndDisconnect(const int& nodeId, QList<Node>& nodes) {
    gSplitNode(nodeId, nodes);
    gInsertNodeAfter(nodeId, Node(Node::MOVE), nodes);
}

bool gShouldSplitThisNode(const int& nodeId,
                          const Node& thisNode,
                          const Node& neighNode,
                          const QList<Node>& thisNodes,
                          const QList<Node>& neighNodes) {
    const int thisPrevId = thisNode.getPrevNodeId();
    const int thisNextId = thisNode.getNextNodeId();
    const int neighPrevId = neighNode.getPrevNodeId();
    const int neighNextId = neighNode.getNextNodeId();
    const bool prevDiffers = thisPrevId != neighPrevId &&
                             thisPrevId != neighNextId;
    const bool nextDiffers = thisNextId != neighNextId &&
                             thisNextId != neighPrevId;
    // if node is normal
    if(thisNode.isNormal()) {
        // if node is in the middle(has both previous and next node)
        if(thisNode.hasNextNode() && thisNode.hasPreviousNode()) {
            if(gNextNormalId(nodeId, thisNodes) == -1 &&
               gNextNormalId(nodeId, neighNodes) != -1) return true;
            const Node& thisNextNode = thisNodes.at(thisNode.getNextNodeId());
            // split node only if both nodes differ
            if(!thisNextNode.isMove()) return prevDiffers && nextDiffers;
        }
    }
    // if node is not normal and in the middle
    // split if previous or next node differs
    return prevDiffers || nextDiffers;
}

bool gNodesConnected(const int& node1Id, const int& node2Id,
                     const QList<Node>& nodes) {
    const Node& node1 = nodes.at(node1Id);
    if(node1.getNextNodeId() == node2Id) return true;
    if(node1.getPrevNodeId() == node2Id) return true;
    return false;
}

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

qreal SmartPath::prevT(const int &nodeId) const {
    const int &prevId = gPrevNonDummyId(nodeId, mNodes);
    const Node& node = mNodes.at(prevId);
    if(node.isNormal()) return 0;
    return node.fT;
}

qreal SmartPath::nextT(const int &nodeId) const {
    const int &nextId = gNextNonDummyId(nodeId, mNodes);
    const Node& node = mNodes.at(nextId);
    if(node.isNormal()) return 1;
    return node.fT;
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
