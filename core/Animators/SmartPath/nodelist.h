#ifndef NODELIST_H
#define NODELIST_H
#include "node.h"
#include "smartPointers/stdselfref.h"
#include "smartPointers/stdpointer.h"
#include "basicreadwrite.h"
class SkPath;
class NodeList {
    friend class SmartPath;
protected:
    NodeList() {}
public:
    Node* operator[](const int& i) const {
        return mNodes[i];
    }

    Node* at(const int& i) const {
        return mNodes[i];
    }

    int count() const {
        return mNodes.count();
    }

    void setNodeType(const int& nodeId, const Node::Type& type) const {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodeType(mNodes[nodeId], type);
    }

    void setNodeType(Node * const node, const Node::Type& type) const {
        node->setType(type);
    }

    void setNodeCtrlsMode(const int& nodeId, const CtrlsMode& ctrlsMode) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodeCtrlsMode(mNodes[nodeId], ctrlsMode);
    }

    void setNodeCtrlsMode(Node * const node, const CtrlsMode& ctrlsMode) {
        node->setCtrlsMode(ctrlsMode);
    }

    void setNodeC0Enabled(const int& nodeId, const bool& enabled) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodeC0Enabled(mNodes[nodeId], enabled);
    }

    void setNodeC0Enabled(Node * const node, const bool& enabled) {
        node->setC0Enabled(enabled);
    }

    void setNodeC2Enabled(const int& nodeId, const bool& enabled) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodeC2Enabled(mNodes[nodeId], enabled);
    }

    void setNodeC2Enabled(Node * const node, const bool& enabled) {
        node->setC2Enabled(enabled);
    }

    void setNodeNextId(const int& nodeId, const int& nextId) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodeNextId(mNodes[nodeId], nextId);
    }

    void setNodeNextId(Node * const node, const int& nextId) {
        if(nextId < 0) node->setNextNode(nullptr);
        else node->setNextNode(mNodes[nextId]);
    }

    void setNodePrevId(const int& nodeId, const int& prevId) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodePrevId(mNodes[nodeId], prevId);
    }

    void setNodePrevId(Node * const node, const int& prevId) {
        if(prevId < 0) node->setPrevNode(nullptr);
        else node->setPrevNode(mNodes[prevId]);
    }

    void setNodePrevAndNextId(const int& nodeId,
                              const int& prevId, const int& nextId) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodePrevAndNextId(mNodes[nodeId], prevId, nextId);
    }

    void setNodePrevAndNextId(Node * const node,
                              const int& prevId,
                              const int& nextId) {
        setNodePrevId(node, prevId);
        setNodeNextId(node, nextId);
    }

    void moveNodesToFrontStartingWith(const int& first) {
        mNodes.moveNodesToFrontStartingWith(first);
        updateNodeIds();
    }

    ListOfNodes detachNodesStartingWith(const int& first) {
        const auto detached = mNodes.detachNodesStartingWith(first);
        updateNodeIds();
        return detached;
    }

    int prevNormalId(const int &nodeId) const;
    int nextNormalId(const int &nodeId) const;

    SkPath toSkPath() const;
    void removeNodeFromList(const int &nodeId);
    void reverseSegment();
    bool isClosed() const;

    int insertFirstNode(const Node &nodeBlueprint);
    void promoteDissolvedNodeToNormal(const int &nodeId, Node * const node);
    void promoteDissolvedNodeToNormal(const int &nodeId);
    void splitNode(const int &nodeId);
    void splitNodeAndDisconnect(const int &nodeId);
    bool nodesConnected(const int &node1Id, const int &node2Id) const;

    void moveNodeAfter(const int &moveNodeId, Node * const moveNode,
                       const int &afterNodeId, Node * const afterNode);
    void moveNodeBefore(const int &moveNodeId, Node * const moveNode,
                        const int &beforeNodeId, Node * const beforeNode);

    void updateDissolvedNodePosition(const int& nodeId);
    void updateDissolvedNodePosition(const int &nodeId, Node * const node);

    NodeList createCopy() const {
        NodeList copy;
        copy.deepCopyNodeList(mNodes);
        return copy;
    }

    bool read(QIODevice * const src);
    bool write(QIODevice * const dst) const;
protected:
    const ListOfNodes& getList() const {
        return mNodes;
    }

    void deepCopyNodeList(const ListOfNodes& list) {
        mNodes.deepCopyFrom(list);
    }

    void shallowCopyNodeList(const ListOfNodes& list) {
        mNodes.shallowCopyFrom(list);
    }

    int insertNodeBefore(const int &nextId, const Node &nodeBlueprint);
    int insertNodeAfter(const int &prevId, const Node &nodeBlueprint);
    int appendNode(const Node &nodeBlueprint);

    static bool sDifferent(const NodeList& list1, const NodeList& list2) {
        const auto& list1v = list1.getList();
        const auto& list2v = list2.getList();
        if(list1v.count() != list2v.count()) return false;
        for(int i = 0; i < list1.count(); i++) {
            if(list1v.at(i) != list2v.at(i)) return true;
        }
        return false;
    }

    static NodeList sInterpolate(const NodeList &list1,
                                 const NodeList &list2,
                                 const qreal &weight2);
private:
    qreal prevT(const int &nodeId) const;
    qreal nextT(const int &nodeId) const;
    Node *insertNodeToList(const int &nodeId, const Node &node);
    void updateNodeIds();

    ListOfNodes mNodes;
};

#endif // NODELIST_H
