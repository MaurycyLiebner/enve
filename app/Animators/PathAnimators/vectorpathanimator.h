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
class SvgNodePoint;
class BasicTransformAnimator;
enum CanvasMode : short;
struct NodeSettings : public StdSelfRef {
    friend class StdSelfRef;
    NodeSettings() {}
    NodeSettings(const NodeSettings *settings) {
        if(settings == nullptr) return;
        copyFrom(settings);
    }
    NodeSettings(const bool &startEnabledT,
                 const bool &endEnabledT,
                 const CtrlsMode &ctrlsModeT) {
        set(startEnabledT, endEnabledT, ctrlsModeT);
    }

    void copyFrom(const NodeSettings *settings) {
        if(settings == nullptr) {
            startEnabled = false;
            endEnabled = false;
            ctrlsMode = CtrlsMode::CTRLS_CORNER;
        } else {
            startEnabled = settings->startEnabled;
            endEnabled = settings->endEnabled;
            ctrlsMode = settings->ctrlsMode;
        }
    }

    void set(const bool &startEnabledT,
             const bool &endEnabledT,
             const CtrlsMode &ctrlsModeT) {
        startEnabled = startEnabledT;
        endEnabled = endEnabledT;
        ctrlsMode = ctrlsModeT;
    }

    bool startEnabled = false;
    bool endEnabled = false;
    CtrlsMode ctrlsMode = CtrlsMode::CTRLS_CORNER;

    void write(QIODevice *target);
    void read(QIODevice *target);
};

class VectorPathAnimator : public Animator,
                           public PathContainer {
    Q_OBJECT
    friend class SelfRef;
public:
    void prp_setAbsFrame(const int &frame);
    SkPath getPathAtRelFrame(const int &relFrame,
                             const bool &considerCurrent = true,
                             const bool &interpolate = true);
    SkPath getPathAtRelFrameF(const qreal &relFrame,
                              const bool &considerCurrent = true,
                              const bool &interpolate = true);

    NodeSettings *getNodeSettingsForPtId(const int &ptId);

    NodeSettings *getNodeSettingsForNodeId(const int &nodeId);

    void replaceNodeSettingsForPtId(const int &ptId,
                                    const NodeSettings *settings);

    void replaceNodeSettingsForNodeId(const int &nodeId,
                                      const NodeSettings *settings,
                                      const bool &saveUndoRedo = true);

    NodeSettings* insertNodeSettingsForNodeId(const int &nodeId,
                                     const NodeSettings *settings,
                                     const bool &saveUndoRedo = true);
    void insertNodeSettingsForNodeId(const int &nodeId,
                                     const stdsptr<NodeSettings> &newSettings,
                                     const bool &saveUndoRedo = true);

    void removeNodeSettingsAt(const int &id,
                              const bool &saveUndoRedo = true);

    void setNodeStartEnabled(const int &nodeId,
                             const bool &enabled);

    void setNodeEndEnabled(const int &nodeId,
                           const bool &enabled);

    void setNodeCtrlsMode(const int &nodeId,
                          const CtrlsMode &ctrlsMode);

    void setPathClosed(const bool &bT);

    const CtrlsMode &getNodeCtrlsMode(const int &nodeId);
    void anim_saveCurrentValueToKey(PathKey *key);

    void finishedPathChange();

    void anim_saveCurrentValueAsKey();
    void anim_addKeyAtRelFrame(const int &relFrame);

    void anim_removeKey(const stdsptr<Key> &keyToRemove,
                        const bool &saveUndoRedo);
    NodePoint *createNewPointOnLineNear(const QPointF &absPos,
                                        const bool &adjust,
                                        const qreal &canvasScaleInv);
    void updateNodePointsFromElements();
    PathAnimator *getParentPathAnimator();

    void finalizeNodesRemove();

    void removeNodeAtAndApproximate(const int &nodeId);
    void removeNodeAt(const int &nodeId,
                      const bool &saveUndoRedo = true);

    NodePoint *addNodeRelPos(const SvgNodePoint *svgPoint,
                             NodePoint *targetPt);
    NodePoint *addNodeAbsPos(const QPointF &absPos,
                             NodePoint *targetPt);
    NodePoint *addNodeRelPos(const QPointF &relPos,
                             NodePoint *targetPt);
    NodePoint *addNodeRelPos(const QPointF &startRelPos,
                             const QPointF &relPos,
                             const QPointF &endRelPos,
                             NodePoint *targetPt);

    NodePoint *addNodeRelPos(const QPointF &startRelPos,
                             const QPointF &relPos,
                             const QPointF &endRelPos,
                             const bool &startEnabled,
                             const bool &endEnabled,
                             const CtrlsMode &ctrlsMode,
                             NodePoint* targetPt,
                             const bool &saveUndoRedo = true);
    NodePoint *addNodeRelPos(const QPointF &startRelPos,
                             const QPointF &relPos,
                             const QPointF &endRelPos,
                             const bool& startEnabled,
                             const bool& endEnabled,
                             const CtrlsMode& ctrlsMode,
                             const int &targetPtId,
                             const bool &saveUndoRedo = true);
    VectorPathEdge *getEdge(const QPointF &absPos,
                            const qreal &canvasScaleInv);
    void selectAllPoints(Canvas *canvas);
    void applyTransformToPoints(const QMatrix &transform);
    void drawSelected(SkCanvas *canvas,
                      const CanvasMode &currentCanvasMode,
                      const SkScalar &invScale,
                      const SkMatrix &combinedTransform);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<stdptr<MovablePoint>> &list);
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);
    void setParentPath(PathAnimator *parentPath);

    void finishAllPointsTransform();
    void startAllPointsTransform();
    void disconnectPoints(NodePoint *pt1,
                          NodePoint *pt2);
    void connectPoints(NodePoint *pt1,
                       NodePoint *pt2);
    void appendToPointsList(const stdsptr<NodePoint> &pt);

    void setElementPos(const int &index,
                       const SkPoint &pos);
    void setElementsFromSkPath(const SkPath &path);
    void startPathChange();
    void cancelPathChange();
    void readProperty(QIODevice *target);
    void writeProperty(QIODevice *target);

    bool SWT_isVectorPathAnimator();
    void anim_moveKeyToRelFrame(Key *key,
                                const int &newFrame,
                                const bool &saveUndoRedo = true,
                                const bool &finish = true);
    void anim_appendKey(const stdsptr<Key> &newKey,
                        const bool &saveUndoRedo = true,
                        const bool &update = true);

    void moveElementPosSubset(int firstId, int count, int targetId);
    void revertElementPosSubset(const int &firstId, int count);
    void revertNodeSettingsSubset(const int &firstId, int count);

    void getNodeSettingsList(QList<stdsptr<NodeSettings>>& nodeSettingsList);

    const QList<stdsptr<NodeSettings>> &getNodeSettingsList();

    void getElementPosList(QList<SkPoint> *elementPosList);

    void getKeysList(QList<stdsptr<PathKey>>& pathKeyList);

    static void getKeysDataForConnection(VectorPathAnimator *targetPath,
                                  VectorPathAnimator* srcPath,
                                  QList<int> &keyFrames,
                                  QList<QList<SkPoint> > &newKeysData,
                                  const bool &addSrcFirst);

    VectorPathAnimator *connectWith(VectorPathAnimator *srcPath);
    stdsptr<Key> readKey(QIODevice *target);
    void shiftAllPointsForAllKeys(const int &by);
    void revertAllPointsForAllKeys();

    void shiftAllNodeSettings(const int &by);

    void shiftAllPoints(const int &by);

    void revertAllPoints();

    void updateAfterChangedFromInside();

    void removeFromParent();

    NodePoint *getNodePtWithNodeId(const int &id);

    int getNodeCount();

    void mergeNodes(const int &nodeId1,
                    const int &nodeId2);

    const bool &isClosed() const;
    void setCtrlsModeForNode(const int &nodeId, const CtrlsMode &mode);

    void prp_updateInfluenceRangeAfterChanged() {
        Animator::prp_updateInfluenceRangeAfterChanged();
        prp_callUpdater();
    }
protected:
    VectorPathAnimator(PathAnimator *pathAnimator);
    VectorPathAnimator(const QList<const NodeSettings*> &settingsList,
                       PathAnimator *pathAnimator);
    VectorPathAnimator(const QList<const NodeSettings *> &settingsList,
                       const QList<SkPoint> &posList,
                       PathAnimator *pathAnimator);
private:
    void revertAllNodeSettings();
    void setFirstPoint(NodePoint *pt);

    NodePoint *createNewNode(const int &targetNodeId,
                             const QPointF &startRelPos,
                             const QPointF &relPos,
                             const QPointF &endRelPos,
                             const bool &startEnabled,
                             const bool &endEnabled,
                             const CtrlsMode &ctrlsMode);

    void updateNodePointIds();

    bool getTAndPointsForMouseEdgeInteraction(const QPointF &absPos,
                                              qreal &pressedT,
                                              NodePoint **prevPoint,
                                              NodePoint **nextPoint,
                                              const qreal &canvasScaleInv);
    NodePoint *createNodePointAndAppendToList();

    bool mElementsUpdateNeeded = false;

    qptr<BasicTransformAnimator> mParentTransform;
    qptr<PathAnimator> mParentPathAnimator;
    stdptr<NodePoint>mFirstPoint;

    QList<int> mNodesToRemove;
    QList<stdsptr<NodeSettings>> mNodeSettings;
    QList<stdsptr<NodePoint>> mPoints;
};

#endif // VECTORPATHANIMATOR_H
