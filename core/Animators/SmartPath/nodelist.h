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
    Node* operator[](const int i) const {
        return mNodes[i];
    }

    Node* at(const int i) const {
        return mNodes[i];
    }

    int count() const {
        return mNodes.count();
    }

    void setNodeType(const int nodeId, const Node::NodeType& type) const {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodeType(mNodes[nodeId], type);
    }

    void setNodeType(Node * const node, const Node::NodeType& type) const {
        node->setType(type);
    }

    void setNodeCtrlsMode(const int nodeId, const CtrlsMode& ctrlsMode) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodeCtrlsMode(mNodes[nodeId], ctrlsMode);
    }

    void setNodeCtrlsMode(Node * const node, const CtrlsMode& ctrlsMode) {
        node->setCtrlsMode(ctrlsMode);
    }

    void setNodeC0Enabled(const int nodeId, const bool enabled) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodeC0Enabled(mNodes[nodeId], enabled);
    }

    void setNodeC0Enabled(Node * const node, const bool enabled) {
        node->setC0Enabled(enabled);
    }

    void setNodeC2Enabled(const int nodeId, const bool enabled) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodeC2Enabled(mNodes[nodeId], enabled);
    }

    void setNodeC2Enabled(Node * const node, const bool enabled) {
        node->setC2Enabled(enabled);
    }

    Node * prevNode(const Node * const node) const {
        return prevNode(node->getNodeId());
    }

    Node * nextNode(const Node * const node) const {
        return nextNode(node->getNodeId());
    }

    Node * prevNode(const int nodeId) const {
        if(mNodes.count() <= 1) return nullptr;
        if(nodeId > 0) return mNodes[nodeId - 1];
        if(mClosed) return mNodes.last();
        return nullptr;
    }

    Node * nextNode(const int nodeId) const {
        if(mNodes.count() <= 1) return nullptr;
        if(nodeId < mNodes.count() - 1) return mNodes[nodeId + 1];
        if(mClosed) return mNodes.first();
        return nullptr;
    }

    Node * prevNormal(const Node * const node) const {
        return prevNormal(node->getNodeId());
    }

    Node * nextNormal(const Node * const node) const {
        return nextNormal(node->getNodeId());
    }

    Node * prevNormal(const int nodeId) const;
    Node * nextNormal(const int nodeId) const;

    SkPath toSkPath() const;
    void setPath(const SkPath &path);
    void removeNodeFromList(const int nodeId);
    void reverse();
    bool isClosed() const;
    void setClosed(const bool closed) {
        mClosed = closed;
    }

    void reset() {
        mClosed = false;
        mNodes.clear();
    }

    void clear() {
        reset();
    }

    int insertFirstNode(const Node &nodeBlueprint);

    void promoteDissolvedNodeToNormal(const int nodeId);
    void promoteDissolvedNodeToNormal(const int nodeId, Node * const node);

    void removeNode(const int nodeId, const bool approx);
    void removeNode(const int nodeId, Node * const node,
                    const bool approx);

    void demoteNormalNodeToDissolved(const int nodeId, const bool approx);
    void demoteNormalNodeToDissolved(const int nodeId, Node * const node,
                                     const bool approx);

    void splitNode(const int nodeId);
    void splitNodeAndDisconnect(const int nodeId);
    void mergeNodes(const int node1Id, const int node2Id);
    bool nodesConnected(const int node1Id, const int node2Id) const;

    void moveNode(const int fromId, const int toId);
    void updateDissolvedNodePosition(const int nodeId);
    void updateDissolvedNodePosition(const int nodeId, Node * const node);

    NodeList createDeepCopy() const {
        NodeList copy;
        copy.deepCopyNodeList(mNodes);
        copy.setClosed(mClosed);
        return copy;
    }

    void applyTransform(const QMatrix &transform) {
        mNodes.applyTransform(transform);
    }

    bool read(QIODevice * const src);
    bool write(QIODevice * const dst) const;
protected:
    void appendShallowCopyFrom(const NodeList& other) {
        mNodes.appendNodesShallowCopy(other.getList());
    }

    void prependShallowCopyFrom(const NodeList& other) {
        mNodes.prependNodesShallowCopy(other.getList());
    }

    void moveNodesToFrontStartingWith(const int first) {
        mNodes.moveNodesToFrontStartingWith(first);
    }

    NodeList detachNodesStartingWith(const int first) {
        const auto detachedList = mNodes.detachNodesStartingWith(first);
        NodeList detached;
        detached.shallowCopyNodeList(detachedList);
        return detached;
    }

    void swap(NodeList& other) {
        mNodes.swap(other.getList());
        const bool wasClosed = mClosed;
        mClosed = other.isClosed();
        other.setClosed(wasClosed);
    }

    ListOfNodes& getList() {
        return mNodes;
    }

    const ListOfNodes& getList() const {
        return mNodes;
    }

    void deepCopyNodeList(const ListOfNodes& list) {
        mNodes.deepCopyFrom(list);
    }

    void shallowCopyNodeList(const ListOfNodes& list) {
        mNodes.shallowCopyFrom(list);
    }

    int insertNodeBefore(const int nextId, const Node &nodeBlueprint);
    int insertNodeAfter(const int prevId, const Node &nodeBlueprint);
    int appendNode(const Node &nodeBlueprint);
    Node * appendAndGetNode(const Node &nodeBlueprint) {
        return mNodes[appendNode(nodeBlueprint)];
    }

    static NodeList sInterpolate(const NodeList &list1,
                                 const NodeList &list2,
                                 const qreal weight2);
private:
    qreal prevT(const int nodeId) const;
    qreal nextT(const int nodeId) const;
    Node *insertNodeToList(const int nodeId, const Node &node);
    void approximateBeforeDemoteOrRemoval(const qreal nodeT,
                                          Node * const node,
                                          Node * const prevNormalV,
                                          Node * const nextNormalV);

    ListOfNodes mNodes;
    bool mClosed = false;
};

#endif // NODELIST_H
