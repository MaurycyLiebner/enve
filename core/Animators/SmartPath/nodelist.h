#ifndef NODELIST_H
#define NODELIST_H
#include "node.h"
#include "smartPointers/stdselfref.h"
#include "smartPointers/stdpointer.h"
#include "basicreadwrite.h"
class SkPath;
class NodeList {
    friend class SmartPath;
    enum Neighbour { NONE, NEXT, PREV, BOTH = NEXT | PREV };
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

    void setNodeCtrlsMode(const int& nodeId, const CtrlsMode& ctrlsMode) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodeCtrlsMode(nodeId, mNodes[nodeId], ctrlsMode);
    }

    void setNodeCtrlsMode(const int& nodeId, Node& node,
                          const CtrlsMode& ctrlsMode) {
        Q_UNUSED(nodeId);
        node.setCtrlsMode(ctrlsMode);
        //updateAfterNodeChanged(nodeId);
    }

    void setNodeC0Enabled(const int& nodeId, const bool& enabled) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodeC0Enabled(nodeId, mNodes[nodeId], enabled);
    }

    void setNodeC0Enabled(const int& nodeId, Node& node,
                          const bool& enabled) {
        Q_UNUSED(nodeId);
        node.setC0Enabled(enabled);
    }

    void setNodeC2Enabled(const int& nodeId, const bool& enabled) {
        if(nodeId < 0 || nodeId >= mNodes.count()) return;
        setNodeC2Enabled(nodeId, mNodes[nodeId], enabled);
    }

    void setNodeC2Enabled(const int& nodeId, Node& node,
                          const bool& enabled) {
        Q_UNUSED(nodeId);
        node.setC2Enabled(enabled);
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
        if(mNoUpdates) return;
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
    int nodesInSameSagment(const int &node1Id, const int &node2Id) const;
    void reverseSegment(const int &nodeId);
    bool segmentClosed(const int &nodeId) const;

    int insertFirstNode(const Node &nodeBlueprint);
    void promoteDissolvedNodeToNormal(const int &nodeId, Node &node);
    void promoteDissolvedNodeToNormal(const int &nodeId);
    void splitNode(const int &nodeId);
    void splitNodeAndDisconnect(const int &nodeId);
    bool nodesConnected(const int &node1Id, const int &node2Id) const;

    void setPrev(NodeList * const prev) {
        mPrev = prev;
        if(mNoUpdates) return;
        updateAllNodesTypeAfterNeighbourChanged();
    }

    void setNext(NodeList * const next) {
        mNext = next;
        if(mNoUpdates) return;
        updateAllNodesTypeAfterNeighbourChanged();
    }

    void moveNodeAfter(const int &moveNodeId, Node &moveNode,
                       const int &afterNodeId, Node &afterNode);
    void moveNodeBefore(const int &moveNodeId, Node &moveNode,
                        const int &beforeNodeId, Node &beforeNode);

    int insertNodeBefore(const int &nextId, const Node &nodeBlueprint) {
        return insertNodeBefore(nextId, nodeBlueprint, BOTH);
    }

    int insertNodeAfter(const int &prevId, const Node &nodeBlueprint) {
        return insertNodeAfter(prevId, nodeBlueprint, BOTH);
    }

    int appendNode(const Node &nodeBlueprint) {
        return appendNode(nodeBlueprint, BOTH);
    }

    NodeList createCopy() const {
        return NodeList(mNodes, true);
    }

    void read(QIODevice * const src);

    void write(QIODevice * const dst) const;
protected:
    NodeList(const bool& noUpdates = false) : mNoUpdates(noUpdates) {}

    NodeList(const QList<Node>& list,
             const bool& noUpdates = false) :
        mNoUpdates(noUpdates), mNodes(list) {}

    const QList<Node>& getList() const {
        return mNodes;
    }

    void setNodeList(const QList<Node>& list) {
        mNodes = list;
    }

    int insertNodeBefore(const int &nextId, const Node &nodeBlueprint,
                         const Neighbour& neigh);
    int insertNodeAfter(const int &prevId, const Node &nodeBlueprint,
                        const Neighbour& neigh);
    int appendNode(const Node &nodeBlueprint, const Neighbour &neigh);

    static NodeList sInterpolate(const NodeList &list1,
                                 const NodeList &list2,
                                 const qreal &weight2);

    static bool sDifferent(const NodeList& list1, const NodeList& list2) {
        const auto list1v = list1.getList();
        const auto list2v = list2.getList();
        if(list1v.count() != list2v.count()) return false;
        for(int i = 0; i < list1.count(); i++) {
            if(list1v.at(i) != list2v.at(i)) return true;
        }
        return false;
    }
private:
    qreal prevT(const int &nodeId) const;
    qreal nextT(const int &nodeId) const;
    Node &insertNodeToList(const int &nodeId, const Node &node);

    bool mNoUpdates;
    NodeList * mPrev = nullptr;
    NodeList * mNext = nullptr;
    QList<Node> mNodes;
};

#endif // NODELIST_H
