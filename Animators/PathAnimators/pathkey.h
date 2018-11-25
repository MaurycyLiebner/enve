#ifndef PATHKEY_H
#define PATHKEY_H
#include "key.h"
#include "skiaincludes.h"
struct NodeSettings;
class VectorPathAnimator;

enum CtrlsMode : short;

class PathContainer {
public:
    PathContainer() {}
    const QList<SkPoint> &getElementsPosList() const {
        return mElementsPos;
    }
    const SkPoint &getElementPos(const int &index) const;
    virtual void setElementPos(const int &index,
                               const SkPoint &pos);
    void addNodeElements(int startPtIndex,
                 const SkPoint &startPos,
                 const SkPoint &pos,
                 const SkPoint &endPos,
                 const bool &saveUndoRedo = true);
    void removeElementPosAt(const int &index,
                            const bool &saveUndoRedo = true);
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

    virtual void removeNodeAt(const int &nodeId,
                              const bool &saveUndoRedo = true) {
        int nodePtId = nodeIdToPointId(nodeId);
        removeNodeElements(nodePtId, saveUndoRedo);
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
        removeNodeElements(nodePtId - 1);
    }

    virtual void setCtrlsModeForNode(const int &nodeId, const CtrlsMode &mode);

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

    void schedulePathUpdate() {
        mPathUpdateNeeded = true;
    }

    virtual void revertAllPoints() {
        revertElementPosSubset(0, -1);
        PathContainer::shiftAllPoints(1);
        mPathUpdateNeeded = true;
    }

    virtual void startPathChange() {
        if(mPathChanged) return;
        mPathChanged = true;
        mSavedElementsPos = mElementsPos;
        return;
    }

    virtual void cancelPathChange() {
        mPathChanged = false;
    }

    virtual void finishedPathChange();

    void setElementsPos(const QList<SkPoint> &newElementsPos,
                        const bool &saveUndoRedo) {
        if(saveUndoRedo) {
            startPathChange();
        }
        mElementsPos = newElementsPos;
        if(saveUndoRedo) {
            finishedPathChange();
        }
    }
    void removeNodeElements(const int &startPtIndex,
                            const bool &saveUndoRedo = true);

    virtual void updateAfterChangedFromInside() = 0;

    virtual void mergeNodes(const int &nodeId1,
                            const int &nodeId2);
protected:
    bool mPathChanged = false;
    bool mPathClosed = false;
    bool mPathUpdateNeeded = false;
    SkPath mPath;
    QList<SkPoint> mElementsPos;
    QList<SkPoint> mSavedElementsPos;
};

class PathKey : public Key, public PathContainer {
public:
    NodeSettings *getNodeSettingsForPtId(const int &ptId);
    bool differsFromKey(Key *key) { return key != this; }
    void writeKey(QIODevice *target);
    void readKey(QIODevice *target);

    PathKeySPtr createNewKeyFromSubsetForPath(
            const VectorPathAnimatorQSPtr& parentAnimator,
            const int &firstId, int count);
    void updateAfterChangedFromInside();
protected:
    PathKey(const VectorPathAnimatorQSPtr &parentAnimator);
    PathKey(const int &relFrame,
            const SkPath &path,
            const VectorPathAnimatorQSPtr &parentAnimator,
            const bool &closed = false);
    PathKey(const int &relFrame,
            const SkPath &path,
            const QList<SkPoint> &elementsPos,
            const VectorPathAnimatorQSPtr &parentAnimator,
            const bool &closed);
    PathKey(const int &relFrame,
            const QList<SkPoint> &elementsPos,
            const VectorPathAnimatorQSPtr &parentAnimator,
            const bool &closed);
};

#endif // PATHKEY_H
