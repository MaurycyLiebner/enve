#include "smartpathcontainer.h"

SmartPath::SmartPath() : mNodesList() {}

void SmartPath::actionRemoveNode(const int &nodeId) {
    Node * const node = mNodesList.at(nodeId);
    if(node->isNormal()) {
        Node * currNode = node;
        while(mNodesList.prevNode(currNode)) {
            currNode = mNodesList.prevNode(currNode);
            if(currNode->isNormal()) break;
            if(currNode->isDissolved()) currNode->fT *= 0.5;
        }
        currNode = node;
        while(mNodesList.nextNode(currNode)) {
            currNode = mNodesList.nextNode(currNode);
            if(currNode->isNormal()) break;
            if(currNode->isDissolved()) currNode->fT = currNode->fT*0.5 + 0.5;
        }
    }
    mNodesList.removeNodeFromList(nodeId);
}

int SmartPath::actionAddFirstNode(const QPointF &c0,
                                  const QPointF &p1,
                                  const QPointF &c2) {
    const int insertId = mNodesList.appendNode(Node(c0, p1, c2));
    return insertId;
}

int SmartPath::actionAppendNodeAtEndNode(const int &endNodeId) {
    Node * const endNode = mNodesList.at(endNodeId);
    if(!endNode->isNormal())
        RuntimeThrow("Invalid node type. "
                     "End nodes should always be NORMAL.");
    const NodePointValues vals = {endNode->fP1, endNode->fP1, endNode->fP1};
    return actionAppendNodeAtEndNode(endNodeId, vals);
}

int SmartPath::actionAppendNodeAtEndNode(const int &endNodeId,
                                         const NodePointValues &values) {
    Node * const endNode = mNodesList.at(endNodeId);
    if(!endNode->isNormal())
        RuntimeThrow("Invalid node type. "
                     "End nodes should always be NORMAL.");
    return mNodesList.insertNodeAfter(endNodeId,
                                      Node(values.fC0, values.fP1, values.fC2));
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

int SmartPath::actionInsertNodeBetween(
        const int &prevId, const int& nextId,
        const QPointF &c0, const QPointF &p1, const QPointF &c2) {
    return insertNodeBetween(prevId, nextId, Node(c0, p1, c2));
}

void SmartPath::actionPromoteDissolvedNodeToNormal(const int &nodeId) {
    mNodesList.promoteDissolvedNodeToNormal(nodeId);
}

void SmartPath::actionMoveNodeBetween(const int& movedNodeId,
                                      const int& prevNodeId,
                                      const int& nextNodeId) {
    if(!mNodesList.nodesConnected(prevNodeId, nextNodeId))
        RuntimeThrow("Trying to move between not connected nodes");
    mNodesList.moveNode(movedNodeId, nextNodeId);
}

void SmartPath::actionDisconnectNodes(const int &node1Id, const int &node2Id) {
    int prevId;
    int nextId;
    if(nextNodeId(node1Id) == node2Id) {
        prevId = node1Id;
        nextId = node2Id;
    } else if(nextNodeId(node2Id) == node1Id) {
        prevId = node2Id;
        nextId = node1Id;
    } else {
        RuntimeThrow("Trying to disconnect not connected nodes");
    }

    Node * const prevNode = mNodesList.at(prevId);
    if(!prevNode->isNormal()) {
        Node * const prevNormalNode = mNodesList.prevNormal(prevId);
        mNodesList.setNodeType(prevNode, Node::NORMAL);
        prevNode->fC0 = prevNormalNode->fC0;
        prevNode->fP1 = prevNormalNode->fP1;
        prevNode->fC2 = prevNormalNode->fC2;
    }
    Node * const nextNode = mNodesList.at(nextId);
    if(!nextNode->isNormal()) {
        Node * const nextNormalNode = mNodesList.nextNormal(nextId);
        mNodesList.setNodeType(nextNode, Node::NORMAL);
        nextNode->fC0 = nextNormalNode->fC0;
        nextNode->fP1 = nextNormalNode->fP1;
        nextNode->fC2 = nextNormalNode->fC2;
    }

    if(isClosed()) {
        mNodesList.moveNodesToFrontStartingWith(nextId);
        mNodesList.setClosed(false);
    } else {
        mLastDetached = mNodesList.detachNodesStartingWith(nextId);
    }
}

void SmartPath::actionConnectNodes(const int &node1Id, const int &node2Id) {
    if((node1Id == 0 && node2Id == mNodesList.count() - 1) ||
       (node2Id == 0 && node1Id == mNodesList.count() - 1)) {
        mNodesList.setClosed(true);
    } else RuntimeThrow("Only first and last node can be connected");
}

SkPath SmartPath::getPathAt() const {
    return mNodesList.toSkPath();
}

void SmartPath::setPath(const SkPath &path) {
    mNodesList.setPath(path);
}

ValueRange SmartPath::dissolvedTRange(const int &nodeId) {
    return {mNodesList.prevT(nodeId), mNodesList.nextT(nodeId)};
}
