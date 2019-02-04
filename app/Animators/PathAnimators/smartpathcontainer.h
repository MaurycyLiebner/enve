#ifndef SMARTPATHCONTAINER_H
#define SMARTPATHCONTAINER_H
#include "simplemath.h"
#include "skia/skiaincludes.h"
#include "exceptions.h"
#include "pointhelpers.h"

#define EQUAL_3_OR(val, or1, or2, or3) val == or1 || val == or2 || val == or3

struct Node {
    enum Type {
        NORMAL_MIDDLE, NORMAL_START, NORMAL_END,
        NORMAL_START_AND_END, DISSOLVED, INVALID
    };

    Node() {}

    Node(const QPointF& c0, const QPointF& p1, const QPointF& c2,
         const Type& type) {
        fC0 = c0;
        fP1 = p1;
        fC2 = c2;
        fType = type;
    }

    Node(const QPointF& c0, const QPointF& p1, const QPointF& c2,
         const qreal& t) : Node(c0, p1, c2, DISSOLVED) {
        fT = t;
    }

    bool isNormal() const {
        return EQUAL_3_OR(fType, NORMAL_MIDDLE, NORMAL_START, NORMAL_END);
    }


    QPointF fC0;
    QPointF fP1;
    QPointF fC2;

    //! @brief T value for segment defined by previous and next normal nodes
    qreal fT;
    Type fType = INVALID;
};

class NodeAction {
public:
    enum Type {
        MERGE, SPLIT,
        DISCONNECT, CONNECT,
        DISSOLVE, REMOVE_DISSOLVED, ADD_DISSOLVED,
        NORMALIZE_DISSOLVED
    };

    NodeAction(const Type& type) : mType(type) {

    }

    const Type& getType() const {
        return mType;
    }

    virtual void apply(QList<Node>& nodes) const = 0;
private:
    Type mType;
};

Node& prevNormalNodeRef(const int& id, QList<Node>& list,
                        int *resultId = nullptr) {
    for(int i = id - 1; i >= 0; i--) {
        Node& node = list[i];
        if(node.isNormal()) {
            if(resultId) *resultId = i;
            return node;
        }
    }
    RuntimeThrow("No previus normal node found");
}

Node& nextNormalNodeRef(const int& id, QList<Node>& list,
                        int *resultId = nullptr) {
    for(int i = id + 1; i < list.count(); i++) {
        Node& node = list[i];
        if(node.isNormal()) {
            if(resultId) *resultId = i;
            return node;
        }
    }
    RuntimeThrow("No next normal node found");
}

Node prevNormalNode(const int& id, const QList<Node>& list,
                    int *resultId = nullptr) {
    for(int i = id - 1; i >= 0; i--) {
        const Node& node = list.at(i);
        if(node.isNormal()) {
            if(resultId) *resultId = i;
            return node;
        }
    }
    if(resultId) *resultId = -1;
    return Node();
}

Node nextNormalNode(const int& id, const QList<Node>& list,
                    int *resultId = nullptr) {
    for(int i = id + 1; i < list.count(); i++) {
        const Node& node = list.at(i);
        if(node.isNormal()) {
            if(resultId) *resultId = i;
            return node;
        }
    }
    if(resultId) *resultId = -1;
    return Node();
}

class MergeNodesAction : public NodeAction {
public:
    MergeNodesAction(const int& firstNodeId, const int& lastNodeId) :
        NodeAction(MERGE), mFirstNodeId(firstNodeId), mLastNodeId(lastNodeId) {}

    void apply(QList<Node>& nodes) const {
        if(mFirstNodeId >= mLastNodeId)
            RuntimeThrow("Invalid merging range");
        auto& firstNode = nodes[mFirstNodeId];
        auto& lastNode = nodes[mLastNodeId];
        firstNode.fC2 = lastNode.fC2;
        if(lastNode.fType == Node::NORMAL_END) {
            if(firstNode.fType == Node::NORMAL_START) {
                firstNode.fType = Node::NORMAL_START_AND_END;
            } else {
                firstNode.fType = Node::NORMAL_END;
            }
        }
        for(int i = mFirstNodeId + 1; i <= mLastNodeId; i++) {
            nodes.removeAt(i);
        }
    }
private:
    int mFirstNodeId;
    int mLastNodeId;
};

class DisconnectNodeAction : public NodeAction {
public:
    DisconnectNodeAction(const int& node1Id, const int& node2Id) :
        NodeAction(DISCONNECT), mNode1Id(node1Id), mNode2Id(node2Id) {}

    void apply(QList<Node>& nodes) const {
        if(mNode1Id + 1 != mNode2Id)
            RuntimeThrow("Cannot disconnect non-adjacent nodes");
        Node& node1 = nodes[mNode1Id];
        Node& node2 = nodes[mNode2Id];
        if(node1.fType == Node::NORMAL_START) {
            node1.fType = Node::NORMAL_START_AND_END;
        } else {
            node1.fType = Node::NORMAL_END;
        }

        if(node2.fType == Node::NORMAL_END) {
            node2.fType = Node::NORMAL_START_AND_END;
        } else {
            node2.fType = Node::NORMAL_START;
        }
    }
private:
    int mNode1Id;
    int mNode2Id;
};

class ConnectNodeAction : public NodeAction {
public:
    ConnectNodeAction(const int& node1Id, const int& node2Id) :
        NodeAction(CONNECT), mNode1Id(node1Id), mNode2Id(node2Id) {}

    void apply(QList<Node>& nodes) const {
        if(mNode1Id + 1 != mNode2Id)
            RuntimeThrow("Cannot disconnect non-adjacent nodes");
        Node& node1 = nodes[mNode1Id];
        Node& node2 = nodes[mNode2Id];
        if(node1.fType == Node::NORMAL_START_AND_END) {
            node1.fType = Node::NORMAL_START;
        } else {
            node1.fType = Node::NORMAL_MIDDLE;
        }

        if(node2.fType == Node::NORMAL_START_AND_END) {
            node2.fType = Node::NORMAL_END;
        } else {
            node2.fType = Node::NORMAL_MIDDLE;
        }
    }
private:
    int mNode1Id;
    int mNode2Id;
};

class SplitNodeAction : public NodeAction {
public:
    SplitNodeAction(const int& nodeId,
                    const int& duplicatesBefore,
                    const int& duplicatesAfter,
                    const bool& disconnect) :
        NodeAction(SPLIT), mNodeId(nodeId),
        mDuplicatesBefore(duplicatesBefore),
        mDuplicatesAfter(duplicatesAfter),
        mDisconnect(disconnect) {}

    // !!! should also handle NORMAL_END and NORMAL_START nodes
    void apply(QList<Node>& nodes) const {
        Node toSplit = nodes.takeAt(mNodeId);

        const int iMax = mDuplicatesBefore + 1 + mDuplicatesAfter;
        for(int i = 0; i <= iMax; i++) {
            Node node;
            node.fC0 = (i == 0 ? toSplit.fC0 : toSplit.fP1);
            node.fP1 = toSplit.fP1;
            node.fC2 = (i == iMax ? toSplit.fC2 : toSplit.fP1);
            node.fType = Node::NORMAL_MIDDLE;
            nodes.insert(mNodeId, node);
        }

        Node& split1 = nodes[mDuplicatesBefore + mNodeId];
        Node& split2 = nodes[mDuplicatesBefore + mNodeId + 1];
        if(mDisconnect) {
            if(toSplit.fType == Node::DISSOLVED)
                RuntimeThrow("SplitNodeAction: Unsupported Node Type DISSOLVED");
            if(split1.fType == Node::NORMAL_START) {
                split1.fType = Node::NORMAL_START_AND_END;
            } else {
                split1.fType = Node::NORMAL_END;
            }

            if(split2.fType == Node::NORMAL_END) {
                split2.fType = Node::NORMAL_START_AND_END;
            } else {
                split2.fType = Node::NORMAL_START;
            }
        }
    }
private:
    int mNodeId;
    int mDuplicatesBefore;
    int mDuplicatesAfter;
    bool mDisconnect;
};

class DissolveNode : public NodeAction {
public:
    DissolveNode(const int& nodeId) :
        NodeAction(DISSOLVE), mNodeId(nodeId) {}

    void apply(QList<Node>& nodes) const {
        Node& node = nodes[mNodeId];
        Node& prevNormal = prevNormalNodeRef(mNodeId, nodes);
        Node& nextNormal = nextNormalNodeRef(mNodeId, nodes);
        qCubicSegment2D seg(prevNormal.fP1, prevNormal.fC2,
                            nextNormal.fC0, nextNormal.fP1);
        const qreal t = seg.tAtPos(node.fP1);
        node.fType = Node::DISSOLVED;
        node.fT = t;
        // fix t values for surrounding dissolved nodes
        for(int i = mNodeId - 1; i >= 0; i--) {
            Node& iNode = nodes[i];
            if(iNode.isNormal()) break;
            iNode.fT = t*iNode.fT;
        }
        for(int i = mNodeId + 1; i < nodes.count(); i++) {
            Node& iNode = nodes[i];
            if(iNode.isNormal()) break;
            iNode.fT = t + (1 - t)*iNode.fT;
        }
    }
private:
    int mNodeId;
};

class RemoveDissolvedNode : public NodeAction {
public:
    RemoveDissolvedNode(const int& nodeId) :
        NodeAction(REMOVE_DISSOLVED), mNodeId(nodeId) {}

    void apply(QList<Node>& nodes) const {
        const Node& node = nodes[mNodeId];
        if(node.fType != Node::DISSOLVED)
            RuntimeThrow("RemoveDissolvedNode: Unsupported Node Type");
        Node& prevNode = nodes[mNodeId - 1];
        Node& nextNode = nodes[mNodeId + 1];
        gGetValuesForNodeRemoval(prevNode.fP1, prevNode.fC2,
                                 node.fC0, node.fP1, node.fC2,
                                 nextNode.fC0, nextNode.fP1,
                                 node.fT);
        nodes.removeAt(mNodeId);
    }
private:
    int mNodeId;
};

class AddDissolvedNode : public NodeAction {
public:
    AddDissolvedNode(const int& node1Id, const int& node2Id, const qreal& t) :
        NodeAction(ADD_DISSOLVED), mNode1Id(node1Id), mNode2Id(node2Id), mT(t) {}

    void apply(QList<Node>& nodes) const {
        if(mNode1Id + 1 != mNode2Id)
            RuntimeThrow("Cannot add node between non-adjacent nodes");
        Node &prevNode = nodes[mNode1Id];
        Node &nextNode = nodes[mNode2Id];
        Node newNode;
        newNode.fT = mT;
        const qreal prevT = prevNode.isNormal() ? 0 : prevNode.fT;
        const qreal nextT = nextNode.isNormal() ? 1 : nextNode.fT;

        const qreal mappedT = gMapTToFragment(prevT, nextT, mT);
        gGetValuesForNodeInsertion(prevNode.fP1, prevNode.fC2,
                                   newNode.fC0, newNode.fP1, newNode.fC2,
                                   nextNode.fC0, nextNode.fP1, mappedT);
        nodes.insert(mNode1Id + 1, newNode);
    }
private:
    int mNode1Id;
    int mNode2Id;
    qreal mT;
};

class NormalizeDissolvedNode : public NodeAction {
public:
    NormalizeDissolvedNode(const int& nodeId) :
        NodeAction(NORMALIZE_DISSOLVED), mNodeId(nodeId) {}

    void apply(QList<Node>& nodes) const {
        Node& node = nodes[mNodeId];
        if(node.fType != Node::DISSOLVED)
             RuntimeThrow("NormalizeDissolvedNode: Unsupported Node Type");
        node.fType = Node::NORMAL_MIDDLE;
        // fix t values for surrounding dissolved nodes
        for(int i = mNodeId - 1; i >= 0; i--) {
            Node& iNode = nodes[i];
            if(iNode.isNormal()) break;
            iNode.fT = gMapTToFragment(0, node.fT, iNode.fT);
        }
        for(int i = mNodeId + 1; i < nodes.count(); i++) {
            Node& iNode = nodes[i];
            if(iNode.isNormal()) break;
            iNode.fT = gMapTToFragment(node.fT, 1, iNode.fT);
        }
    }
private:
    int mNodeId;
};
class ActionsApplier {
public:


};

#endif // SMARTPATHCONTAINER_H
