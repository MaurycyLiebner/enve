#ifndef PATHKEY_H
#define PATHKEY_H
#include "key.h"
#include "skiaincludes.h"
struct NodeSettings;
class VectorPathAnimator;
class PathContainer {
public:
    PathContainer() {}
    const QList<SkPoint> &getElementsPosList() const {
        return mElementsPos;
    }
    const SkPoint &getElementPos(const int &index) const;
    virtual void setElementPos(const int &index,
                               const SkPoint &pos);
    void prependElementPos(const SkPoint &pos);
    void appendElementPos(const SkPoint &pos);
    void insertElementPos(const int &index,
                          const SkPoint &pos);
    void removeElementPosAt(const int &index);
    const SkPath &getPath();
    void updatePath();
    void closedChanged(const bool &bT);
    void setElementsFromSkPath(const SkPath &path);
    void clearElements();

    void addNewPointAtTBetweenPts(const SkScalar &tVal,
                                  const int &id1,
                                  const int &id2,
                                  const bool &newPtSmooth);

    virtual NodeSettings *getNodeSettingsForPtId(const int &ptId) = 0;
    void setCurrentPath(const SkPath &path) {
        mPath = path;
    }

    int pointIdToNodeId(const int &ptId) {
        return ptId/3;
    }

    int nodeIdToPointId(const int &nodeId) {
        return nodeId*3 + 1;
    }

    virtual void removeNodeAt(const int &nodeId) {
        int nodePtId = nodeIdToPointId(nodeId);
        removeElementPosAt(nodePtId - 1);
        removeElementPosAt(nodePtId - 1);
        removeElementPosAt(nodePtId - 1);
    }

    virtual void removeNodeAtAndApproximate(const int &nodeId) {
        int nodePtId = nodeIdToPointId(nodeId);
        removeElementPosAt(nodePtId - 1);
        removeElementPosAt(nodePtId - 1);
        removeElementPosAt(nodePtId - 1);
    }

    virtual void moveElementPosSubset(
            const int &firstId,
            const int &count,
            int targetId) {
        if(firstId == targetId) return;
        int lastId = firstId + count - 1;
        if(targetId > firstId) {
            int takeId = lastId + 1;
            for(int i = 0; i < targetId - firstId; i++) {
                mElementsPos.move(takeId, firstId);
            }
        } else {
            int takeId = firstId - 1;
            for(int i = 0; i < firstId - targetId; i++) {
                mElementsPos.move(takeId, lastId + 1);
                takeId--;
            }
        }
    }

    virtual void revertElementPosSubset(
            const int &firstId,
            const int &count) {
        int lastId = firstId + count - 1;
        for(int i = 0; i < count; i++) {
            mElementsPos.move(lastId, firstId + i);
        }
    }
protected:
    bool mPathClosed = false;
    bool mPathNeedUpdate = false;
    SkPath mPath;
    QList<SkPoint> mElementsPos;
};

class PathKey : public Key,
                public PathContainer {
public:
    PathKey(VectorPathAnimator *parentAnimator);
    PathKey(const int &relFrame,
            const SkPath &path,
            VectorPathAnimator *parentAnimator);
    NodeSettings *getNodeSettingsForPtId(const int &ptId);
    bool differsFromKey(Key *key) { return true; }
};

#endif // PATHKEY_H
