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

    static Node sInterpolate(const Node &node1, const Node &node2,
                             const qreal &weight2);

    QPointF fC0;
    QPointF fP1;
    QPointF fC2;

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

    static Node sInterpolateNormal(const Node &node1, const Node &node2,
                                   const qreal& weight2);
};
#include "smartPointers/stdselfref.h"
class ListOfNodes {
public:
    ListOfNodes() {}

    bool isEmpty() const {
        return mList.isEmpty();
    }

    void append(const Node& nodeBlueprint) {
        insert(mList.count(), nodeBlueprint);
    }

    void insert(const int& id, const Node& nodeBlueprint) {
        Node * const newNode = insertNewNode(id);
        *newNode = nodeBlueprint;
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
    }

    Node * operator[](const int& id) const {
        return at(id);
    }

    void removeAt(const int& id) {
        mList.removeAt(id);
    }

    void reverse() {
        const auto cpy = mList;
        mList.clear();
        for(const auto& node : cpy) {
            mList.prepend(node);
        }
    }

    void moveNodesToFrontStartingWith(const int& first) {
        for(int i = first; i < mList.count(); i++) {
            mList.prepend(mList.takeLast());
        }
    }

    ListOfNodes detachNodesStartingWith(const int& first) {
        QList<stdsptr<Node>> detached;
        const int iniCount = mList.count();
        for(int i = first; i < iniCount; i++) {
            detached.prepend(mList.takeLast());
        }
        return ListOfNodes(detached);
    }

    auto begin() const {
        return mList.begin();
    }

    auto end() const {
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
    ListOfNodes(const QList<stdsptr<Node>>& list) : mList(list) {}

    Node* insertNewNode(const int& id) {
        const auto newNode = stdsptr<Node>(new Node);
        mList.insert(id, newNode);
        return newNode.get();
    }

    QList<stdsptr<Node>> mList;
};

#endif // NODE_H
