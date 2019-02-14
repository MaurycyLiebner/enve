#ifndef SMARTPATHCONTAINER_H
#define SMARTPATHCONTAINER_H
#include "simplemath.h"
#include "skia/skiaincludes.h"
#include "exceptions.h"
#include "pointhelpers.h"
#include "nodelist.h"

#include "framerange.h"
#include "smartPointers/stdselfref.h"
#include "smartPointers/stdpointer.h"

class SmartPath : public StdSelfRef {
    friend class StdSelfRef;
    enum Neighbour { NONE, NEXT, PREV, BOTH = NEXT | PREV };
public:
    void actionRemoveNormalNode(const int& nodeId);

    void actionAddFirstNode(const QPointF& c0,
                            const QPointF& p1,
                            const QPointF& c2);

    void actionInsertNodeBetween(const int &prevId,
                                 const int& nextId,
                                 const QPointF &c0,
                                 const QPointF &p1,
                                 const QPointF &c2);
    void actionInsertNodeBetween(const int &prevId,
                                 const int& nextId,
                                 const qreal& t);

    void actionPromoteDissolvedNodeToNormal(const int& nodeId);

    void actionDisconnectNodes(const int& node1Id, const int& node2Id);

    void actionConnectNodes(const int& node1Id, const int& node2Id);

    int dissolvedOrDummyNodeInsertedToNeigh(const int &targetNodeId,
                                            const Neighbour &neigh);
    void normalOrMoveNodeInsertedToNeigh(const int &targetNodeId,
                                         const Neighbour& neigh);

    void removeNodeWithIdAndTellPrevToDoSame(const int& nodeId);

    void removeNodeWithIdAndTellNextToDoSame(const int& nodeId);

    QList<int> updateAllNodesTypeAfterNeighbourChanged();

    void setPrev(SmartPath * const prev);
    void setNext(SmartPath * const next);

    NodeList *getNodes() const;

    SkPath getPathAt() const;
    SkPath getPathForPrev() const;
    SkPath getPathForNext() const;

    SkPath interpolateWithNext(const qreal& nextWeight) const;
    SkPath interpolateWithPrev(const qreal& nextWeight) const;
protected:
    SmartPath();
    SmartPath(const QList<Node>& nodes);
private:
    SkPath getPathFor(SmartPath * const neighbour) const;
    void insertNodeBetween(const int &prevId, const int &nextId,
                           const Node &nodeBlueprint);
    void moveNodeBefore(const int &moveNodeId, Node &moveNode,
                        const int &beforeNodeId, Node &beforeNode);
    void moveNodeAfter(const int &moveNodeId, Node &moveNode,
                       const int &afterNodeId, Node &afterNode);

    stdptr<SmartPath> mPrev;
    stdptr<SmartPath> mNext;
    stdsptr<NodeList> mNodes;
};

#endif // SMARTPATHCONTAINER_H
