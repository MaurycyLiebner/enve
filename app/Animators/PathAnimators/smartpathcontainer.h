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
        fRelT = t;
    }

    bool isNormal() const {
        return EQUAL_3_OR(fType, NORMAL_MIDDLE, NORMAL_START, NORMAL_END);
    }

    QPointF fC0;
    QPointF fP1;
    QPointF fC2;

    //! @brief T value for segment defined by previous and next node
    qreal fRelT;
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

// !!! should be able to marge last node with first node
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
        if(!node1.isNormal() || !node2.isNormal())
            RuntimeThrow("Unsupported node type(s)");
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
            RuntimeThrow("Cannot connect non-adjacent nodes");
        Node& node1 = nodes[mNode1Id];
        Node& node2 = nodes[mNode2Id];
        if(!node1.isNormal() || !node2.isNormal())
            RuntimeThrow("Unsupported node type(s)");
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
    DissolveNode(const int& nodeId, const qreal& t) :
        NodeAction(DISSOLVE), mNodeId(nodeId), mT(t) {}

    void apply(QList<Node>& nodes) const {
        Node& node = nodes[mNodeId];

        node.fType = Node::DISSOLVED;
        node.fRelT = mT;
//        Node& prevNode = nodes[mNodeId - 1];
//        Node& nextNode = nodes[mNodeId + 1];
//        qCubicSegment2D surrSeg(prevNode.fP1, prevNode.fC2,
//                                nextNode.fC0, nextNode.fP1);
//        node.fRelT = surrSeg.tAtPos(node.fP1);
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
                                 node.fRelT);
        if(prevNode.fType == Node::DISSOLVED) {
            const qreal a = prevNode.fRelT;
            const qreal b = node.fRelT;
            const qreal div = 1 + a*b - a;
            if(isZero6Dec(div)) {
                prevNode.fRelT = 0.5;
            } else {
                prevNode.fRelT = a*b/div;
            }
//            Node& prevPrevNode = nodes[mNodeId - 2];
//            qCubicSegment2D seg(prevPrevNode.fP1, prevPrevNode.fC2,
//                                nextNode.fC0, nextNode.fP1);
//            prevNode.fRelT = seg.tAtPos(prevNode.fP1);
        }
        if(nextNode.fType == Node::DISSOLVED) {
            const qreal a = node.fRelT;
            const qreal b = nextNode.fRelT;
            const qreal div = 1 + a*b - a;
            if(isZero6Dec(div)) {
                prevNode.fRelT = 0.5;
            } else {
                 nextNode.fRelT = b/div;
            }
//            Node& nextNextNode = nodes[mNodeId + 2];
//            qCubicSegment2D seg(prevNode.fP1, prevNode.fC2,
//                                nextNextNode.fC0, nextNextNode.fP1);
//            nextNode.fRelT = seg.tAtPos(prevNode.fP1);
        }
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
        newNode.fRelT = mT;
        newNode.fType = Node::DISSOLVED;
        gGetValuesForNodeInsertion(prevNode.fP1, prevNode.fC2,
                                   newNode.fC0, newNode.fP1, newNode.fC2,
                                   nextNode.fC0, nextNode.fP1, mT);
        if(prevNode.fType == Node::DISSOLVED) {
            const qreal mNewNodeT = gMapTFromFragment(prevNode.fRelT, 1, mT);
            prevNode.fRelT = gMapTToFragment(0, mNewNodeT, prevNode.fRelT);
        }
        if(nextNode.fType == Node::DISSOLVED) {
            const qreal mNewNodeT = gMapTFromFragment(0, nextNode.fRelT, mT);
            nextNode.fRelT = gMapTToFragment(mNewNodeT, 1, nextNode.fRelT);
        }
        nodes.insert(mNode2Id, newNode);
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
    }
private:
    int mNodeId;
};
class ActionsApplier {
public:


};

#endif // SMARTPATHCONTAINER_H
