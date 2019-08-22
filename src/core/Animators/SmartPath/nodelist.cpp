#include "nodelist.h"
#include "skia/skiaincludes.h"
#include "pointhelpers.h"
#include "exceptions.h"
#include "smartPointers/ememory.h"

qCubicSegment2D gSegmentFromNodes(const Node& prevNode,
                                  const Node& nextNode) {
    return qCubicSegment2D(prevNode.p1(), prevNode.c2(),
                           nextNode.c0(), nextNode.p1());
}

void NodeList::moveNode(const int fromId, const int toId) {
    mNodes.moveNode(fromId, toId);
    Node * const node = at(toId);
    if(node->isDissolved()) {
        const qreal prevTV = prevT(toId);
        const qreal tFrag = nextT(toId) - prevTV;
        node->setT(0.5*tFrag + prevTV);
    }
}

void NodeList::updateDissolvedNodePosition(const int nodeId) {
    updateDissolvedNodePosition(nodeId, at(nodeId));
}

void NodeList::updateDissolvedNodePosition(const int nodeId,
                                           Node * const node) {
    if(node->isNormal()) RuntimeThrow("Unsupported node type");
    const Node * const prevNode = prevNormal(nodeId);
    const Node * const nextNode = nextNormal(nodeId);
    const auto normalSeg = gSegmentFromNodes(*prevNode, *nextNode);
    node->mP1 = normalSeg.posAtT(node->mT);
}

bool NodeList::read(eReadStream& src) {
    int nNodes;
    src >> nNodes;
    mNodes.clear();
    for(int i = 0; i < nNodes; i++) {
        Node node;
        src.read(&node, sizeof(Node));
        mNodes.append(node);
    }
    src >> mClosed;
    return true;
}

bool NodeList::write(eWriteStream& dst) const {
    const int nNodes = mNodes.count();
    dst << nNodes;
    for(const auto& node : mNodes) {
        dst.write(node.get(), sizeof(Node));
    }
    dst << mClosed;
    return true;
}

void NodeList::removeNodeFromList(const int nodeId) {
    mNodes.removeAt(nodeId);
}

Node *NodeList::insertNodeToList(const int nodeId, const Node &node) {
    if(nodeId < 0 || nodeId > mNodes.count())
        RuntimeThrow("Wrong insert id");
    mNodes.insert(nodeId, node);
    Node * const insertedNode = mNodes[nodeId];
    return insertedNode;
}

void NodeList::reverse() {
    mNodes.reverse();
}

bool NodeList::isClosed() const {
    return mClosed;
}

int NodeList::insertNodeBefore(const int nextId,
                               const Node& nodeBlueprint) {
    const int insertId = nextId;
    Node * const insertedNode = insertNodeToList(insertId, nodeBlueprint);
    if(nodeBlueprint.isDissolved())
        promoteDissolvedNodeToNormal(insertId, insertedNode);
    return insertId;
}

int NodeList::insertNodeAfter(const int prevId,
                              const Node& nodeBlueprint) {
    const int insertId = prevId + 1;
//    Node * const insertedNode =
   /* const auto node = */insertNodeToList(insertId, nodeBlueprint);
//    if(nodeBlueprint.isDissolved())
//        updateDissolvedNodePosition(insertId, node);
//    if(nodeBlueprint.isDissolved())
//        promoteDissolvedNodeToNormal(insertId, insertedNode);
    return insertId;
}

int NodeList::appendNode(const Node &nodeBlueprint) {
    const int insertId = mNodes.count();
    insertNodeToList(insertId, nodeBlueprint);
    return insertId;
}

void NodeList::approximateBeforeDemoteOrRemoval(
        const qreal nodeT,
        Node * const node,
        Node * const prevNormalV,
        Node * const nextNormalV) {
    if(!node || !prevNormalV || !nextNormalV) return;
    if(!prevNormalV->getC0Enabled()) prevNormalV->setC0Enabled(true);
    if(!nextNormalV->getC2Enabled()) nextNormalV->setC2Enabled(true);

    const auto prevSeg = gSegmentFromNodes(*prevNormalV, *node);
    const QPointF midPrevPt = prevSeg.posAtT(0.5);

    const auto nextSeg = gSegmentFromNodes(*node, *nextNormalV);
    const QPointF midNextPt = nextSeg.posAtT(0.5);

    auto seg = gSegmentFromNodes(*prevNormalV, *nextNormalV);
    seg.makePassThroughRel(node->p1(), nodeT);
    seg.makePassThroughRel(midPrevPt, nodeT*0.5);
    seg.makePassThroughRel(midNextPt, nodeT + 0.5*(1 - nodeT));
    seg.makePassThroughRel(node->p1(), nodeT);

    prevNormalV->setC2(seg.c1());
    nextNormalV->setC0(seg.c2());
}

void NodeList::removeNode(const int nodeId, const bool approx) {
    removeNode(nodeId, at(nodeId), approx);
}

void NodeList::removeNode(const int nodeId, Node * const node,
                          const bool approx) {
    if(node->isNormal()) {
        if(approx) {
            Node * const prevNormalV = prevNormal(nodeId);
            Node * const nextNormalV = nextNormal(nodeId);
            approximateBeforeDemoteOrRemoval(0.5, node,
                                             prevNormalV, nextNormalV);
        }
        Node * currNode = node;
        while(prevNode(currNode)) {
            currNode = prevNode(currNode);
            if(currNode->isNormal()) break;
            if(currNode->isDissolved()) currNode->setT(currNode->t()*0.5);
        }
        currNode = node;
        while(nextNode(currNode)) {
            currNode = nextNode(currNode);
            if(currNode->isNormal()) break;
            if(currNode->isDissolved()) currNode->setT(currNode->t()*0.5 + 0.5);
        }
    }
    removeNodeFromList(nodeId);
}

void NodeList::demoteNormalNodeToDissolved(const int nodeId,
                                           const bool approx) {
    demoteNormalNodeToDissolved(nodeId, at(nodeId), approx);
}

void NodeList::demoteNormalNodeToDissolved(const int nodeId,
                                           Node * const node,
                                           const bool approx) {
    if(node->isDissolved()) return;
    Node * const prevNormalV = prevNormal(nodeId);
    Node * const nextNormalV = nextNormal(nodeId);
    if(!prevNormalV || !nextNormalV) return;

    if(approx) {
        approximateBeforeDemoteOrRemoval(node->t(), node,
                                         prevNormalV, nextNormalV);
    }

    setNodeType(node, Node::DISSOLVED);

    const int prevNormalId = prevNormalV->getNodeId();
    const int nextNormalId = nextNormalV->getNodeId();
    const int nDiss = prevNormalId < nextNormalId ?
                nextNormalId - prevNormalId - 1 :
                count() - 1 - prevNormalId + nextNormalId;
    const qreal tInc = 1./(nDiss + 1);
    qreal iT = tInc;
    Node * iPrevNode = prevNormalV;
    for(int i = 0; i < nDiss; i++) {
        const auto iCurrNode = nextNode(iPrevNode);
        iCurrNode->setT(iT);
        iPrevNode = iCurrNode;
        iT += tInc;
    }

    updateDissolvedNodePosition(nodeId);
}

void NodeList::promoteDissolvedNodeToNormal(const int nodeId,
                                            Node * const node) {
    if(node->isNormal()) return;
    Node * const prevNormalV = prevNormal(nodeId);
    Node * const nextNormalV = nextNormal(nodeId);

    const auto seg = gSegmentFromNodes(*prevNormalV, *nextNormalV);
    const auto div = seg.dividedAtT(node->t());
    const auto& first = div.first;
    const auto& second = div.second;
    prevNormalV->setC2(first.c1());
    node->setC0(first.c2());
    node->setP1(first.p3());
    node->setC2(second.c1());
    setNodeType(node, Node::NORMAL);
    setNodeCtrlsMode(prevNormalV, CtrlsMode::CTRLS_SMOOTH);
    setNodeCtrlsMode(node, CtrlsMode::CTRLS_SMOOTH);
    setNodeCtrlsMode(nextNormalV, CtrlsMode::CTRLS_SMOOTH);
    nextNormalV->setC0(second.c2());
    for(int i = prevNormalV->getNodeId() + 1; i < nodeId; i++) {
        Node * const iNode = mNodes[i];
        if(iNode->isDissolved()) {
            const qreal iT = gMapTToFragment(0, node->t(), iNode->t());
            iNode->setT(iT);
        }
    }
    for(int i = nodeId + 1; i < nextNormalV->getNodeId(); i++) {
        Node * const iNode = mNodes[i];
        if(iNode->isDissolved()) {
            const qreal iT = gMapTToFragment(node->t(), 1, iNode->t());
            iNode->setT(iT);
        }
    }
}

void NodeList::promoteDissolvedNodeToNormal(const int nodeId) {
    promoteDissolvedNodeToNormal(nodeId, at(nodeId));
}

void NodeList::splitNode(const int nodeId) {
    Node * const node = mNodes[nodeId];
    Node newNode = *node;
    if(node->isNormal()) {
        node->mC2 = node->mP1;
        newNode.mC0 = newNode.mP1;
    }
    const int nextNormalIdV = nextNormal(nodeId)->getNodeId();
    if(nextNormalIdV == 0) {
        insertNodeBefore(nextNormalIdV, newNode);
    } else insertNodeAfter(nodeId, newNode);
}

void NodeList::splitNodeAndDisconnect(const int nodeId) {
    splitNode(nodeId);
}

void NodeList::mergeNodes(const int node1Id, const int node2Id) {
    if(!nodesConnected(node1Id, node2Id))
        RuntimeThrow("Only neighbouring connected nodes can be merged");
    const int resId = qMin(node1Id, node2Id);
    const int otherId = qMax(node1Id, node2Id);
    Node * const resultingNode = at(resId);
    Node * const otherNode = at(otherId);
    *resultingNode = Node::sInterpolateNormal(*resultingNode, *otherNode, 0.5);
    resultingNode->setNodeId(node1Id);
    removeNodeFromList(node2Id);
}

bool NodeList::nodesConnected(const int node1Id, const int node2Id) const {
    if(qAbs(node2Id - node1Id) == 1) return true;
    const bool oneIsFirst = node1Id == 0 || node2Id == 0;
    const bool oneIsLast = node1Id == mNodes.count() - 1 ||
                           node2Id == mNodes.count() - 1;
    if(mClosed && oneIsFirst && oneIsLast) return true;
    return false;
}

void gCubicTo(const Node& prevNode, const Node& nextNode,
              QList<qreal>& dissolvedTs, SkPath& result) {
    qCubicSegment2D seg(prevNode.p1(), prevNode.c2(),
                        nextNode.c0(), nextNode.p1());
    qreal lastT = 0;
    for(const qreal t : dissolvedTs) {
        const qreal mappedT = gMapTToFragment(lastT, 1, t);
        auto div = seg.dividedAtT(mappedT);
        const auto& first = div.first;
        result.cubicTo(toSkPoint(first.c1()),
                       toSkPoint(first.c2()),
                       toSkPoint(first.p3()));
        seg = div.second;
        lastT = t;
    }
    result.cubicTo(toSkPoint(seg.c1()),
                   toSkPoint(seg.c2()),
                   toSkPoint(seg.p3()));
    dissolvedTs.clear();
}

SkPath NodeList::toSkPath() const {
    SkPath result;

    const Node * firstNode = nullptr;
    const Node * prevNormalNode = nullptr;

    QList<qreal> dissolvedTs;

    bool move = true;
    for(const auto& node : mNodes) {
        if(node->isDissolved()) dissolvedTs << node->t();
        else if(node->isNormal()) {
            if(move) {
                firstNode = node.get();
                result.moveTo(toSkPoint(node->p1()));
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

void NodeList::setPath(const SkPath &path) {
    mNodes.clear();
    Node * firstNode = nullptr;
    Node * prevNode = nullptr;
    SkPath::RawIter iter = SkPath::RawIter(path);

    SkPoint pts[4];
    int verbId = 0;

    // for converting conics to quads
    SkAutoConicToQuads conicToQuads;
    int quadsCount = 0;
    int quadId = 0;
    SkPoint *ptsT = nullptr;

    SkPath::Verb verbT = iter.next(pts);
    for(;;) {
        switch(verbT) {
            case SkPath::kMove_Verb: {
                if(firstNode) return;
                const QPointF qPt = toQPointF(pts[0]);
                firstNode = appendAndGetNode(Node(qPt));
                prevNode = firstNode;
            }
                break;
            case SkPath::kLine_Verb: {
                const QPointF qPt = toQPointF(pts[1]);

                prevNode->setC2Enabled(false);
                prevNode->mC2 = prevNode->mP1;

                if(iter.peek() == SkPath::kClose_Verb) {
                    firstNode->setC0Enabled(false);
                    firstNode->mC0 = firstNode->mP1;
                } else {
                    prevNode = appendAndGetNode(Node(qPt));
                }
            }
                break;
            case SkPath::kConic_Verb: {
                ptsT = const_cast<SkPoint*>(conicToQuads.computeQuads(
                            pts, iter.conicWeight(), 2));
                quadsCount = conicToQuads.countQuads();
                quadId = 0;
            }
                break;
            case SkPath::kQuad_Verb: {
                const SkPoint ctrlPtT = pts[1];
                pts[1] = pts[0] + (ctrlPtT - pts[0])*0.66667f;
                pts[3] = pts[2];
                pts[2] = pts[3] + (ctrlPtT - pts[3])*0.66667f;
                verbT = SkPath::kCubic_Verb;
                continue;
            }
            case SkPath::kCubic_Verb: {
                const QPointF c0Pt = toQPointF(pts[1]);
                const QPointF c1Pt = toQPointF(pts[2]);
                const QPointF p2Pt = toQPointF(pts[3]);

                prevNode->setC2Enabled(true);
                prevNode->mC2 = c0Pt;
                prevNode->disableUnnecessaryCtrls();
                prevNode->guessCtrlsMode();

                if(iter.peek() == SkPath::kClose_Verb && quadsCount == 0) {
                    firstNode->setC0Enabled(true);
                    firstNode->mC0 = c1Pt;
                } else {
                    prevNode = appendAndGetNode(Node(c1Pt, p2Pt, p2Pt));
                }
            }
                break;
            case SkPath::kClose_Verb:
                setClosed(true);
                break;
            case SkPath::kDone_Verb:
                return;
        }
        if(quadsCount > 0) {
            const int firstPtId = quadId*2;
            pts[0] = ptsT[firstPtId];
            pts[1] = ptsT[firstPtId + 1];
            pts[2] = ptsT[firstPtId + 2];
            verbT = SkPath::kQuad_Verb;
            quadId++;
            quadsCount--;
        } else {
            verbT = iter.next(pts);
            verbId++;
        }
    }
}

qreal NodeList::prevT(const int nodeId) const {
    const Node * const node = prevNode(nodeId);
    if(!node) return 0;
    if(node->isNormal()) return 0;
    return node->t();
}

qreal NodeList::nextT(const int nodeId) const {
    const Node * const node = nextNode(nodeId);
    if(!node) return 1;
    if(node->isNormal()) return 1;
    return node->t();
}

Node * NodeList::prevNormal(const int nodeId) const {
    if(mNodes.count() <= 1) return nullptr;
    Node * currNode = mNodes.at(nodeId);
    while(prevNode(currNode)) {
        currNode = prevNode(currNode);
        if(currNode->getNodeId() == nodeId) return nullptr;
        if(currNode->isNormal()) return currNode;
    }
    return nullptr;
}

Node * NodeList::nextNormal(const int nodeId) const {
    if(mNodes.count() <= 1) return nullptr;
    Node * currNode = mNodes.at(nodeId);
    while(nextNode(currNode)) {
        currNode = nextNode(currNode);
        if(currNode->getNodeId() == nodeId) return nullptr;
        if(currNode->isNormal()) return currNode;
    }
    return nullptr;
}

NodeList NodeList::sInterpolate(const NodeList &list1,
                                const NodeList &list2,
                                const qreal weight2) {
    if(list1.count() != list2.count())
        RuntimeThrow("Cannot interpolate paths with different node count");
    if(list1.isClosed() != list2.isClosed())
        RuntimeThrow("Cannot interpolate a closed path with an open path.");
    auto list1Cpy = list1.createDeepCopy();
    auto list2Cpy = list2.createDeepCopy();
    const bool closed = list1Cpy.isClosed();
    NodeList result;
    result.setClosed(closed);
    ListOfNodes& resultList = result.getList();
    const int listCount = list1Cpy.count();
    for(int i = 0; i < listCount; i++) {
        const Node * const node1 = list1Cpy.at(i);
        const Node * const node2 = list2Cpy.at(i);
        if(node1->getType() == node2->getType()) continue;
        if(node1->isDissolved()) {
            list1Cpy.promoteDissolvedNodeToNormal(i);
        } else if(node2->isDissolved()) {
            list2Cpy.promoteDissolvedNodeToNormal(i);
        } else RuntimeThrow("Nodes with different type should not happen");
    }
    for(int i = 0; i < listCount; i++) {
        const Node * const node1 = list1Cpy.at(i);
        const Node * const node2 = list2Cpy.at(i);
        if(node1->isNormal() && node2->isNormal()) {
            const auto normalInter = Node::sInterpolateNormal(
                        *node1, *node2, weight2);
            resultList.append(normalInter);
        } else if(node1->isDissolved() && node2->isDissolved()) {
            const auto dissInter = Node::sInterpolateDissolved(
                        *node1, *node2, weight2);
            resultList.append(dissInter);
        } else RuntimeThrow("Nodes with different type should not happen");
    }

    return result;
}
