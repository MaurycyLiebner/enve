#ifndef SMARTPATHCONTAINER_H
#define SMARTPATHCONTAINER_H
#include "simplemath.h"
#include "skia/skiaincludes.h"
#include "exceptions.h"
#include "pointhelpers.h"
#include "nodelist.h"

#include "framerange.h"
#include "smartPointers/sharedpointerdefs.h"

class PathBase {
public:
    PathBase(const NodeList::Type &type);
    void actionRemoveNormalNode(const int& nodeId);

    int actionAddFirstNode(const QPointF& c0,
                           const QPointF& p1,
                           const QPointF& c2);

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

    void removeNodeWithIdAndTellPrevToDoSame(const int& nodeId);

    void removeNodeWithIdAndTellNextToDoSame(const int& nodeId);

    void setPrev(PathBase * const prev);
    void setNext(PathBase * const next);

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

    void updateDissolvedNodePosition(const int& nodeId) {
        mNodesList.updateDissolvedNodePosition(nodeId);
    }

    void updateDissolvedNodePosition(const int& nodeId, Node& node) {
        mNodesList.updateDissolvedNodePosition(nodeId, node);
    }

    void save() {
        mSavedList = mNodesList.getList();
    }

    void restore() {
        mNodesList.setNodeList(mSavedList);
    }

    PathBase createCopy() const {
        return PathBase(mNodesList.getList(), mType);
    }

    int getNodeCount() const {
        return mNodesList.getList().count();
    }

    void assign(const PathBase& src) {
        mNodesList.setNodeList(src.getNodesRef().getList());
    }

    static bool sDifferent(const PathBase& path1, const PathBase& path2) {
        return NodeList::sDifferent(path1.getNodesRef(),
                                    path2.getNodesRef());
    }

    static NodeList::Type sReadType(QIODevice * const src) {
        NodeList::Type type;
        src->read(rcChar(&type), sizeof(NodeList::Type));
        return type;
    }

    bool read(QIODevice * const src) {
        return mNodesList.read(src);
    }

    bool write(QIODevice * const dst) const {
        dst->write(rcConstChar(&mType), sizeof(NodeList::Type));
        return mNodesList.write(dst);
    }
protected:
    PathBase(const QList<Node> &list,
             const NodeList::Type& type);

    void updateAllNodesTypeAfterNeighbourChanged() {
        if(mType == NodeList::NORMAL) return;
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
        if(!mPrev || mType == NodeList::NORMAL) return mNodesList;
        return getNodesListFor(mPrev);
    }

    NodeList getNodesListForNext() const {
        if(!mNext || mType == NodeList::NORMAL) return mNodesList;
        return getNodesListFor(mNext);
    }
    NodeList interpolateNodesListWithNext(const qreal& nextWeight) const;
    NodeList interpolateNodesListWithPrev(const qreal& prevWeight) const;
private:
    NodeList getNodesListFor(PathBase * const neighbour) const;
    SkPath getPathFor(PathBase * const neighbour) const;
    int insertNodeBetween(const int &prevId, const int &nextId,
                          const Node &nodeBlueprint);

    PathBase * mPrev = nullptr;
    PathBase * mNext = nullptr;

    const NodeList::Type mType;
    NodeList mNodesList;
    QList<Node> mSavedList;
};

class SmartPath : public PathBase {
public:
    SmartPath() : PathBase(NodeList::SMART) {}
protected:
    SmartPath(const QList<Node> &list) :
        PathBase(list, NodeList::SMART) {}
};

class NormalPath : public PathBase {
public:
    NormalPath() : PathBase(NodeList::NORMAL) {}
protected:
    NormalPath(const QList<Node> &list) :
        PathBase(list, NodeList::NORMAL) {}
};

#endif // SMARTPATHCONTAINER_H
