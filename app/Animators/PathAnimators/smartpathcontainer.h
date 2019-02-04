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



    static Node sCreateDissolved(Node &prevNode, Node &nextNode, const qreal& t) {
        Node result;
        result.fT = t;
        gGetValuesForNodeInsertion(prevNode.fP1, prevNode.fC2,
                                   result.fC0, result.fP1, result.fC2,
                                   nextNode.fC0, nextNode.fP1, t);
        return result;
    }

    QPointF fC0;
    QPointF fP1;
    QPointF fC2;
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
    DissolveNode(const int& nodeId, const qreal& t) :
        NodeAction(DISSOLVE), mNodeId(nodeId), mT(t) {}

    void apply(QList<Node>& nodes) const {
        Node& node = nodes[mNodeId];
        node.fType = Node::DISSOLVED;
        node.fT = mT;
        // fix t values for surrounding dissolved nodes
        for(int i = mNodeId - 1; i >= 0; i--) {
            Node& iNode = nodes[i];
            if(iNode.isNormal()) break;
            iNode.fT = mT*iNode.fT;
        }
        for(int i = mNodeId + 1; i < nodes.count(); i++) {
            Node& iNode = nodes[i];
            if(iNode.isNormal()) break;
            iNode.fT = mT + (1 - mT)*iNode.fT;
        }
    }
private:
    int mNodeId;
    qreal mT;
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
        Node prevNode = nodes.at(mNode1Id);
        Node nextNode = nodes.at(mNode2Id);
        Node newNode = Node::sCreateDissolved(prevNode, nextNode, mT);
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
