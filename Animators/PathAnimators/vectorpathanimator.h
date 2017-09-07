#ifndef VECTORPATHANIMATOR_H
#define VECTORPATHANIMATOR_H
#include "Animators/animator.h"
#include "skiaincludes.h"
#include "pointhelpers.h"
#include "pathkey.h"

class PathAnimator;
class NodePoint;
class VectorPathEdge;
class Canvas;
class MovablePoint;
enum CanvasMode : short;
struct NodeSettings {
    NodeSettings() {}
    NodeSettings(const bool &startEnabledT,
                 const bool &endEnabledT,
                 const CtrlsMode &ctrlsModeT) {
        startEnabled = startEnabledT;
        endEnabled = endEnabledT;
        ctrlsMode = ctrlsModeT;
    }

    bool startEnabled = false;
    bool endEnabled = false;
    CtrlsMode ctrlsMode = CtrlsMode::CTRLS_CORNER;
};

class VectorPathAnimator : public Animator,
                           public PathContainer {
    Q_OBJECT
public:
    VectorPathAnimator(PathAnimator *pathAnimator);

    void prp_setAbsFrame(const int &frame);
    SkPath getPathAtRelFrame(const int &relFrame,
                             const bool &considerCurrent = true);

    NodeSettings *getNodeSettingsForPtId(const int &ptId) {
        return mNodeSettings.at(pointIdToNodeId(ptId));
    }

    NodeSettings *getNodeSettingsForNodeId(const int &nodeId) {
        return mNodeSettings.at(nodeId);
    }

    void replaceNodeSettingsForPtId(const int &ptId,
                                    const NodeSettings &settings) {
        replaceNodeSettingsForNodeId(pointIdToNodeId(ptId), settings);
    }

    void replaceNodeSettingsForNodeId(const int &nodeId,
                                      const NodeSettings &settings) {
        *mNodeSettings.at(nodeId) = settings;
    }

    NodeSettings *insertNodeSettingsForNodeId(const int &nodeId,
                                              const NodeSettings &settings) {
        NodeSettings *newSettings = new NodeSettings;
        *newSettings = settings;
        mNodeSettings.insert(nodeId, newSettings);
        return newSettings;
    }

    void setNodeStartEnabled(const int &nodeId,
                             const bool &enabled) {
        NodeSettings *settings = getNodeSettingsForNodeId(nodeId);
        settings->startEnabled = enabled;
    }

    void setNodeEndEnabled(const int &nodeId,
                           const bool &enabled) {
        NodeSettings *settings = getNodeSettingsForNodeId(nodeId);
        settings->endEnabled = enabled;
    }

    void setNodeCtrlsMode(const int &nodeId,
                          const CtrlsMode &ctrlsMode) {
        NodeSettings *settings = getNodeSettingsForNodeId(nodeId);
        settings->ctrlsMode = ctrlsMode;
    }

    void setPathClosed(const bool &bT) {
        PathContainer::setPathClosed(bT);
        foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
            ((PathKey*)key.get())->setPathClosed(bT);
        }
        setElementsFromSkPath(getPathAtRelFrame(anim_mCurrentRelFrame));
        prp_updateInfluenceRangeAfterChanged();
    }

    const CtrlsMode &getNodeCtrlsMode(const int &nodeId) {
        return getNodeSettingsForNodeId(nodeId)->ctrlsMode;
    }
    void anim_saveCurrentValueToKey(PathKey *key);

    void setCurrentPosForPtWithId(const int &ptId,
                                  const SkPoint &pos,
                                  const bool &finish);

    void finishedPathChange();

    void anim_saveCurrentValueAsKey();
    void anim_removeKey(Key *keyToRemove, const bool &saveUndoRedo);
    NodePoint *createNewPointOnLineNear(const QPointF &absPos,
                                        const bool &adjust,
                                        const qreal &canvasScaleInv);
    void updateNodePointsFromCurrentPath();
    PathAnimator *getParentPathAnimator() {
        return mParentPathAnimator;
    }

    void removeNodeAtAndApproximate(const int &nodeId);
    void removeNodeAt(const int &nodeId);

    NodePoint *addNodeAbsPos(const QPointF &absPos,
                              NodePoint *targetPt);
    NodePoint *addNodeRelPos(const QPointF &relPos,
                             NodePoint *targetPt);
    NodePoint *addNodeRelPos(const QPointF &startRelPos,
                             const QPointF &relPos,
                             const QPointF &endRelPos,
                             NodePoint *targetPt,
                             const NodeSettings &nodeSettings =
            NodeSettings());
    VectorPathEdge *getEdge(const QPointF &absPos,
                            const qreal &canvasScaleInv);
    void selectAllPoints(Canvas *canvas);
    void applyTransformToPoints(const QMatrix &transform);
    void drawSelected(SkCanvas *canvas,
                      const CanvasMode &currentCanvasMode,
                      const qreal &invScale,
                      const SkMatrix &combinedTransform);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<MovablePoint *> *list);
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);
    void setParentPath(PathAnimator *parentPath) {
        mParentPathAnimator = parentPath;
    }

    VectorPathAnimator *makeDuplicate() {
        VectorPathAnimator *path =
                new VectorPathAnimator(mParentPathAnimator);
        return path;
    }

    void finishAllPointsTransform();
    void startAllPointsTransform();
    void disconnectPoints(NodePoint *pt1,
                          NodePoint *pt2);
    void connectPoints(NodePoint *pt1,
                       NodePoint *pt2);
    void appendToPointsList(NodePoint *pt);

    void setElementPos(const int &index,
                       const SkPoint &pos);
    void setElementsFromSkPath(const SkPath &path);
    void startPathChange();
    void cancelPathChange();
    void readVectorPathAnimator(std::fstream *file);
    void writeVectorPathAnimator(std::fstream *file);

    bool SWT_isVectorPathAnimator() { return true; }
    void anim_moveKeyToRelFrame(Key *key,
                                const int &newFrame,
                                const bool &saveUndoRedo = true,
                                const bool &finish = true);
    void anim_appendKey(Key *newKey,
                        const bool &saveUndoRedo = true,
                        const bool &update = true);

private:
    NodePoint *createNewNode(const int &targetNodeId,
                             const QPointF &startRelPos,
                             const QPointF &relPos,
                             const QPointF &endRelPos,
                             const NodeSettings &nodeSettings);
    void updateNodePointIds();

    bool getTAndPointsForMouseEdgeInteraction(const QPointF &absPos,
                                              qreal *pressedT,
                                              NodePoint **prevPoint,
                                              NodePoint **nextPoint,
                                              const qreal &canvasScaleInv);
    QList<NodeSettings*> mNodeSettings;
    PathAnimator *mParentPathAnimator;
    NodePoint *mFirstPoint = NULL;
    QList<NodePoint*> mPoints;
    bool mPathChanged = false;
    bool mElementsUpdateNeeded = false;
};

#endif // VECTORPATHANIMATOR_H
