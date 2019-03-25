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
    SmartPath(const SmartPath &list);

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
        Node * const node = mNodesList.at(nodeId);
        if(!node->isDissolved()) RuntimeThrow("Setting dissolved node value "
                                             "on a node of a different type");
        node->fT = t;
        updateDissolvedNodePosition(nodeId, node);
    }

    void actionSetNormalNodeValues(const int& nodeId,
                                   const QPointF& c0,
                                   const QPointF& p1,
                                   const QPointF& c2) {
        Node * const node = mNodesList.at(nodeId);
        if(!node->isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        node->fC0 = c0;
        node->fP1 = p1;
        node->fC2 = c2;
    }

    void actionSetNormalNodeP1(const int& nodeId, const QPointF& p1) {
        Node * const node = mNodesList.at(nodeId);
        if(!node->isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        node->fP1 = p1;
    }

    void actionSetNormalNodeC0(const int& nodeId, const QPointF& c0) {
        Node * const node = mNodesList.at(nodeId);
        if(!node->isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        node->fC0 = c0;
    }

    void actionSetNormalNodeC2(const int& nodeId, const QPointF& c2) {
        Node * const node = mNodesList.at(nodeId);
        if(!node->isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        node->fC2 = c2;
    }

    void actionSetNormalNodeCtrlsMode(const int& nodeId, const CtrlsMode& mode) {
        Node * const node = mNodesList.at(nodeId);
        if(!node->isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        mNodesList.setNodeCtrlsMode(node, mode);
    }

    void actionSetNormalNodeC0Enabled(const int& nodeId, const bool& enabled) {
        Node * const node = mNodesList.at(nodeId);
        if(!node->isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        mNodesList.setNodeC0Enabled(node, enabled);
    }

    void actionSetNormalNodeC2Enabled(const int& nodeId, const bool& enabled) {
        Node * const node = mNodesList.at(nodeId);
        if(!node->isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        mNodesList.setNodeC2Enabled(node, enabled);
    }


    SkPath getPathAt() const;

    const Node * getNodePtr(const int& id) const {
        if(id < 0) return nullptr;
        if(id >= mNodesList.count()) return nullptr;
        return mNodesList[id];
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

    void updateDissolvedNodePosition(const int& nodeId, Node * const node) {
        mNodesList.updateDissolvedNodePosition(nodeId, node);
    }

    void save() {
        mSavedList = mNodesList.getList();
    }

    void restore() {
        mNodesList.setNodeList(mSavedList);
    }

    SmartPath createCopy() const {
        return SmartPath(*this);
    }

    int getNodeCount() const {
        return mNodesList.count();
    }

    void assign(const SmartPath& src) {
        mNodesList.setNodeList(src.getNodesRef().getList());
    }

    static bool sDifferent(const SmartPath& path1,
                           const SmartPath& path2) {
        return NodeList::sDifferent(path1.getNodesRef(),
                                    path2.getNodesRef());
    }

    static void sInterpolate(const SmartPath &path1,
                             const SmartPath &path2,
                             const qreal &path2Weight,
                             SmartPath& target) {
        const auto list = NodeList::sInterpolate(
                    path1.getNodesRef(),
                    path2.getNodesRef(),
                    path2Weight);
        target.getNodesPtr()->setNodeList(list.getList());
    }

    bool read(QIODevice * const src) {
        return mNodesList.read(src);
    }

    bool write(QIODevice * const dst) const {
        return mNodesList.write(dst);
    }
protected:
    NodeList *getNodesPtr();

    const NodeList& getNodesRef() const {
        return mNodesList;
    }

    NodeList& getNodesRef() {
        return mNodesList;
    }
private:
    int insertNodeBetween(const int &prevId, const int &nextId,
                          const Node &nodeBlueprint);
    void prepareForNewNeighBetweenThisAnd(SmartPath * const neighbour);

    NodeList mNodesList;
    ListOfNodes mSavedList;
};

#endif // SMARTPATHCONTAINER_H
