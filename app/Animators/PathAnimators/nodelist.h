#ifndef NODELIST_H
#define NODELIST_H
#include "node.h"
#include "smartPointers/stdselfref.h"
#include "smartPointers/stdpointer.h"
class SkPath;
class NodeList : public StdSelfRef {
public:

    Node& operator[](const int& i) {
        return mNodes[i];
    }

    const Node& operator[](const int& i) const {
        return mNodes[i];
    }

    Node& at(const int& i) {
        return mNodes[i];
    }

    const Node& at(const int& i) const {
        return mNodes[i];
    }

    int count() const {
        return mNodes.count();
    }

    void setNodeType(const int& nodeId, const Node::Type& type) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodeType(nodeId, mNodes[nodeId], type);
    }

    void setNodeType(const int& nodeId, Node& node,
                     const Node::Type& type) {
        node.setType(type);
        updateAfterNodeChanged(nodeId);
    }

    void setNodeNextId(const int& nodeId, const int& nextId) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodeNextId(nodeId, mNodes[nodeId], nextId);
    }

    void setNodeNextId(const int& nodeId, Node& node,
                       const int& nextId) {
        node.setNextNodeId(nextId);
        updateAfterNodeChanged(nodeId);
    }

    void setNodePrevId(const int& nodeId, const int& prevId) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodePrevId(nodeId, mNodes[nodeId], prevId);
    }

    void setNodePrevId(const int& nodeId, Node& node,
                       const int& prevId) {
        node.setPrevNodeId(prevId);
        updateAfterNodeChanged(nodeId);
    }

    void setNodePrevAndNextId(const int& nodeId,
                              const int& prevId, const int& nextId) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodePrevAndNextId(nodeId, mNodes[nodeId], prevId, nextId);
    }

    void setNodePrevAndNextId(const int& nodeId, Node& node,
                              const int& prevId, const int& nextId) {
        node.setPrevNodeId(prevId);
        node.setNextNodeId(nextId);
        updateAfterNodeChanged(nodeId);
    }

    void updateAfterNodeChanged(const int& nodeId) {
        updateNodeTypeAfterNeighbourChanged(nodeId);
        if(mPrev) mPrev->updateNodeTypeAfterNeighbourChanged(nodeId);
        if(mNext) mNext->updateNodeTypeAfterNeighbourChanged(nodeId);
    }

    bool updateNodeTypeAfterNeighbourChanged(const int &nodeId);
    QList<int> updateAllNodesTypeAfterNeighbourChanged();

    int prevNormalId(const int &nodeId) const;
    int nextNormalId(const int &nodeId) const;
    int prevNonDummyId(const int &nodeId) const;
    int nextNonDummyId(const int &nodeId) const;
    SkPath toSkPath() const;
    int firstSegmentNode(const int &nodeId) const;
    int lastSegmentNode(const int &nodeId) const;
    void removeNodeFromList(const int &nodeId);
    Node &insertNodeToList(const int &nodeId, const Node &node);
    int nodesInSameSagment(const int &node1Id, const int &node2Id) const;
    void reverseSegment(const int &nodeId);
    bool segmentClosed(const int &nodeId) const;
    int insertNodeBefore(const int &nextId, const Node &nodeBlueprint);
    int insertNodeAfter(const int &prevId, const Node &nodeBlueprint);
    void promoteDissolvedNodeToNormal(const int &nodeId, Node &node);
    void promoteDissolvedNodeToNormal(const int &nodeId);
    void splitNode(const int &nodeId);
    void splitNodeAndDisconnect(const int &nodeId);
    bool shouldSplitThisNode(const int &nodeId,
                             const Node &thisNode,
                             const Node &neighNode,
                             const NodeList * const thisNodes,
                             const NodeList * const neighNodes) const;
    bool nodesConnected(const int &node1Id, const int &node2Id) const;

    void setPrev(NodeList * const prev) {
        mPrev = prev;
        updateAllNodesTypeAfterNeighbourChanged();
    }

    void setNext(NodeList * const next) {
        mNext = next;
        updateAllNodesTypeAfterNeighbourChanged();
    }

    void moveNodeAfter(const int &moveNodeId, Node &moveNode,
                       const int &afterNodeId, Node &afterNode);
    void moveNodeBefore(const int &moveNodeId, Node &moveNode,
                        const int &beforeNodeId, Node &beforeNode);
private:
    qreal prevT(const int &nodeId) const;
    qreal nextT(const int &nodeId) const;

    stdptr<NodeList> mPrev;
    stdptr<NodeList> mNext;
    QList<Node> mNodes;
};

#endif // NODELIST_H
