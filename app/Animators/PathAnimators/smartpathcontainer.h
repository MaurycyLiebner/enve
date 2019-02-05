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
        NORMALIZE_DISSOLVED, REMOVE_START_OR_END
    };

    NodeAction(const Type& type) : mType(type) {

    }

    const Type& getType() const {
        return mType;
    }

    virtual void apply(QList<Node>& nodes) const = 0;
    virtual int getMaxInfluencedId() const = 0;
private:
    Type mType;
};

class OneNodeAction : public NodeAction {
public:
    OneNodeAction(const int& nodeId, const Type& type) :
        NodeAction(type) {
        mNodeId = nodeId;
    }

    int getMaxInfluencedId() const {
        return mNodeId;
    }
protected:
    int mNodeId;
};

class TwoNodesAction : public NodeAction {
public:
    TwoNodesAction(const int& node1Id,
                   const int& node2Id,
                   const Type& type) :
        NodeAction(type) {
        mNode1Id = node1Id;
        mNode2Id = node2Id;
    }

    int getMaxInfluencedId() const {
        return mNode2Id;
    }
protected:
    int mNode1Id;
    int mNode2Id;
};

//class MergeNodesAction : public NodeAction {
//public:
//    //! @brief FirstNodeId can be negative.
//    MergeNodesAction(const int& firstNodeId, const int& lastNodeId) :
//        NodeAction(MERGE), mFirstNodeId(firstNodeId), mLastNodeId(lastNodeId) {}

//    void apply(QList<Node>& nodes) const {
//        if(mFirstNodeId >= mLastNodeId)
//            RuntimeThrow("Invalid merging range");
//        const int fixedFirstId = mFirstNodeId < 0 ?
//                    nodes.count() + mFirstNodeId :
//                    mFirstNodeId;
//        auto& firstNode = nodes[fixedFirstId];
//        auto& lastNode = nodes[mLastNodeId];
//        firstNode.fC2 = lastNode.fC2;
//        if(lastNode.fType == Node::NORMAL_END) {
//            if(firstNode.fType == Node::NORMAL_START) {
//                firstNode.fType = Node::NORMAL_START_AND_END;
//            } else {
//                firstNode.fType = Node::NORMAL_END;
//            }
//        }
//        for(int i = mFirstNodeId + 1; i <= mLastNodeId; i++) {
//            const int fixedI = i < 0 ? nodes.count() + i : i;
//            nodes.removeAt(fixedI);
//        }
//    }
//private:
//    int mFirstNodeId;
//    int mLastNodeId;
//};

//class SplitNodeAction : public NodeAction {
//public:
//    SplitNodeAction(const int& nodeId,
//                    const int& duplicatesBefore,
//                    const int& duplicatesAfter,
//                    const bool& disconnect) :
//        NodeAction(SPLIT), mNodeId(nodeId),
//        mDuplicatesBefore(duplicatesBefore),
//        mDuplicatesAfter(duplicatesAfter),
//        mDisconnect(disconnect) {}

//    // !!! should also handle NORMAL_END and NORMAL_START nodes
//    void apply(QList<Node>& nodes) const {
//        Node toSplit = nodes.takeAt(mNodeId);

//        const int iMax = mDuplicatesBefore + 1 + mDuplicatesAfter;
//        for(int i = 0; i <= iMax; i++) {
//            Node node;
//            node.fC0 = (i == 0 ? toSplit.fC0 : toSplit.fP1);
//            node.fP1 = toSplit.fP1;
//            node.fC2 = (i == iMax ? toSplit.fC2 : toSplit.fP1);
//            node.fType = Node::NORMAL_MIDDLE;
//            nodes.insert(mNodeId, node);
//        }

//        Node& split1 = nodes[mDuplicatesBefore + mNodeId];
//        Node& split2 = nodes[mDuplicatesBefore + mNodeId + 1];
//        if(mDisconnect) {
//            if(toSplit.fType == Node::DISSOLVED)
//                RuntimeThrow("SplitNodeAction: Unsupported Node Type DISSOLVED");
//            if(split1.fType == Node::NORMAL_START) {
//                split1.fType = Node::NORMAL_START_AND_END;
//            } else {
//                split1.fType = Node::NORMAL_END;
//            }

//            if(split2.fType == Node::NORMAL_END) {
//                split2.fType = Node::NORMAL_START_AND_END;
//            } else {
//                split2.fType = Node::NORMAL_START;
//            }
//        }
//    }
//private:
//    int mNodeId;
//    int mDuplicatesBefore;
//    int mDuplicatesAfter;
//    bool mDisconnect;
//};

//! @brief Assumes node1Id is before node2Id
bool areNodesAdjecent(const int& node1Id, const int& node2Id,
                      const QList<Node>& nodes) {
    if(node1Id + 1 == node2Id) return true;
    return node1Id == nodes.count() - 1 && node2Id == 0;;
}

int fixNegativeId(const int& id, const QList<Node>& nodes) {
    if(id >= 0) return id;
    return id + nodes.count();
}

class DisconnectNodesAction : public TwoNodesAction {
public:
    //! @brief Node1Id can be negative (ex. -1 will reference last node)
    DisconnectNodesAction(const int& node1Id, const int& node2Id) :
        TwoNodesAction(node1Id, node2Id, DISCONNECT) {}

    void apply(QList<Node>& nodes) const {
        if(!areNodesAdjecent(mNode1Id, mNode2Id, nodes))
            RuntimeThrow("Cannot disconnect non-adjacent nodes");
        Node& node1 = nodes[fixNegativeId(mNode1Id, nodes)];
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
};

class ConnectNodesAction : public TwoNodesAction {
public:
    //! @brief Node1Id can be negative (ex. -1 will reference last node)
    ConnectNodesAction(const int& node1Id, const int& node2Id) :
        TwoNodesAction(node1Id, node2Id, CONNECT) {}

    void apply(QList<Node>& nodes) const {
        if(!areNodesAdjecent(mNode1Id, mNode2Id, nodes))
            RuntimeThrow("Cannot connect non-adjacent nodes");
        Node& node1 = nodes[fixNegativeId(mNode1Id, nodes)];
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
};

class DissolveNode : public OneNodeAction {
public:
    DissolveNode(const int& nodeId, const qreal& t) :
        OneNodeAction(nodeId, DISSOLVE), mT(t) {}

    void apply(QList<Node>& nodes) const {
        Node& node = nodes[mNodeId];

        node.fType = Node::DISSOLVED;
        node.fRelT = mT;
    }
private:
    qreal mT;
};

//class RemoveStartOrEndNode : public OneNodeAction {
//    RemoveStartOrEndNode(const int& nodeId) :
//        OneNodeAction(nodeId, REMOVE_START_OR_END) {}

//    void apply(QList<Node>& nodes) const {
//        const Node& node = nodes[mNodeId];
//        if(node.fType != Node::NORMAL_START &&
//           node.fType != Node::NORMAL_END &&
//           node.fType != Node::NORMAL_START_AND_END)
//            RuntimeThrow("RemoveStartOrEndeNode: Unsupported Node Type");
//        if(node.fType == Node::NORMAL_START) {
//            Node& nextNode = nodes[mNodeId + 1];
//            if(nextNode.fType == Node::NORMAL_END) {
//                nextNode.fType = Node::NORMAL_START_AND_END;
//            } else {
//                nextNode.fType = Node::NORMAL_START;
//            }
//        } else if(node.fType == Node::NORMAL_END) {
//            Node& prevNode = nodes[mNodeId - 1];
//            if(prevNode.fType == Node::NORMAL_START) {
//                prevNode.fType = Node::NORMAL_START_AND_END;
//            } else {
//                prevNode.fType = Node::NORMAL_END;
//            }
//        }
//        nodes.removeAt(mNodeId);
//    }
//};

class RemoveDissolvedNode : public OneNodeAction {
public:
    RemoveDissolvedNode(const int& nodeId) :
        OneNodeAction(nodeId, REMOVE_DISSOLVED) {}

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
        }
        nodes.removeAt(mNodeId);
    }
};

class InsertDissolvedNode : public OneNodeAction {
public:
    InsertDissolvedNode(const int& nodeId, const qreal& t) :
        OneNodeAction(nodeId, ADD_DISSOLVED), mT(t) {}

    void apply(QList<Node>& nodes) const {
        nodes.insert(mNodeId, Node());
        Node& newNode = nodes[mNodeId];
        Node &prevNode = nodes[mNodeId - 1];
        Node &nextNode = nodes[mNodeId + 1];
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
    }
private:
    qreal mT;
};

class NormalizeDissolvedNode : public OneNodeAction {
public:
    NormalizeDissolvedNode(const int& nodeId) :
        OneNodeAction(nodeId, NORMALIZE_DISSOLVED) {}

    void apply(QList<Node>& nodes) const {
        Node& node = nodes[mNodeId];
        if(node.fType != Node::DISSOLVED)
             RuntimeThrow("NormalizeDissolvedNode: Unsupported Node Type");
        node.fType = Node::NORMAL_MIDDLE;
    }
};

SkPath nodesToSkPath(const QList<Node>& nodes) {
    SkPath result;
    QPointF lastC2;
    bool first = true;
    Node firstMoveNode;
    for(const Node& node : nodes) {
        const bool move = first || node.fType == Node::NORMAL_START;
        if(first) first = false;
        if(move) {
            firstMoveNode = node;
            result.moveTo(qPointToSk(node.fP1));
            lastC2 = node.fC2;
            continue;
        }
        result.cubicTo(qPointToSk(lastC2),
                       qPointToSk(node.fC0),
                       qPointToSk(node.fP1));
        if(node.fType == Node::NORMAL_END) {
            result.cubicTo(qPointToSk(node.fC2),
                           qPointToSk(firstMoveNode.fC0),
                           qPointToSk(firstMoveNode.fP1));
        } else {
            lastC2 = node.fC2;
        }
    }
    return result;
}

bool actionsCompare(const NodeAction * const node1,
                    const NodeAction * const node2) {
    return node1->getMaxInfluencedId() > node2->getMaxInfluencedId();
}

void sortNodeActions(QList<NodeAction*>& actions) {
    std::sort(actions.begin(), actions.end(), actionsCompare);
}

qCubicSegment2D segmentFromNodes(const Node& prevNode,
                                 const Node& nextNode) {
    return qCubicSegment2D(prevNode.fP1, prevNode.fC2,
                           nextNode.fC0, nextNode.fP1);
}

class ActionsApplier {
public:

    void prevNodeInserted(const int& nodeId) {
        InsertDissolvedNode(nodeId, 0.5).apply(mNodesBefore);
    }

    void prevNodeRemoved(const int& nodeId) {
        auto& node = mNodesBefore[nodeId];
        if(node.fType == Node::DISSOLVED) {
            auto nodeAction = getActionForNode(nodeId);
            if(nodeAction->getType() == NodeAction::REMOVE_DISSOLVED) {
                nodeAction->apply(mNodesBefore);
            } else if(nodeAction->getType() == NodeAction::NORMALIZE_DISSOLVED) {
                return;
            }
            insertAction(new RemoveDissolvedNode(nodeId));
        }
        node.fType = Node::DISSOLVED;
        node.fRelT = 0.5;
        Node& prevNode = mNodesBefore[nodeId - 1];
        Node& nextNode = mNodesBefore[nodeId + 1];
        auto prevNextSeg = segmentFromNodes(prevNode, nextNode);
        auto divided = prevNextSeg.dividedAtT(node.fRelT);
        const auto& first = divided.first;
        const auto& second = divided.second;
        prevNode.fC2 = first.c1();
        node.fC0 = first.c2();
        node.fP1 = first.p1();
        node.fC2 = second.c1();
        nextNode.fC0 = second.c2();
        insertAction(new RemoveDissolvedNode(nodeId));
    }

    void atNodeRemoved(const int& nodeId) {
        insertAction(new DissolveNode(nodeId, 0.5));
    }

    void atNodeInserted(const int& nodeId, const qreal& t) {

        insertAction(new NormalizeDissolvedNode(nodeId));
    }

private:
//    void applyNodeAction(const NodeAction * const action) {
//        action->apply(mNodesBefore);
//    }

    void insertAction(NodeAction * const action) {
        mAfterUpToDate = false;
    }

    NodeAction * getActionForNode(const int& nodeId) {
        return nullptr;
    }

    void updateNodesAfter() {
        mNodesAfter = mNodesBefore;
        for(const auto& action : mActions) {
            action->apply(mNodesAfter);
        }
        mAfterUpToDate = true;
    }
    bool mAfterUpToDate = true;

    QList<Node> mNodesBefore;
    QList<Node> mNodesAfter;

    QList<NodeAction*> mActions;
};

#endif // SMARTPATHCONTAINER_H
