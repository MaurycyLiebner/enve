#ifndef SMARTPATHCONTAINER_H
#define SMARTPATHCONTAINER_H
#include "simplemath.h"
#include "skia/skiaincludes.h"
#include "exceptions.h"

struct Node {
    enum Type {
        NORMAL_MIDDLE, NORMAL_START, NORMAL_END, NORMAL_START_AND_END, DISSOLVED,
    };

    SkScalar fC0;
    SkScalar fP1;
    SkScalar fC2;
    SkScalar fT;
    Type fType;
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

class MergeNodeAction : public NodeAction {
public:
    MergeNodeAction(const int& node1Id, const int& node2Id) :
        NodeAction(MERGE), mNode1Id(node1Id), mNode2Id(node2Id) {}

    void apply(QList<Node>& nodes) const {
        nodes.removeAt(mNode1Id);
    }
private:
    int mNode1Id;
    int mNode2Id;
};

class DisconnectNodeAction : public NodeAction {
public:
    DisconnectNodeAction(const int& node1Id, const int& node2Id) :
        NodeAction(DISCONNECT), mNode1Id(node1Id), mNode2Id(node2Id) {}

    void apply(QList<Node>& nodes) const {
        nodes[mNode1Id].fType = Node::NORMAL_END;
        nodes[mNode2Id].fType = Node::NORMAL_START;
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
        Node toSplit = nodes.at(mNodeId);
        Node split1 = toSplit;
        Node split2 = toSplit;
        if(mDisconnect) {
            if(toSplit.fType == Node::DISSOLVED)
                RuntimeThrow("SplitNodeAction: Unsupported Node Type DISSOLVED");
            split1.fType = Node::NORMAL_END;
            split2.fType = Node::NORMAL_START;
        }

        nodes.replace(mNodeId, split1);
        nodes.insert(mNodeId + 1, split2);
        for(int i = 0; i < mDuplicatesAfter; i++) {
            nodes.insert(mNodeId + 2, toSplit);
        }
        for(int i = 0; i < mDuplicatesBefore; i++) {
            nodes.insert(mNodeId, toSplit);
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
    DissolveNode(const int& nodeId, const SkScalar& t) :
        NodeAction(DISSOLVE), mNodeId(nodeId), mT(t) {}

    void apply(QList<Node>& nodes) const {
        Node& node = nodes[mNodeId];
        node.fType = Node::DISSOLVED;
        node.fT = mT;
    }
private:
    int mNodeId;
    SkScalar mT;
};

class RemoveDissolvedNode : public NodeAction {
public:
    RemoveDissolvedNode(const int& nodeId) :
        NodeAction(REMOVE_DISSOLVED), mNodeId(nodeId) {}

    void apply(QList<Node>& nodes) const {
        const Node& node = nodes[mNodeId];
        if(node.fType != Node::DISSOLVED)
            RuntimeThrow("RemoveDissolvedNode: Unsupported Node Type");
        nodes.removeAt(mNodeId);
    }
private:
    int mNodeId;
};

class AddDissolvedNode : public NodeAction {
public:
    AddDissolvedNode(const int& node1Id, const int& node2Id, const SkScalar& t) :
        NodeAction(ADD_DISSOLVED), mNode1Id(node1Id), mNode2Id(node2Id), mT(t) {}

    void apply(QList<Node>& nodes) const {
        Node prevNormalNode = nodes.at(mNode1Id);
        Node nextNormalNode = nodes.at(mNode2Id);
    }
private:
    int mNode1Id;
    int mNode2Id;
    SkScalar mT;
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
        // !!! fix t values for surrounding dissolved nodes
    }
private:
    int mNodeId;
};
class ActionsApplier {
public:


};

#endif // SMARTPATHCONTAINER_H
