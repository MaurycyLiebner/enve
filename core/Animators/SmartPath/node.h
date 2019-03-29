#ifndef NODE_H
#define NODE_H
#include "pointhelpers.h"
#include "exceptions.h"

struct Node {
    friend class NodeList;
    friend class ListOfNodes;
    enum Type : char {
        DISSOLVED, NORMAL, NONE
    };

    Node() { fType = NONE; }

    Node(const QPointF& c0, const QPointF& p1, const QPointF& c2) {
        fC0 = c0;
        fP1 = p1;
        fC2 = c2;
        fType = NORMAL;
    }

    Node(const qreal& t) {
        fT = t;
        fType = DISSOLVED;
    }

    bool isNormal() const { return fType == NORMAL; }

    bool isDissolved() const { return fType == DISSOLVED; }

    int getNodeId() const {
        return fId;
    }
    static Node sInterpolateNormal(const Node &node1, const Node &node2,
                                   const qreal& weight2);

    static Node sInterpolateDissolved(const Node &node1, const Node &node2,
                                      const qreal &weight2);

    QPointF fC0;
    QPointF fP1;
    QPointF fC2;

    QPointF getC0() const {
        if(fC0Enabled) return fC0;
        return fP1;
    }

    QPointF getC2() const {
        if(fC2Enabled) return fC2;
        return fP1;
    }

    //! @brief T value for segment defined by previous and next normal node
    qreal fT;

    Type getType() const { return fType; }
    CtrlsMode getCtrlsMode() const { return fCtrlsMode; }
    bool getC0Enabled() const {
        return fC0Enabled;
    }

    bool getC2Enabled() const {
        return fC2Enabled;
    }

    void applyTransform(const QMatrix &transform) {
        fC0 = transform.map(fC0);
        fP1 = transform.map(fP1);
        fC2 = transform.map(fC2);
    }
protected:
    void setNodeId(const int& nodeId) {
        fId = nodeId;
    }

    void setType(const Type& type) {
        fType = type;
    }

    void setCtrlsMode(const CtrlsMode& ctrlsMode) {
        fCtrlsMode = ctrlsMode;
        if(ctrlsMode == CtrlsMode::CTRLS_SYMMETRIC) {
            gGetCtrlsSymmetricPos(fC0, fP1, fC2, fC0, fC2);
        } else if(ctrlsMode == CtrlsMode::CTRLS_SMOOTH) {
            gGetCtrlsSmoothPos(fC0, fP1, fC2, fC0, fC2);
        } else return;
        setC0Enabled(true);
        setC2Enabled(true);
    }

    void setC0Enabled(const bool& enabled) {
        fC0Enabled = enabled;
    }

    void setC2Enabled(const bool& enabled) {
        fC2Enabled = enabled;
    }
private:
    bool fC0Enabled = true;
    bool fC2Enabled = true;
    Type fType;
    int fId = -1;
    CtrlsMode fCtrlsMode = CtrlsMode::CTRLS_SYMMETRIC;
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

    void insert(const int& id, const Node& nodeBlueprint) {
        Node * const newNode = insertNewNode(id);
        *newNode = nodeBlueprint;
        newNode->setNodeId(id);
    }

    void clear() {
        mList.clear();
    }

    void swap(ListOfNodes& other) {
        mList.swap(other.getList());
    }

    Node* at(const int& id) const {
        if(id < 0 || id >= count())
            RuntimeThrow("Index out of range.");
        return mList.at(id).get();
    }

    stdsptr<Node> atSPtr(const int& id) const {
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
        mList = other.getList();
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

    void moveNode(const int& fromId, const int& toId) {
        mList.move(fromId, toId);
        updateNodeIds(qMin(fromId, toId));
    }

    Node * operator[](const int& id) const {
        return at(id);
    }

    void removeAt(const int& id) {
        mList.removeAt(id);
        updateNodeIds(id);
    }

    void reverse() {
        const auto cpy = mList;
        mList.clear();
        for(const auto& node : cpy) {
            const auto oldC0 = node->fC0;
            node->fC0 = node->fC2;
            node->fC2 = oldC0;
            mList.prepend(node);
        }
        updateNodeIds();
    }

    void moveNodesToFrontStartingWith(const int& first) {
        for(int i = first; i < mList.count(); i++)
            mList.prepend(mList.takeLast());
        updateNodeIds();
    }

    ListOfNodes detachNodesStartingWith(const int& first) {
        QList<stdsptr<Node>> detached;
        const int iniCount = mList.count();
        for(int i = first; i < iniCount; i++) {
            detached.prepend(mList.takeLast());
        }
        return ListOfNodes(detached);
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
protected:
    QList<stdsptr<Node>> &getList() {
        return mList;
    }

    const QList<stdsptr<Node>> &getList() const {
        return mList;
    }
private:
    ListOfNodes(const QList<stdsptr<Node>>& list) : mList(list) {
        updateNodeIds();
    }

    void updateNodeIds() {
        updateNodeIds(0);
    }

    void updateNodeIds(const int& minId) {
        const int maxId = mList.count() - 1;
        updateNodeIds(minId, maxId);
    }

    void updateNodeIds(const int& minId, const int& maxId) {
        for(int i = minId; i <= maxId; i++)
            updateNodeId(i);
    }

    void updateNodeId(const int& id) {
        Node * const iNode = mList.at(id).get();
        iNode->setNodeId(id);
    }

    Node* insertNewNode(const int& id) {
        const auto newNode = stdsptr<Node>(new Node);
        mList.insert(id, newNode);
        updateNodeIds(id);
        return newNode.get();
    }

    QList<stdsptr<Node>> mList;
};

#endif // NODE_H
