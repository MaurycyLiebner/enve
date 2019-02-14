#include "nodelist.h"
#include "skia/skiaincludes.h"
#include "pointhelpers.h"
#include "exceptions.h"

int NodeList::firstSegmentNode(const int& nodeId) const {
    if(nodeId < 0) return -1;
    if(nodeId >= mNodes.count()) return -1;
    const Node * currNode = &mNodes.at(nodeId);
    int smallestId = nodeId;
    int currId = nodeId;
    while(true) {
        if(!currNode->hasPreviousNode()) return currId;
        const int prevId = currNode->getPrevNodeId();
        if(prevId == currId)
            RuntimeThrow("Node points to itself");
        if(prevId == smallestId) return smallestId;
        smallestId = qMin(prevId, smallestId);
        currNode = &mNodes.at(prevId);
        if(currNode->isMove()) return currId;
        currId = prevId;
    }
}

int NodeList::lastSegmentNode(const int& nodeId) const {
    if(nodeId < 0) return -1;
    if(nodeId >= mNodes.count()) return -1;
    const Node * currNode = &mNodes.at(nodeId);
    int smallestId = nodeId;
    int currId = nodeId;
    while(true) {
        if(currNode->isMove()) return currId;
        const int nextId = currNode->getNextNodeId();
        if(nextId == currId)
            RuntimeThrow("Node points to itself");
        if(nextId == smallestId) return currId;
        smallestId = qMin(nextId, smallestId);
        currNode = &mNodes.at(nextId);
        currId = nextId;
    }
}


void NodeList::removeNodeFromList(const int &nodeId) {
    const Node& nodeToRemove = mNodes.at(nodeId);
    if(nodeToRemove.hasPreviousNode()) {
        Node& prevNode = mNodes[nodeToRemove.getPrevNodeId()];
        prevNode.setNextNodeId(nodeToRemove.getNextNodeId());
    }
    if(nodeToRemove.hasNextNode()) {
        Node& nextNode = mNodes[nodeToRemove.getNextNodeId()];
        nextNode.setPrevNodeId(nodeToRemove.getPrevNodeId());
    }
    mNodes.removeAt(nodeId);
    for(int i = 0; i < mNodes.count(); i++) {
        Node& iNode = mNodes[i];
        iNode.shiftIdsGreaterThan(nodeId, -1);
    }
}

Node &NodeList::insertNodeToList(const int &nodeId, const Node &node) {
    mNodes.insert(nodeId, node);
    for(int i = 0; i < mNodes.count(); i++) {
        Node& iNode = mNodes[i];
        iNode.shiftIdsGreaterThan(nodeId - 1, 1);
    }
    return mNodes[nodeId];
}

int NodeList::nodesInSameSagment(const int& node1Id,
                                 const int& node2Id) const {
    const int firstSegment1 = firstSegmentNode(node1Id);
    const int firstSegment2 = firstSegmentNode(node2Id);
    return firstSegment1 && firstSegment1 == firstSegment2;
}

void NodeList::reverseSegment(const int& nodeId) {
    const int firstNodeId = firstSegmentNode(nodeId);
    if(firstNodeId == -1) return;
    Node& firstNode = mNodes[firstNodeId];
    int nextSegId = firstNode.getNextNodeId();
    firstNode.switchPrevAndNext();
    while(const int currNodeId = nextSegId) {
        if(currNodeId == firstNodeId) break;
        Node& currNode = mNodes[nextSegId];
        nextSegId = currNode.getNextNodeId();
        currNode.switchPrevAndNext();
    }
}

bool NodeList::segmentClosed(const int& nodeId) const {
    const int firstNodeId = firstSegmentNode(nodeId);
    if(firstNodeId == -1) return false;
    return mNodes.at(firstNodeId).hasPreviousNode();
}

int NodeList::insertNodeBefore(const int& nextId,
                               const Node& nodeBlueprint) {
    const int insertId = nextId;
    Node& insertedNode = insertNodeToList(insertId, nodeBlueprint);
    const int shiftedNextId = nextId + 1;
    Node& nextNode = mNodes[shiftedNextId];
    const int prevId = nextNode.getPrevNodeId();
    if(prevId != -1) {
        Node& prevNode = mNodes[prevId];
        prevNode.setNextNodeId(insertId);
    }
    nextNode.setPrevNodeId(insertId);
    insertedNode.setPrevNodeId(prevId);
    insertedNode.setNextNodeId(shiftedNextId);
    return insertId;
}

int NodeList::insertNodeAfter(const int& prevId,
                              const Node& nodeBlueprint) {
    Node& prevNode = mNodes[prevId];
    if(prevNode.isMove()) return insertNodeBefore(prevId + 1, nodeBlueprint);
    const int insertId = prevId + 1;
    Node& insertedNode = insertNodeToList(insertId, nodeBlueprint);
    const int nextId = prevNode.getNextNodeId();
    if(nextId != -1) {
        Node& nextNode = mNodes[nextId];
        if(nodeBlueprint.isMove()) nextNode.setPrevNodeId(-1);
        else nextNode.setPrevNodeId(insertId);
    }
    prevNode.setNextNodeId(insertId);
    insertedNode.setPrevNodeId(prevId);
    if(nodeBlueprint.isMove()) insertedNode.setNextNodeId(-1);
    else insertedNode.setNextNodeId(nextId);

    return insertId;
}

qCubicSegment2D gSegmentFromNodes(const Node& prevNode,
                                  const Node& nextNode) {
    return qCubicSegment2D(prevNode.fP1, prevNode.fC2,
                           nextNode.fC0, nextNode.fP1);
}

void NodeList::promoteDissolvedNodeToNormal(const int& nodeId,
                                            Node& node) {
    const int prevNormalIdV = prevNormalId(nodeId);
    const int nextNormalIdV = nextNormalId(nodeId);
    Node& prevNormal = mNodes[prevNormalIdV];
    Node& nextNormal = mNodes[nextNormalIdV];

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
        Node& iNode = mNodes[i];
        if(iNode.isDissolved()) {
            iNode.fT = gMapTToFragment(0, node.fT, iNode.fT);
        }
    }
    for(int i = nodeId + 1; i < nextNormalIdV; i++) {
        Node& iNode = mNodes[i];
        if(iNode.isDissolved()) {
            iNode.fT = gMapTToFragment(node.fT, 1, iNode.fT);
        }
    }
}

void NodeList::promoteDissolvedNodeToNormal(const int& nodeId) {
    promoteDissolvedNodeToNormal(nodeId, at(nodeId));
}

void NodeList::splitNode(const int& nodeId) {
    Node& node = mNodes[nodeId];
    Node newNode = node;
    if(node.isNormal()) {
        node.fC2 = node.fP1;
        newNode.fC0 = newNode.fP1;
    }
    insertNodeAfter(nodeId, newNode);
}

void NodeList::splitNodeAndDisconnect(const int& nodeId) {
    splitNode(nodeId);
    insertNodeAfter(nodeId, Node(Node::MOVE));
}

bool NodeList::shouldSplitThisNode(const int& nodeId,
                                   const Node& thisNode,
                                   const Node& neighNode,
                                   const NodeList * const thisNodes,
                                   const NodeList * const neighNodes) const {
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
            if(thisNodes->nextNormalId(nodeId) == -1 &&
               neighNodes->nextNormalId(nodeId) != -1) return true;
            const Node& thisNextNode = thisNodes->at(thisNode.getNextNodeId());
            // split node only if both nodes differ
            if(!thisNextNode.isMove()) return prevDiffers && nextDiffers;
        }
    }
    // if node is not normal and in the middle
    // split if previous or next node differs
    return prevDiffers || nextDiffers;
}

bool NodeList::nodesConnected(const int& node1Id,
                              const int& node2Id) const {
    const Node& node1 = mNodes.at(node1Id);
    if(node1.getNextNodeId() == node2Id) return true;
    if(node1.getPrevNodeId() == node2Id) return true;
    return false;
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

SkPath NodeList::toSkPath() const{
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

qreal NodeList::prevT(const int &nodeId) const {
    const int &prevId = prevNonDummyId(nodeId);
    const Node& node = mNodes.at(prevId);
    if(node.isNormal()) return 0;
    return node.fT;
}

qreal NodeList::nextT(const int &nodeId) const {
    const int &nextId = nextNonDummyId(nodeId);
    const Node& node = mNodes.at(nextId);
    if(node.isNormal()) return 1;
    return node.fT;
}

int NodeList::prevNormalId(const int& nodeId) const {
    const Node * currNode = &mNodes.at(nodeId);
    while(currNode->hasPreviousNode()) {
        const int currId = currNode->getPrevNodeId();
        currNode = &mNodes.at(currId);
        if(currNode->isNormal()) return currId;
    }
    return -1;
}

int NodeList::nextNormalId(const int& nodeId) const {
    const Node * currNode = &mNodes.at(nodeId);
    while(currNode->hasNextNode()) {
        const int currId = currNode->getNextNodeId();
        currNode = &mNodes.at(currId);
        if(currNode->isNormal()) return currId;
    }
    return -1;
}

int NodeList::prevNonDummyId(const int& nodeId) const {
    const Node * currNode = &mNodes.at(nodeId);
    while(currNode->hasPreviousNode()) {
        const int currId = currNode->getPrevNodeId();
        currNode = &mNodes.at(currId);
        if(!currNode->isDummy()) return currId;
    }
    return -1;
}

int NodeList::nextNonDummyId(const int& nodeId) const {
    const Node * currNode = &mNodes.at(nodeId);
    while(currNode->hasNextNode()) {
        const int currId = currNode->getNextNodeId();
        currNode = &mNodes.at(currId);
        if(!currNode->isDummy()) return currId;
    }
    return -1;
}

bool NodeList::updateNodeTypeAfterNeighbourChanged(const int &nodeId) {
    Node& node = mNodes[nodeId];
    if(node.isNormal() || node.isMove()) return false;
    Node::Type prevType = Node::DUMMY;
    Node::Type nextType = Node::DUMMY;
    int prevNextId = -1;
    int prevPrevId = -1;
    if(mPrev) {
        const Node& prevNode = mPrev->at(nodeId);
        prevType = prevNode.getType();
        prevNextId = prevNode.getNextNodeId();
        prevPrevId = prevNode.getPrevNodeId();
    }
    int nextNextId = -1;
    int nextPrevId = -1;
    if(mNext) {
        const Node& nextNode = mNext->at(nodeId);
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
