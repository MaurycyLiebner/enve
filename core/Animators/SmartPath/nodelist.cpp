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
    setNodeNextId(afterNode, moveNodeId);
    setNodePrevAndNextId(moveNode, afterNodeId, afterNextId);
    setNodePrevId(afterNextId, moveNodeId);
}

void NodeList::moveNodeBefore(const int& moveNodeId, Node * const moveNode,
                              const int& beforeNodeId, Node * const beforeNode) {
    const int movePrevId = moveNode->getPrevNodeId();
    const int moveNextId = moveNode->getNextNodeId();
    setNodeNextId(movePrevId, moveNextId);
    setNodePrevId(moveNextId, movePrevId);

    const int beforePrevId = beforeNode->getPrevNodeId();
    setNodePrevId(beforeNode, moveNodeId);
    setNodePrevAndNextId(moveNode, beforePrevId, beforeNodeId);
    setNodeNextId(beforePrevId, moveNodeId);
}

void NodeList::updateDissolvedNodePosition(const int &nodeId) {
    updateDissolvedNodePosition(nodeId, at(nodeId));
}

void NodeList::updateDissolvedNodePosition(const int &nodeId,
                                           Node * const node) {
    if(node->isNormal()) RuntimeThrow("Unsupported node type");
    const int prevId = prevNormalId(nodeId);
    const int nextId = nextNormalId(nodeId);
    const Node * const prevNode = at(prevId);
    const Node * const nextNode = at(nextId);
    const auto normalSeg = gSegmentFromNodes(*prevNode, *nextNode);
    node->fP1 = normalSeg.posAtT(node->fT);
}

bool NodeList::read(QIODevice * const src) {
    int nNodes;
    src->read(rcChar(&nNodes), sizeof(int));
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

void NodeList::removeNodeFromList(const int &nodeId) {
    const Node * const nodeToRemove = mNodes.at(nodeId);
    if(nodeToRemove->getPrevNode()) {
        setNodeNextId(nodeToRemove->getPrevNodeId(),
                      nodeToRemove->getNextNodeId());
    }
    if(nodeToRemove->getNextNode()) {
        setNodePrevId(nodeToRemove->getNextNodeId(),
                      nodeToRemove->getPrevNodeId());
    }
    mNodes.removeAt(nodeId);
    updateNodeIds();
}

void NodeList::updateNodeIds() {
    for(int i = 0; i < mNodes.count(); i++) {
        Node * const iNode = mNodes[i];
        iNode->setNodeId(i);
    }
}

Node *NodeList::insertNodeToList(const int &nodeId, const Node &node) {
    mNodes.insert(nodeId, node);
    updateNodeIds();
    Node * const insertedNode = mNodes[nodeId];
    insertedNode->setNodeId(nodeId);
    return insertedNode;
}

void NodeList::reverseSegment() {
    mNodes.reverse();
}

bool NodeList::isClosed() const {
    if(mNodes.isEmpty()) return false;
    return mNodes.last()->getNextNode();
}

int NodeList::insertNodeBefore(const int& nextId,
                               const Node& nodeBlueprint) {
    const int insertId = nextId;
    Node * const insertedNode = insertNodeToList(insertId, nodeBlueprint);
    const int shiftedNextId = nextId + 1;
    Node * const nextNode = mNodes[shiftedNextId];
    moveNodeBefore(insertId, insertedNode, shiftedNextId, nextNode);
    if(nodeBlueprint.isDissolved())
        promoteDissolvedNodeToNormal(insertId, insertedNode);
    return insertId;
}

int NodeList::insertNodeAfter(const int& prevId,
                              const Node& nodeBlueprint) {
    Node * const prevNode = mNodes[prevId];
    if(prevNode->getNextNode())
        return insertNodeBefore(prevId, nodeBlueprint);
    const int insertId = prevId + 1;
    Node * const insertedNode = insertNodeToList(insertId, nodeBlueprint);

    moveNodeAfter(insertId, insertedNode, prevId, prevNode);

    if(nodeBlueprint.isDissolved())
        promoteDissolvedNodeToNormal(insertId, insertedNode);
    return insertId;
}

int NodeList::appendNode(const Node &nodeBlueprint) {
    const int insertId = mNodes.count();
    insertNodeToList(insertId, nodeBlueprint);
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
    node->fP1 = first.p3();
    node->fC2 = second.c1();
    setNodeType(node, Node::NORMAL);
    setNodeCtrlsMode(node, CtrlsMode::CTRLS_SMOOTH);
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
    const int nextNormalIdV = nextNormalId(nodeId);
    if(nextNormalIdV == 0) {
        insertNodeBefore(nextNormalIdV, newNode);
    } else insertNodeAfter(nodeId, newNode);
}

void NodeList::splitNodeAndDisconnect(const int& nodeId) {
    splitNode(nodeId);
}

bool NodeList::nodesConnected(const int& node1Id, const int& node2Id) const {
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
        result.cubicTo(toSkPoint(first.c1()),
                       toSkPoint(first.c2()),
                       toSkPoint(first.p3()));
        //qDebug() << "Cubic to:" << first.p1();
        seg = div.second;
        lastT = t;
    }
    result.cubicTo(toSkPoint(seg.c1()),
                   toSkPoint(seg.c2()),
                   toSkPoint(seg.p3()));
    //qDebug() << "Cubic to:" << seg.p1();
    dissolvedTs.clear();
}

SkPath NodeList::toSkPath() const {
    SkPath result;

    const Node * firstNode = nullptr;
    const Node * prevNormalNode = nullptr;

    QList<qreal> dissolvedTs;

    bool move = true;
    for(const auto& node : mNodes) {
        if(node->isDissolved()) dissolvedTs << node->fT;
        else if(node->isNormal()) {
            if(move) {
                firstNode = node.get();
                result.moveTo(toSkPoint(node->fP1));
                move = false;
            } else {
                gCubicTo(*prevNormalNode, *node,
                         dissolvedTs, result);
            }
            prevNormalNode = node.get();
        } else {
            RuntimeThrow("Unrecognized node type");
        }
    }
    if(isClosed()) {
        gCubicTo(*prevNormalNode, *firstNode, dissolvedTs, result);
        result.close();
    }
    return result;
}

qreal NodeList::prevT(const int &nodeId) const {
    if(nodeId == 0) return 0;
    const Node * const node = mNodes.at(nodeId - 1);
    if(node->isNormal()) return 0;
    return node->fT;
}

qreal NodeList::nextT(const int &nodeId) const {
    if(nodeId == mNodes.count() - 1) return 1;
    const Node * const node = mNodes.at(nodeId + 1);
    if(node->isNormal()) return 1;
    return node->fT;
}

int NodeList::prevNormalId(const int& nodeId) const {
    const Node * currNode = mNodes.at(nodeId);
    while(currNode->getPrevNode()) {
        const int currId = currNode->getPrevNodeId();
        currNode = mNodes.at(currId);
        if(currNode->isNormal()) return currId;
    }
    return -1;
}

int NodeList::nextNormalId(const int& nodeId) const {
    const Node * currNode = mNodes.at(nodeId);
    while(currNode->getNextNode()) {
        const int currId = currNode->getNextNodeId();
        currNode = mNodes.at(currId);
        if(currNode->isNormal()) return currId;
    }
    return -1;
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
        resultList.append(Node::sInterpolate(*node1, *node2, weight2));
    }
    return NodeList(resultList);
}
