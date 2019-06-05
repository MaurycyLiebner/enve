#ifndef NODE_H
#define NODE_H
#include "pointhelpers.h"
#include "exceptions.h"

struct NormalNodeData {
    bool fC0Enabled;
    bool fC2Enabled;
    CtrlsMode fCtrlsMode;
    QPointF fC0;
    QPointF fP1;
    QPointF fC2;
};

struct Node {
    friend class NodeList;
    friend class ListOfNodes;
    enum NodeType : char {
        DISSOLVED, NORMAL, NONE
    };

    Node() { mType = NONE; }

    Node(const NormalNodeData& data) {
        mType = NORMAL;
        setNormalData(data);
    }

    Node(const QPointF& c0, const QPointF& p1, const QPointF& c2) {
        mC0 = c0;
        mP1 = p1;
        mC2 = c2;
        mType = NORMAL;
    }

    Node(const qreal t) {
        mT = t;
        mType = DISSOLVED;
    }

    bool isNormal() const { return mType == NORMAL; }

    bool isDissolved() const { return mType == DISSOLVED; }

    int getNodeId() const {
        return mId;
    }

    void setNormalData(const NormalNodeData& data) {
        mC0 = data.fC0;
        mP1 = data.fP1;
        mC2 = data.fC2;
        setC0Enabled(data.fC0Enabled);
        setC2Enabled(data.fC2Enabled);
        setCtrlsMode(data.fCtrlsMode);
    }

    static Node sInterpolateNormal(const Node &node1, const Node &node2,
                                   const qreal weight2);

    static Node sInterpolateDissolved(const Node &node1, const Node &node2,
                                      const qreal weight2);

    QPointF c0() const {
        if(mC0Enabled) return mC0;
        return mP1;
    }

    QPointF p1() const {
        return mP1;
    }

    QPointF c2() const {
        if(mC2Enabled) return mC2;
        return mP1;
    }

    qreal t() const {
        return mT;
    }

    void setC0(const QPointF& c0) {
        mC0 = c0;
    }

    void setC2(const QPointF& c2) {
        mC2 = c2;
    }

    void setP1(const QPointF& p1) {
        mP1 = p1;
    }

    void setT(const qreal t) {
        mT = t;
    }

    NodeType getType() const { return mType; }
    CtrlsMode getCtrlsMode() const { return mCtrlsMode; }
    bool getC0Enabled() const {
        return mC0Enabled;
    }

    bool getC2Enabled() const {
        return mC2Enabled;
    }

    void applyTransform(const QMatrix &transform) {
        mC0 = transform.map(mC0);
        mP1 = transform.map(mP1);
        mC2 = transform.map(mC2);
    }
protected:
    void setNodeId(const int nodeId) {
        mId = nodeId;
    }

    void setType(const NodeType& type) {
        mType = type;
    }

    void setCtrlsMode(const CtrlsMode& ctrlsMode) {
        mCtrlsMode = ctrlsMode;
        if(ctrlsMode == CtrlsMode::CTRLS_SYMMETRIC) {
            gGetCtrlsSymmetricPos(mC0, mP1, mC2, mC0, mC2);
        } else if(ctrlsMode == CtrlsMode::CTRLS_SMOOTH) {
            gGetCtrlsSmoothPos(mC0, mP1, mC2, mC0, mC2);
        } else return;
        setC0Enabled(true);
        setC2Enabled(true);
    }

    void setC0Enabled(const bool enabled) {
        mC0Enabled = enabled;
    }

    void setC2Enabled(const bool enabled) {
        mC2Enabled = enabled;
    }
private:
    bool mC0Enabled = true;
    bool mC2Enabled = true;
    NodeType mType;
    CtrlsMode mCtrlsMode = CtrlsMode::CTRLS_SYMMETRIC;
    int mId = -1;
    //! @brief T value for segment defined by previous and next normal node
    qreal mT;
    QPointF mC0;
    QPointF mP1;
    QPointF mC2;
};
#include "smartPointers/stdselfref.h"
class ListOfNodes {
public:
    ListOfNodes() {}

    bool isEmpty() const {
        return mList.isEmpty();
    }

    int append(const Node& nodeBlueprint) {
        const int id = mList.count();
        insert(id, nodeBlueprint);
        return id;
    }

    void insert(const int id, const Node& nodeBlueprint) {
        Node * const newNode = insertNewNode(id);
        *newNode = nodeBlueprint;
        newNode->setNodeId(id);
    }

    void clear() {
        mList.clear();
    }

    void swap(ListOfNodes& other) {
        mList.swap(other.mList);
    }

    Node* at(const int id) const {
        return atSPtr(id).get();
    }

    stdsptr<Node> atSPtr(const int id) const {
        if(id < 0 || id >= count()) RuntimeThrow("Index out of range.");
        return mList.at(id);
    }

    Node* first() const {
        return at(0);
    }

    Node* last() const {
        return at(mList.count() - 1);
    }

    int count() const {
        return mList.count();
    }

    void shallowCopyFrom(const ListOfNodes& other) {
        mList = other.mList;
    }

    void deepCopyFrom(const ListOfNodes& other) {
        const int otherCount = other.count();
        const int thisCount = count();
        int i = 0;
        for(; i < otherCount && i < thisCount; i++) {
            *mList.at(i).get() = *other.at(i);
        }
        for(; i < otherCount; i++) {
            insert(i, *other.at(i));
        }
        for(; i < thisCount; i++) {
            removeAt(i);
        }
    }

    void moveNode(const int fromId, const int toId) {
        mList.move(fromId, toId);
        updateNodeIds(qMin(fromId, toId));
    }

    Node * operator[](const int id) const {
        return at(id);
    }

    void removeAt(const int id) {
        mList.removeAt(id);
        updateNodeIds(id);
    }

    void reverse() {
        const auto cpy = mList;
        mList.clear();
        for(const auto& node : cpy) {
            const auto oldC0 = node->c0();
            node->setC0(node->c2());
            node->setC2(oldC0);
            mList.prepend(node);
        }
        updateNodeIds();
    }

    void moveNodesToFrontStartingWith(const int first) {
        for(int i = first; i < mList.count(); i++)
            mList.prepend(mList.takeLast());
        updateNodeIds();
    }

    ListOfNodes detachNodesStartingWith(const int first) {
        ListOfNodes result;
        const int iniCount = mList.count();
        for(int i = first, j = 0; i < iniCount; i++, j++) {
            const auto node = mList.takeLast();
            node->setNodeId(j);
            result.mList.prepend(node);
        }
        return result;
    }

    void appendNodesShallowCopy(const ListOfNodes& src) {
        if(src.isEmpty()) return;
        const int oldCount = count();
        for(const auto& node : src)
            mList.append(node);
        updateNodeIds(oldCount);
    }

    void prependNodesShallowCopy(const ListOfNodes& src) {
        if(src.isEmpty()) return;
        for(int i = src.count() - 1; i >= 0; i--) {
            const auto node = src.atSPtr(i);
            mList.prepend(node);
        }
        updateNodeIds();
    }

    void applyTransform(const QMatrix &transform) {
        for(const auto& node : mList)
            node->applyTransform(transform);
    }

    typedef QList<stdsptr<Node>>::const_iterator const_iterator;
    typedef QList<stdsptr<Node>>::iterator iterator;

    const_iterator begin() const {
        return mList.begin();
    }

    const_iterator end() const {
        return mList.end();
    }
private:
    void updateNodeIds() {
        updateNodeIds(0);
    }

    void updateNodeIds(const int minId) {
        const int maxId = mList.count() - 1;
        updateNodeIds(minId, maxId);
    }

    void updateNodeIds(const int minId, const int maxId) {
        for(int i = minId; i <= maxId; i++)
            updateNodeId(i);
    }

    void updateNodeId(const int id) {
        mList.at(id)->setNodeId(id);
    }

    Node* insertNewNode(const int id) {
        const auto newNode = stdsptr<Node>(new Node);
        mList.insert(id, newNode);
        updateNodeIds(id);
        return newNode.get();
    }

    QList<stdsptr<Node>> mList;
};

#endif // NODE_H
