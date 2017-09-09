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
    virtual void setPathClosed(const bool &bT);
    virtual void setElementsFromSkPath(const SkPath &path);
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
            int firstId,
            int count,
            int targetId) {
        if(firstId == targetId) return;
        if(targetId == -1) {
            targetId = mElementsPos.count() - count;
        }
        if(count == -1) {
            count = mElementsPos.count() - firstId;
        }
        int lastId = firstId + count - 1;
        if(targetId > firstId) {
            int takeId = lastId + 1;
            int moveTo = firstId;
            for(int i = 0; i < targetId - firstId; i++) {
                mElementsPos.move(takeId, moveTo);
                takeId++;
                moveTo++;
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
    void readPathContainer(std::fstream *file);
    void writePathContainer(std::fstream *file);
protected:
    bool mPathClosed = false;
    bool mPathUpdateNeeded = false;
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
    PathKey(const int &relFrame,
            const SkPath &path,
            const QList<SkPoint> &elementsPos,
            VectorPathAnimator *parentAnimator,
            const bool &closed);

    NodeSettings *getNodeSettingsForPtId(const int &ptId);
    bool differsFromKey(Key *key) { return key != this; }
    void writePathKey(std::fstream *file);
    void readPathKey(std::fstream *file);
};

#endif // PATHKEY_H
