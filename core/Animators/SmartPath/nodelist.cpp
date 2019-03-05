#include "nodelist.h"
#include "skia/skiaincludes.h"
#include "pointhelpers.h"
#include "exceptions.h"
#include "smartPointers/sharedpointerdefs.h"

qCubicSegment2D gSegmentFromNodes(const Node& prevNode,
                                  const Node& nextNode) {
    return qCubicSegment2D(prevNode.fP1, prevNode.fC2,
                           nextNode.fC0, nextNode.fP1);
}

void NodeList::moveNodeAfter(const int& moveNodeId, Node * const moveNode,
                             const int& afterNodeId, Node * const afterNode) {
    const int movePrevId = moveNode->getPrevNodeId();
    const int moveNextId = moveNode->getNextNodeId();
    setNodeNextId(movePrevId, moveNextId);
    setNodePrevId(moveNextId, movePrevId);

    const int afterNextId = afterNode->getNextNodeId();
    setNodeNextId(afterNodeId, afterNode, moveNodeId);
    setNodePrevAndNextId(moveNodeId, moveNode, afterNodeId, afterNextId);
    setNodePrevId(afterNextId, moveNodeId);
}

void NodeList::moveNodeBefore(const int& moveNodeId, Node * const moveNode,
                              const int& beforeNodeId, Node * const beforeNode) {
    const int movePrevId = moveNode->getPrevNodeId();
    const int moveNextId = moveNode->getNextNodeId();
    setNodeNextId(movePrevId, moveNextId);
    setNodePrevId(moveNextId, movePrevId);

    const int beforePrevId = beforeNode->getPrevNodeId();
    setNodePrevId(beforeNodeId, beforeNode, moveNodeId);
    setNodePrevAndNextId(moveNodeId, moveNode, beforePrevId, beforeNodeId);
    setNodeNextId(beforePrevId, moveNodeId);
}

void NodeList::updateDissolvedNodePosition(const int &nodeId) {
    updateDissolvedNodePosition(nodeId, at(nodeId));
}

void NodeList::updateDissolvedNodePosition(const int &nodeId,
                                           Node * const node) {
    const int prevId = prevNormalId(nodeId);
    const int nextId = nextNormalId(nodeId);
    const Node * const prevNode = at(prevId);
    const Node * const nextNode = at(nextId);
    const auto normalSeg = gSegmentFromNodes(*prevNode, *nextNode);
    node->fP1 = normalSeg.posAtT(node->fT);
}

void NodeList::updateDummyNodePosition(const int& nodeId) {
    updateDummyNodePosition(nodeId, at(nodeId));
}

void NodeList::updateDummyNodePosition(const int &nodeId, Node * const node) {
    int nPrev = 0;
    int nDummies = 1;
    const Node * cNode = node;
    while(cNode->getPrevNodeId() != -1) {
        const int prevId = cNode->getPrevNodeId();
        cNode = at(prevId);
        if(!cNode->isDummy()) break;
        nDummies++;
        nPrev++;
    }
    cNode = node;
    while(cNode->getNextNodeId() != -1) {
        const int nextId = cNode->getNextNodeId();
        cNode = at(nextId);
        if(!cNode->isDummy()) break;
        nDummies++;
    }
    const qreal prevTv = prevT(nodeId);
    const qreal nextTv = nextT(nodeId);
    node->fT = prevTv + (nPrev + 1)*(nextTv - prevTv)/(nDummies + 1);
    updateDissolvedNodePosition(nodeId, node);
}

bool NodeList::read(QIODevice * const src) {
    int nNodes;
    src->read(rcChar(&nNodes), sizeof(int));
    if(mPrev) {
        if(mPrev->getList().count() != nNodes)
            RuntimeThrow("Reading from src would break number of nodes");
    }
    if(mNext) {
        if(mNext->getList().count() != nNodes)
            RuntimeThrow("Reading from src would break number of nodes");
    }
    mNodes.clear();
    for(int i = 0; i < nNodes; i++) {
        Node node;
        src->read(rcChar(&node), sizeof(Node));
        mNodes.append(node);
    }
    return true;
}

bool NodeList::write(QIODevice * const dst) const {
    int nNodes = mNodes.count();
    dst->write(rcConstChar(&nNodes), sizeof(int));
    for(const auto& node : mNodes) {
        dst->write(rcConstChar(node.get()), sizeof(Node));
    }
    return true;
}

NodeList NodeList::sInterpolate(const NodeList &list1,
                                const NodeList &list2,
                                const qreal& weight2) {
    ListOfNodes resultList;
    const auto list1v = list1.getList();
    const auto list2v = list2.getList();
    if(list1v.count() != list2v.count())
        RuntimeThrow("Cannot interpolate paths with different node count");
    const int listCount = list1v.count();
    for(int i = 0; i < listCount; i++) {
        const Node * const node1 = list1v.at(i);
        const Node * const node2 = list2v.at(i);
        resultList.append(Node::sInterpolateNormal(*node1, *node2, weight2));
    }
    return NodeList(resultList);
}

int NodeList::firstSegmentNode(const int& nodeId) const {
    if(nodeId < 0) return -1;
    if(nodeId >= mNodes.count()) return -1;
    const Node * currNode = mNodes.at(nodeId);
    int smallestId = nodeId;
    int currId = nodeId;
    while(true) {
        if(!currNode->hasPreviousNode()) return currId;
        const int prevId = currNode->getPrevNodeId();
        if(prevId == currId)
            RuntimeThrow("Node points to itself");
        if(prevId == smallestId) return smallestId;
        smallestId = qMin(prevId, smallestId);
        currNode = mNodes.at(prevId);
        if(currNode->isMove()) return currId;
        currId = prevId;
    }
}

int NodeList::lastSegmentNode(const int& nodeId) const {
    if(nodeId < 0) return -1;
    if(nodeId >= mNodes.count()) return -1;
    const Node * currNode = mNodes.at(nodeId);
    int smallestId = nodeId;
    int currId = nodeId;
    while(true) {
        if(currNode->isMove()) return currId;
        const int nextId = currNode->getNextNodeId();
        if(nextId == currId)
            RuntimeThrow("Node points to itself");
        if(nextId == smallestId) return currId;
        smallestId = qMin(nextId, smallestId);
        currNode = mNodes.at(nextId);
        currId = nextId;
    }
}


void NodeList::removeNodeFromList(const int &nodeId) {
    const Node * const nodeToRemove = mNodes.at(nodeId);
    if(nodeToRemove->hasPreviousNode()) {
        setNodeNextId(nodeToRemove->getPrevNodeId(),
                      nodeToRemove->getNextNodeId());
    }
    if(nodeToRemove->hasNextNode()) {
        setNodePrevId(nodeToRemove->getNextNodeId(),
                      nodeToRemove->getPrevNodeId());
    }
    mNodes.removeAt(nodeId);
    for(int i = 0; i < mNodes.count(); i++) {
        Node * const iNode = mNodes[i];
        iNode->shiftIdsGreaterThan(nodeId, -1);
    }
}

Node *NodeList::insertNodeToList(const int &nodeId, const Node &node) {
    mNodes.insert(nodeId, node);
    for(int i = 0; i < mNodes.count(); i++) {
        Node * const iNode = mNodes[i];
        iNode->shiftIdsGreaterThan(nodeId - 1, 1);
    }
    Node * const insertedNode = mNodes[nodeId];
    insertedNode->setNodeId(nodeId);
    return insertedNode;
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
    Node * const firstNode = mNodes[firstNodeId];
    int nextSegId = firstNode->getNextNodeId();
    firstNode->switchPrevAndNext();
    while(const int currNodeId = nextSegId) {
        if(currNodeId == firstNodeId) break;
        Node * const currNode = mNodes[nextSegId];
        nextSegId = currNode->getNextNodeId();
        currNode->switchPrevAndNext();
    }
}

bool NodeList::segmentClosed(const int& nodeId) const {
    const int firstNodeId = firstSegmentNode(nodeId);
    if(firstNodeId == -1) return false;
    const Node * const firstNode = mNodes.at(firstNodeId);
    if(!firstNode->hasPreviousNode()) return false;
    const int prevId = firstNode->getPrevNodeId();
    const Node * const prevNode = mNodes.at(prevId);
    if(prevNode->isMove()) return false;
    return true;
}

int NodeList::insertNodeBefore(const int& nextId,
                               const Node& nodeBlueprint,
                               const Neighbour& neigh) {
    const int insertId = nextId;
    Node * const insertedNode = insertNodeToList(insertId, nodeBlueprint);
    if((neigh & NEXT) && mNext)
        mNext->insertNodeBefore(nextId, Node(), NEXT);
    if((neigh & PREV) && mPrev)
        mPrev->insertNodeBefore(nextId, Node(), PREV);
    insertedNode->setPrevNodeId(-1);
    insertedNode->setNextNodeId(-1);
    const int shiftedNextId = nextId + 1;
    Node * const nextNode = mNodes[shiftedNextId];
    moveNodeBefore(insertId, insertedNode, shiftedNextId, nextNode);
    if(insertedNode->isDissolved())
        promoteDissolvedNodeToNormal(insertId, insertedNode);
    return insertId;
}

int NodeList::insertNodeAfter(const int& prevId,
                              const Node& nodeBlueprint,
                              const Neighbour& neigh) {
    const int insertId = prevId + 1;
    Node * const insertedNode = insertNodeToList(insertId, nodeBlueprint);
    if((neigh & NEXT) && mNext)
        mNext->insertNodeAfter(prevId, Node(), NEXT);
    if((neigh & PREV) && mPrev)
        mPrev->insertNodeAfter(prevId, Node(), PREV);
    insertedNode->setPrevNodeId(-1);
    insertedNode->setNextNodeId(-1);
    Node * const prevNode = mNodes[prevId];
    moveNodeAfter(insertId, insertedNode, prevId, prevNode);
    if(insertedNode->isDissolved())
        promoteDissolvedNodeToNormal(insertId, insertedNode);
    return insertId;
}

int NodeList::appendNode(const Node &nodeBlueprint,
                         const Neighbour& neigh) {
    const int insertId = mNodes.count();
    insertNodeToList(insertId, nodeBlueprint);
    if((neigh & NEXT) && mNext) mNext->appendNode(Node(), NEXT);
    if((neigh & PREV) && mPrev) mPrev->appendNode(Node(), PREV);
    return insertId;
}

void NodeList::promoteDissolvedNodeToNormal(const int& nodeId,
                                            Node * const node) {
    const int prevNormalIdV = prevNormalId(nodeId);
    const int nextNormalIdV = nextNormalId(nodeId);
    Node * const prevNormal = mNodes[prevNormalIdV];
    Node * const nextNormal = mNodes[nextNormalIdV];

    auto seg = gSegmentFromNodes(*prevNormal, *nextNormal);
    auto div = seg.dividedAtT(node->fT);
    const auto& first = div.first;
    const auto& second = div.second;
    prevNormal->fC2 = first.c1();
    node->fC0 = first.c2();
    node->fP1 = first.p1();
    node->fC2 = second.c1();
    setNodeType(nodeId, node, Node::NORMAL);
    setNodeCtrlsMode(nodeId, node, CtrlsMode::CTRLS_SMOOTH);
    nextNormal->fC0 = second.c2();
    for(int i = prevNormalIdV + 1; i < nodeId; i++) {
        Node * const iNode = mNodes[i];
        if(iNode->isDissolved()) {
            iNode->fT = gMapTToFragment(0, node->fT, iNode->fT);
        }
    }
    for(int i = nodeId + 1; i < nextNormalIdV; i++) {
        Node * const iNode = mNodes[i];
        if(iNode->isDissolved()) {
            iNode->fT = gMapTToFragment(node->fT, 1, iNode->fT);
        }
    }
}

void NodeList::promoteDissolvedNodeToNormal(const int& nodeId) {
    promoteDissolvedNodeToNormal(nodeId, at(nodeId));
}

void NodeList::splitNode(const int& nodeId) {
    Node * const node = mNodes[nodeId];
    Node newNode = *node;
    if(node->isNormal()) {
        node->fC2 = node->fP1;
        newNode.fC0 = newNode.fP1;
    }
    insertNodeAfter(nodeId, newNode);
}

void NodeList::splitNodeAndDisconnect(const int& nodeId) {
    splitNode(nodeId);
    insertNodeAfter(nodeId, Node(Node::MOVE));
}

bool NodeList::nodesConnected(const int& node1Id,
                              const int& node2Id) const {
    const Node * const node1 = mNodes.at(node1Id);
    if(node1->getNextNodeId() == node2Id) return true;
    if(node1->getPrevNodeId() == node2Id) return true;
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

SkPath NodeList::toSkPath() const {
    SkPath result;

    const Node * firstNode = nullptr;
    const Node * prevNormalNode = nullptr;

    QList<qreal> dissolvedTs;

    QList<int> srcIds;
    for(int i = 0; i < mNodes.count(); i++) {
        srcIds << i;
    }

    while(!srcIds.isEmpty()) {
        const int firstSrcId = firstSegmentNode(srcIds.first());
        if(firstSrcId == -1) {
            srcIds.removeFirst();
            continue;
        }
        SkPath currPath;
        int nextSrcId = firstSrcId;
        const bool close = segmentClosed(firstSrcId);
        bool move = true;
        while(true) {
            srcIds.removeOne(nextSrcId);
            const Node * const node = at(nextSrcId);

            if(node->isDummy()) {
            } else if(node->isDissolved()) dissolvedTs << node->fT;
            else if(node->isMove()) break;
            else if(node->isNormal()) {
                if(move) {
                    firstNode = node;
                    currPath.moveTo(qPointToSk(node->fP1));
                    qDebug() << "Move to:" << node->fP1;
                    move = false;
                } else {
                    gCubicTo(*prevNormalNode, *node,
                             dissolvedTs, currPath);
                }
                prevNormalNode = node;
            } else {
                RuntimeThrow("Unrecognized node type");
            }

            if(!node->hasNextNode()) break;
            nextSrcId = node->getNextNodeId();
            if(nextSrcId == firstSrcId) break;
        }
        if(!currPath.isEmpty()) {
            if(close) {
                gCubicTo(*prevNormalNode, *firstNode,
                        dissolvedTs, currPath);
                currPath.close();
            }
            result.addPath(currPath);
        }
    }
    return result;
}

qreal NodeList::prevT(const int &nodeId) const {
    const int &prevId = prevNonDummyId(nodeId);
    const Node * const node = mNodes.at(prevId);
    if(node->isNormal()) return 0;
    return node->fT;
}

qreal NodeList::nextT(const int &nodeId) const {
    const int &nextId = nextNonDummyId(nodeId);
    const Node * const node = mNodes.at(nextId);
    if(node->isNormal()) return 1;
    return node->fT;
}

int NodeList::prevNormalId(const int& nodeId) const {
    const Node * currNode = mNodes.at(nodeId);
    while(currNode->hasPreviousNode()) {
        const int currId = currNode->getPrevNodeId();
        currNode = mNodes.at(currId);
        if(currNode->isNormal()) return currId;
    }
    return -1;
}

int NodeList::nextNormalId(const int& nodeId) const {
    const Node * currNode = mNodes.at(nodeId);
    while(currNode->hasNextNode()) {
        const int currId = currNode->getNextNodeId();
        currNode = mNodes.at(currId);
        if(currNode->isNormal()) return currId;
    }
    return -1;
}

int NodeList::prevNonDummyId(const int& nodeId) const {
    const Node * currNode = mNodes.at(nodeId);
    while(currNode->hasPreviousNode()) {
        const int currId = currNode->getPrevNodeId();
        currNode = mNodes.at(currId);
        if(!currNode->isDummy()) return currId;
    }
    return -1;
}

int NodeList::nextNonDummyId(const int& nodeId) const {
    const Node * currNode = mNodes.at(nodeId);
    while(currNode->hasNextNode()) {
        const int currId = currNode->getNextNodeId();
        currNode = mNodes.at(currId);
        if(!currNode->isDummy()) return currId;
    }
    return -1;
}

QList<int> NodeList::updateAllNodesTypeAfterNeighbourChanged() {
    if(mNoUpdates) return QList<int>();
    QList<int> changed;
    for(int i = 0; i < mNodes.count(); i++) {
        if(updateNodeTypeAfterNeighbourChanged(i)) changed << i;
    }
    return changed;
}

bool NodeList::updateNodeTypeAfterNeighbourChanged(const int &nodeId) const {
    if(mNoUpdates) return false;
    Node* node = mNodes[nodeId];
    if(node->isNormal() || node->isMove()) return false;
    Node::Type prevType = Node::DUMMY;
    Node::Type nextType = Node::DUMMY;
    int prevNextId = -1;
    int prevPrevId = -1;
    if(mPrev) {
        const Node * const prevNode = mPrev->at(nodeId);
        prevType = prevNode->getType();
        prevNextId = prevNode->getNextNodeId();
        prevPrevId = prevNode->getPrevNodeId();
    }
    int nextNextId = -1;
    int nextPrevId = -1;
    if(mNext) {
        const Node * const nextNode = mNext->at(nodeId);
        nextType = nextNode->getType();
        nextNextId = nextNode->getNextNodeId();
        nextPrevId = nextNode->getPrevNodeId();
    }
    const int nodeNextId = node->getNextNodeId();
    if(prevType == Node::NORMAL || nextType == Node::NORMAL ||
       prevType == Node::MOVE || nextType == Node::MOVE ||
       ((nodeNextId != nextNextId && nodeNextId != nextPrevId) && nextType != Node::DUMMY) ||
       ((nodeNextId != prevNextId && nodeNextId != prevPrevId) && prevType != Node::DUMMY)) {
        if(node->getType() != Node::DISSOLVED) {
            node->fT = 0.5*(prevT(nodeId) + nextT(nodeId));
            node->setType(Node::DISSOLVED);
            return true;
        }
        return false;
    }
    if(node->getType() != Node::DUMMY) {
        node->setType(Node::DUMMY);
        return true;
    }
    return false;
}
