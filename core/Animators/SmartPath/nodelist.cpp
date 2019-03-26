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

void NodeList::moveNode(const int& fromId, const int& toId) {
    mNodes.moveNode(fromId, toId);
}

void NodeList::updateDissolvedNodePosition(const int &nodeId) {
    updateDissolvedNodePosition(nodeId, at(nodeId));
}

void NodeList::updateDissolvedNodePosition(const int &nodeId,
                                           Node * const node) {
    if(node->isNormal()) RuntimeThrow("Unsupported node type");
    const Node * const prevNode = prevNormal(nodeId);
    const Node * const nextNode = nextNormal(nodeId);
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
    src->read(rcChar(&mClosed), sizeof(bool));
    return true;
}

bool NodeList::write(QIODevice * const dst) const {
    const int nNodes = mNodes.count();
    dst->write(rcConstChar(&nNodes), sizeof(int));
    for(const auto& node : mNodes) {
        dst->write(rcConstChar(node.get()), sizeof(Node));
    }
    dst->write(rcConstChar(&mClosed), sizeof(bool));
    return true;
}

void NodeList::removeNodeFromList(const int &nodeId) {
    mNodes.removeAt(nodeId);
    updateNodeIds(nodeId, mNodes.count() - 1);
}

void NodeList::updateNodeIds() {
    updateNodeIds(0, mNodes.count() - 1);
}

void NodeList::updateNodeIds(const int& minId, const int& maxId) {
    for(int i = minId; i <= maxId; i++) {
        Node * const iNode = mNodes[i];
        iNode->setNodeId(i);
    }
}

Node *NodeList::insertNodeToList(const int &nodeId, const Node &node) {
    if(nodeId < 0 || nodeId > mNodes.count())
        RuntimeThrow("Wrong insert id");
    mNodes.insert(nodeId, node);
    updateNodeIds(nodeId, mNodes.count() - 1);
    Node * const insertedNode = mNodes[nodeId];
    return insertedNode;
}

void NodeList::reverseSegment() {
    mNodes.reverse();
}

bool NodeList::isClosed() const {
    return mClosed;
}

int NodeList::insertNodeBefore(const int& nextId,
                               const Node& nodeBlueprint) {
    const int insertId = nextId;
    Node * const insertedNode = insertNodeToList(insertId, nodeBlueprint);
    if(nodeBlueprint.isDissolved())
        promoteDissolvedNodeToNormal(insertId, insertedNode);
    return insertId;
}

int NodeList::insertNodeAfter(const int& prevId,
                              const Node& nodeBlueprint) {
    const int insertId = prevId + 1;
    Node * const insertedNode = insertNodeToList(insertId, nodeBlueprint);
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
    Node * const prevNormalV = prevNormal(nodeId);
    Node * const nextNormalV = nextNormal(nodeId);

    auto seg = gSegmentFromNodes(*prevNormalV, *nextNormalV);
    auto div = seg.dividedAtT(node->fT);
    const auto& first = div.first;
    const auto& second = div.second;
    prevNormalV->fC2 = first.c1();
    node->fC0 = first.c2();
    node->fP1 = first.p3();
    node->fC2 = second.c1();
    setNodeType(node, Node::NORMAL);
    setNodeCtrlsMode(node, CtrlsMode::CTRLS_SMOOTH);
    nextNormalV->fC0 = second.c2();
    for(int i = prevNormalV->getNodeId() + 1; i < nodeId; i++) {
        Node * const iNode = mNodes[i];
        if(iNode->isDissolved()) {
            iNode->fT = gMapTToFragment(0, node->fT, iNode->fT);
        }
    }
    for(int i = nodeId + 1; i < nextNormalV->getNodeId(); i++) {
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
    const int nextNormalIdV = nextNormal(nodeId)->getNodeId();
    if(nextNormalIdV == 0) {
        insertNodeBefore(nextNormalIdV, newNode);
    } else insertNodeAfter(nodeId, newNode);
}

void NodeList::splitNodeAndDisconnect(const int& nodeId) {
    splitNode(nodeId);
}

bool NodeList::nodesConnected(const int& node1Id, const int& node2Id) const {
    if(qAbs(node2Id - node1Id) == 1) return true;
    const bool oneIsFirst = node1Id == 0 || node2Id == 0;
    const bool oneIsLast = node1Id == mNodes.count() - 1 ||
                           node2Id == mNodes.count() - 1;
    if(mClosed && oneIsFirst && oneIsLast) return true;
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

Node * NodeList::prevNormal(const int& nodeId) const {
    Node * currNode = mNodes.at(nodeId);
    while(prevNode(currNode)) {
        currNode = prevNode(currNode);
        if(currNode->isNormal()) return currNode;
    }
    return nullptr;
}

Node * NodeList::nextNormal(const int& nodeId) const {
    Node * currNode = mNodes.at(nodeId);
    while(nextNode(currNode)) {
        currNode = nextNode(currNode);
        if(currNode->isNormal()) return currNode;
    }
    return nullptr;
}

NodeList NodeList::sInterpolate(const NodeList &list1,
                                const NodeList &list2,
                                const qreal& weight2) {
    if(list1.count() != list2.count())
        RuntimeThrow("Cannot interpolate paths with different node count");
    if(list1.isClosed() != list2.isClosed())
        RuntimeThrow("Cannot interpolate a closed path with an open path.");
    const bool closed = list1.isClosed();
    ListOfNodes resultList;
    const auto list1v = list1.getList();
    const auto list2v = list2.getList();
    const int listCount = list1v.count();
    for(int i = 0; i < listCount; i++) {
        const Node * const node1 = list1v.at(i);
        const Node * const node2 = list2v.at(i);
        resultList.append(Node::sInterpolate(*node1, *node2, weight2));
    }
    NodeList result;
    result.shallowCopyNodeList(resultList);
    result.updateNodeIds();
    result.setClosed(closed);
    return result;
}
