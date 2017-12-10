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
        bool isLast = nodePtId == 1 ||
                (nodePtId == mElementsPos.count() - 2 && !mPathClosed);
        if(!isLast) {
            int prevCtrlId = nodePtId - 2;
            if(prevCtrlId < 0) prevCtrlId = mElementsPos.count() - 1;
            int nextCtrlId = nodePtId + 2;
            if(nextCtrlId >= mElementsPos.count()) nextCtrlId = 0;
            // make approximate
        }
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

    virtual void applyTransformToPoints(const QMatrix &transform);

    virtual void revertElementPosSubset(
            const int &firstId,
            int count) {
        if(count == -1) {
            count = mElementsPos.count() - firstId;
        }
        int lastId = firstId + count - 1;
        for(int i = 0; i < count; i++) {
            mElementsPos.move(lastId, firstId + i);
        }
    }

    QList<SkPoint> takeElementsPosSubset(const int &firstId,
                                         int count) {
        if(count == -1) {
            count = mElementsPos.count() - firstId;
        }
        QList<SkPoint> elements;
        for(int i = 0; i < count; i++) {
            elements << mElementsPos.takeAt(firstId);
        }
        setPathClosed(false);
        return elements;
    }

    void readPathContainer(QIODevice *target);
    void writePathContainer(QIODevice *target);
    static QList<SkPoint> extractElementsFromSkPath(const SkPath &path);

    virtual void shiftAllPoints(const int &by) {
        if(by == 0) return;
        if(mPathClosed) {
            for(int i = 0; i < by*3; i++) {
                mElementsPos.prepend(mElementsPos.takeLast());
            }
            for(int i = 0; i < -by*3; i++) {
                mElementsPos.append(mElementsPos.takeFirst());
            }
            mPathUpdateNeeded = true;
        }
    }

    virtual void revertAllPoints() {
        revertElementPosSubset(0, -1);
        PathContainer::shiftAllPoints(1);
    }
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
    PathKey(const int &relFrame,
            const QList<SkPoint> &elementsPos,
            VectorPathAnimator *parentAnimator,
            const bool &closed);

    NodeSettings *getNodeSettingsForPtId(const int &ptId);
    bool differsFromKey(Key *key) { return key != this; }
    void writeKey(QIODevice *target);
    void readKey(QIODevice *target);

    PathKey *createNewKeyFromSubsetForPath(VectorPathAnimator *parentAnimator,
                                           const int &firstId,
                                           int count);
};

#endif // PATHKEY_H
