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

    void actionRemoveNode(const int& nodeId);

    int actionAddFirstNode(const QPointF& c0,
                           const QPointF& p1,
                           const QPointF& c2);
    int actionAddFirstNode(const NormalNodeData &data);

    void actionMoveNodeBetween(const int& movedNodeId,
                               const int& prevNodeId,
                               const int& nextNodeId);

    int actionAppendNodeAtEndNode(const int &endNodeId);
    int actionAppendNodeAtEndNode(const NodePointValues &values);
    int actionAppendNodeAtEndNode();

    int actionInsertNodeBetween(const int &prevId,
                                const int& nextId,
                                const QPointF &c0,
                                const QPointF &p1,
                                const QPointF &c2);
    int actionInsertNodeBetween(const int &prevId,
                                const int& nextId,
                                const qreal& t);

    void actionPromoteDissolvedNodeToNormal(const int& nodeId);
    void actionDemoteToDissolved(const int &nodeId);

    void actionDisconnectNodes(const int& node1Id, const int& node2Id);

    void actionConnectNodes(const int& node1Id, const int& node2Id);

    void actionSetDissolvedNodeT(const int& nodeId, const qreal& t) {
        Node * const node = mNodesList.at(nodeId);
        if(!node->isDissolved()) RuntimeThrow("Setting dissolved node value "
                                             "on a node of a different type");
        node->setT(t);
        updateDissolvedNodePosition(nodeId, node);
    }

    void actionSetNormalNodeValues(const int& nodeId,
                                   const QPointF& c0,
                                   const QPointF& p1,
                                   const QPointF& c2) {
        Node * const node = mNodesList.at(nodeId);
        if(!node->isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        node->setC0(c0);
        node->setP1(p1);
        node->setC2(c2);
    }

    void actionSetNormalNodeValues(const int& nodeId,
                                   const NormalNodeData& data) {
        Node * const node = mNodesList.at(nodeId);
        if(!node->isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        node->setNormalData(data);
    }

    void actionSetNormalNodeP1(const int& nodeId, const QPointF& p1) {
        Node * const node = mNodesList.at(nodeId);
        if(!node->isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        node->setP1(p1);
    }

    void actionSetNormalNodeC0(const int& nodeId, const QPointF& c0) {
        Node * const node = mNodesList.at(nodeId);
        if(!node->isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        node->setC0(c0);
    }

    void actionSetNormalNodeC2(const int& nodeId, const QPointF& c2) {
        Node * const node = mNodesList.at(nodeId);
        if(!node->isNormal()) RuntimeThrow("Setting normal node values "
                                          "on a node of a different type");
        node->setC2(c2);
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

    void actionReversePath() {
        mNodesList.reverse();
    }

    void actionAppendMoveAllFrom(SmartPath& other) {
        mNodesList.appendShallowCopyFrom(other.getNodesRef());
        other.clear();
    }

    void actionPrependMoveAllFrom(SmartPath& other) {
        mNodesList.prependShallowCopyFrom(other.getNodesRef());
        other.clear();
    }
    void actionMergeNodes(const int &node1Id, const int &node2Id);

    void reset() {
        mNodesList.clear();
        mSavedList.clear();
        mLastDetached.clear();
    }

    void clear() {
        reset();
    }

    bool isEmpty() const {
        return mNodesList.count() == 0;
    }

    SkPath getPathAt() const;
    void setPath(const SkPath& path);

    const Node * getNodePtr(const int& id) const {
        if(id < 0) return nullptr;
        if(id >= mNodesList.count()) return nullptr;
        return mNodesList[id];
    }

    int prevNodeId(const int &nodeId) const {
        const auto node = mNodesList.prevNode(nodeId);
        if(node) return node->getNodeId();
        return -1;
    }

    int nextNodeId(const int &nodeId) const {
        const auto node = mNodesList.nextNode(nodeId);
        if(node) return node->getNodeId();
        return -1;
    }

    int prevNormalId(const int &nodeId) const {
        const auto node = mNodesList.prevNormal(nodeId);
        if(node) return node->getNodeId();
        return -1;
    }

    int nextNormalId(const int &nodeId) const {
        const auto node = mNodesList.nextNormal(nodeId);
        if(node) return node->getNodeId();
        return -1;
    }

    ValueRange dissolvedTRange(const int& nodeId);

    void updateDissolvedNodePosition(const int& nodeId) {
        mNodesList.updateDissolvedNodePosition(nodeId);
    }

    void updateDissolvedNodePosition(const int& nodeId, Node * const node) {
        mNodesList.updateDissolvedNodePosition(nodeId, node);
    }

    bool isClosed() const {
        return mNodesList.isClosed();
    }

    void save() {
        mSavedList = mNodesList.createDeepCopy();
    }

    void restore() {
        mNodesList = mSavedList.createDeepCopy();
    }

    SmartPath createCopy() const {
        SmartPath copy;
        copy.deepCopy(*this);
        return copy;
    }

    int getNodeCount() const {
        return mNodesList.count();
    }

    void deepCopy(const SmartPath& src) {
        deepCopy(src.getNodesRef());
    }

    void deepCopy(const NodeList& src) {
        mNodesList = src.createDeepCopy();
    }

    void shallowCopy(const SmartPath& src) {
        shallowCopy(src.getNodesRef());
    }

    void shallowCopy(const NodeList& src) {
        mNodesList = src;
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
        target.shallowCopy(list);
    }

    bool hasDetached() const {
        return mLastDetached.count() > 0;
    }

    NodeList getAndClearLastDetached() {
        NodeList detached;
        mLastDetached.swap(detached);
        return detached;
    }

    void applyTransform(const QMatrix &transform) {
        mNodesList.applyTransform(transform);
    }

    bool read(QIODevice * const src) {
        return mNodesList.read(src);
    }

    bool write(QIODevice * const dst) const {
        return mNodesList.write(dst);
    }
protected:
    const NodeList& getNodesRef() const {
        return mNodesList;
    }
private:
    int insertNodeBetween(const int &prevId, const int &nextId,
                          const Node &nodeBlueprint);

    NodeList mNodesList;
    NodeList mSavedList;

    NodeList mLastDetached;
};

#endif // SMARTPATHCONTAINER_H
