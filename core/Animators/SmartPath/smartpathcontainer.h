#ifndef SMARTPATHCONTAINER_H
#define SMARTPATHCONTAINER_H
#include "simplemath.h"
#include "skia/skiaincludes.h"
#include "exceptions.h"
#include "pointhelpers.h"
#include "nodelist.h"
#include "nodepointvalues.h"
#include "framerange.h"
#include "smartPointers/sharedpointerdefs.h"

class SmartPath {
public:
    SmartPath();
    void actionRemoveNormalNode(const int& nodeId);

    int actionAddFirstNode(const QPointF& c0,
                           const QPointF& p1,
                           const QPointF& c2);

    void actionMoveNodeBetween(const int& movedNodeId,
                               const int& prevNodeId,
                               const int& nextNodeId);

    int actionAppendNodeAtEndNode(const int& endNodeId,
                                  const NodePointValues &values);

    int actionInsertNodeBetween(const int &prevId,
                                const int& nextId,
                                const QPointF &c0,
                                const QPointF &p1,
                                const QPointF &c2);
    int actionInsertNodeBetween(const int &prevId,
                                const int& nextId,
                                const qreal& t);

    void actionPromoteDissolvedNodeToNormal(const int& nodeId);

    void actionDisconnectNodes(const int& node1Id, const int& node2Id);

    void actionConnectNodes(const int& node1Id, const int& node2Id);

    void actionSetDissolvedNodeT(const int& nodeId, const qreal& t) {
        Node& node = mNodesList.at(nodeId);
        if(!node.isDissolved()) RuntimeThrow("Setting dissolved node value "
                                             "on a node of a different type");
        node.fT = t;
        updateDissolvedNodePosition(nodeId, node);
    }

    void actionSetNormalNodeValues(const int& nodeId,
                                   const QPointF& c0,
                                   const QPointF& p1,
                                   const QPointF& c2) {
        Node& node = mNodesList.at(nodeId);
        if(!node.isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        node.fC0 = c0;
        node.fP1 = p1;
        node.fC2 = c2;
    }

    void actionSetNormalNodeP1(const int& nodeId, const QPointF& p1) {
        Node& node = mNodesList.at(nodeId);
        if(!node.isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        node.fP1 = p1;
    }

    void actionSetNormalNodeC0(const int& nodeId, const QPointF& c0) {
        Node& node = mNodesList.at(nodeId);
        if(!node.isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        node.fC0 = c0;
    }

    void actionSetNormalNodeC2(const int& nodeId, const QPointF& c2) {
        Node& node = mNodesList.at(nodeId);
        if(!node.isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        node.fC2 = c2;
    }

    void actionSetNormalNodeCtrlsMode(const int& nodeId, const CtrlsMode& mode) {
        Node& node = mNodesList.at(nodeId);
        if(!node.isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        mNodesList.setNodeCtrlsMode(nodeId, node, mode);
    }

    void actionSetNormalNodeC0Enabled(const int& nodeId, const bool& enabled) {
        Node& node = mNodesList.at(nodeId);
        if(!node.isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        mNodesList.setNodeC0Enabled(nodeId, node, enabled);
    }

    void actionSetNormalNodeC2Enabled(const int& nodeId, const bool& enabled) {
        Node& node = mNodesList.at(nodeId);
        if(!node.isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        mNodesList.setNodeC2Enabled(nodeId, node, enabled);
    }

    void setPrev(SmartPath * const prev);
    void setNext(SmartPath * const next);

    SkPath getPathAt() const;
    SkPath getPathForPrev() const;
    SkPath getPathForNext() const;

    SkPath interpolateWithNext(const qreal& nextWeight) const;
    SkPath interpolateWithPrev(const qreal& prevWeight) const;

    const Node * getNodePtr(const int& id) const {
        if(id < 0) return nullptr;
        if(id >= mNodesList.count()) return nullptr;
        return &mNodesList[id];
    }

    int prevNormalId(const int &nodeId) const {
        return mNodesList.prevNormalId(nodeId);
    }

    int nextNormalId(const int &nodeId) const {
        return mNodesList.nextNormalId(nodeId);
    }

    ValueRange dissolvedTRange(const int& nodeId);

    void updateDissolvedNodePosition(const int& nodeId) {
        mNodesList.updateDissolvedNodePosition(nodeId);
    }

    void updateDissolvedNodePosition(const int& nodeId, Node& node) {
        mNodesList.updateDissolvedNodePosition(nodeId, node);
    }

    void updateDummyNodePosition(const int& nodeId) {
        mNodesList.updateDummyNodePosition(nodeId);
    }

    void updateDummyNodePosition(const int& nodeId, Node& node) {
        mNodesList.updateDummyNodePosition(nodeId, node);
    }

    void save() {
        mSavedList.clear();
        const int iCount = mNodesList.count();
        for(int i = 0; i < iCount; i++) {
            mSavedList.append(mNodesList.at(i));
        }
    }

    void restore() {
        mNodesList.setNodeList(mSavedList);
    }

    SmartPath createCopy() const {
        return SmartPath(mNodesList.getList());
    }

    int getNodeCount() const {
        return mNodesList.count();
    }

    void assign(const SmartPath& src) {
        mNodesList.setNodeList(src.getNodesRef().getList());
    }

    static bool sDifferent(const SmartPath& path1, const SmartPath& path2) {
        return NodeList::sDifferent(path1.getNodesRef(),
                                    path2.getNodesRef());
    }
    bool read(QIODevice * const src) {
        return mNodesList.read(src);
    }

    bool write(QIODevice * const dst) const {
        return mNodesList.write(dst);
    }
protected:
    SmartPath(const QList<Node> &list);

    void removeNodeWithIdAndTellPrevToDoSame(const int& nodeId);

    void removeNodeWithIdAndTellNextToDoSame(const int& nodeId);

    void updateAllNodesTypeAfterNeighbourChanged() {
        mNodesList.updateAllNodesTypeAfterNeighbourChanged();
    }

    NodeList *getNodesPtr();

    const NodeList& getNodesRef() const {
        return mNodesList;
    }

    NodeList& getNodesRef() {
        return mNodesList;
    }

    NodeList getNodesListForPrev() const {
        if(!mPrev) return mNodesList;
        return getNodesListFor(mPrev);
    }

    NodeList getNodesListForNext() const {
        if(!mNext) return mNodesList;
        return getNodesListFor(mNext);
    }
    NodeList interpolateNodesListWithNext(const qreal& nextWeight) const;
    NodeList interpolateNodesListWithPrev(const qreal& prevWeight) const;
private:
    NodeList getNodesListFor(SmartPath * const neighbour) const;
    SkPath getPathFor(SmartPath * const neighbour) const;
    int insertNodeBetween(const int &prevId, const int &nextId,
                          const Node &nodeBlueprint);

    SmartPath * mPrev = nullptr;
    SmartPath * mNext = nullptr;

    NodeList mNodesList;
    QList<Node> mSavedList;
};

#endif // SMARTPATHCONTAINER_H
