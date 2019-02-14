#ifndef NODE_H
#define NODE_H
#include <QPointF>

struct Node {
    enum Type : char {
        DUMMY, DISSOLVED, NORMAL, MOVE
    };

    Node() : Node(DUMMY) {}

    Node(const Type& type) { mType = type; }

    Node(const QPointF& c0, const QPointF& p1, const QPointF& c2) {
        fC0 = c0;
        fP1 = p1;
        fC2 = c2;
        mType = NORMAL;
    }

    Node(const qreal& t) {
        fT = t;
        mType = DISSOLVED;
    }

    bool isMove() const { return mType == MOVE; }

    bool isNormal() const { return mType == NORMAL; }

    bool isDummy() const { return mType == DUMMY; }

    bool isDissolved() const { return mType == DISSOLVED; }

    void switchPrevAndNext() {
        const int prevT = mPrevNodeId;
        mPrevNodeId = mNextNodeId;
        mNextNodeId = prevT;
    }

    int getNextNodeId() const {
        return mNextNodeId;
    }
    int getPrevNodeId() const {
        return mPrevNodeId;
    }

    bool hasPreviousNode() const {
        return getPrevNodeId() >= 0;
    }

    bool hasNextNode() const {
        return getNextNodeId() >= 0;
    }

    void setNextNodeId(const int& nextNodeId) {
        mNextNodeId = nextNodeId;
    }

    void setPrevNodeId(const int& prevNodeId) {
        mPrevNodeId = prevNodeId;
    }

    QPointF fC0;
    QPointF fP1;
    QPointF fC2;

    //! @brief T value for segment defined by previous and next normal node
    qreal fT;

    void setType(const Type& type) {
        mType = type;
    }

    const Type& getType() const { return mType; }

    void shiftIdsGreaterThan(const int& greater, const int& shiftBy) {
        if(mNextNodeId) if(mNextNodeId > greater) mNextNodeId += shiftBy;
        if(mPrevNodeId) if(mPrevNodeId > greater) mPrevNodeId += shiftBy;
    }

    void shiftIdsSmallerThan(const int& smaller, const int& shiftBy) {
        if(mNextNodeId) if(mNextNodeId < smaller) mNextNodeId += shiftBy;
        if(mPrevNodeId) if(mPrevNodeId < smaller) mPrevNodeId += shiftBy;
    }
private:
    Type mType;

    //! @brief Next connected node id in the list.
    int mNextNodeId = -1;

    //! @brief Previous connected node id in the list.
    int mPrevNodeId = -1;
};

#endif // NODE_H
