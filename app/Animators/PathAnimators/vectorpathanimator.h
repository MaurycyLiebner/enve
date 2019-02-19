#ifndef VECTORPATHANIMATOR_H
#define VECTORPATHANIMATOR_H
#include "Animators/interpolationanimator.h"
#include "skia/skiaincludes.h"
#include "pointhelpers.h"
#include "pathkey.h"
#include "nodesettings.h"
#define GetAsPK(key) GetAsPtr(key, PathKey)

class PathAnimator;
class NodePoint;
class VectorPathEdge;
class Canvas;
class MovablePoint;
class SvgNodePoint;
class BasicTransformAnimator;
enum CanvasMode : short;

class VectorPathAnimator : public InterpolationAnimator,
                           public PathContainer {
    Q_OBJECT
    friend class SelfRef;
public:
    NodeSettings *getNodeSettingsForPtId(const int &ptId);

    void setPathClosed(const bool &bT);
    void finishedPathChange();
    void removeNodeAtAndApproximate(const int &nodeId);
    void removeNodeAt(const int &nodeId);
    void applyTransformToPoints(const QMatrix &transform);
    void setElementPos(const int &index, const SkPoint &pos);
    void setElementsFromSkPath(const SkPath &path);
    void startPathChange();
    void cancelPathChange();
    void moveElementPosSubset(int firstId, int count, int targetId);
    void revertElementPosSubset(const int &firstId, int count);
    void shiftAllPoints(const int &by);
    void revertAllPoints();
    void updateAfterChangedFromInside();
    void mergeNodes(const int &nodeId1, const int &nodeId2);
    void setCtrlsModeForNode(const int &nodeId, const CtrlsMode &mode);

    void anim_setAbsFrame(const int &frame);
    void readProperty(QIODevice *target);
    void writeProperty(QIODevice * const target) const;

    void anim_moveKeyToRelFrame(Key *key, const int &newFrame);
    void anim_appendKey(const stdsptr<Key> &newKey);
    void anim_saveCurrentValueAsKey();
    void anim_addKeyAtRelFrame(const int &relFrame);
    void anim_removeKey(const stdsptr<Key> &keyToRemove);
    stdsptr<Key> readKey(QIODevice *target);
    bool SWT_isVectorPathAnimator() const { return true; }

    SkPath getPathAtRelFrame(const int &relFrame);
    SkPath getPathAtRelFrameF(const qreal &relFrame);

    NodeSettings *getNodeSettingsForNodeId(const int &nodeId);

    void replaceNodeSettingsForPtId(const int &ptId,
                                    const NodeSettings *settings);

    void replaceNodeSettingsForNodeId(const int &nodeId,
                                      const NodeSettings *settings);

    NodeSettings* insertNodeSettingsForNodeId(const int &nodeId,
                                     const NodeSettings *settings);
    void insertNodeSettingsForNodeId(const int &nodeId,
                                     const stdsptr<NodeSettings> &newSettings);

    void removeNodeSettingsAt(const int &id);

    void setNodeStartEnabled(const int &nodeId,
                             const bool &enabled);

    void setNodeEndEnabled(const int &nodeId,
                           const bool &enabled);

    void setNodeCtrlsMode(const int &nodeId,
                          const CtrlsMode &ctrlsMode);

    const CtrlsMode &getNodeCtrlsMode(const int &nodeId);
    void anim_saveCurrentValueToKey(PathKey *key);

    NodePoint *createNewPointOnLineNear(const QPointF &absPos,
                                        const bool &adjust,
                                        const qreal &canvasScaleInv);
    void updateNodePointsFromElements();
    PathAnimator *getParentPathAnimator();

    void finalizeNodesRemove();

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
                             NodePoint* targetPt);
    NodePoint *addNodeRelPos(const QPointF &startRelPos,
                             const QPointF &relPos,
                             const QPointF &endRelPos,
                             const bool& startEnabled,
                             const bool& endEnabled,
                             const CtrlsMode& ctrlsMode,
                             const int &targetPtId);
    VectorPathEdge *getEdge(const QPointF &absPos,
                            const qreal &canvasScaleInv);
    void selectAllPoints(Canvas *canvas);
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
    void shiftAllPointsForAllKeys(const int &by);
    void revertAllPointsForAllKeys();

    void shiftAllNodeSettings(const int &by);

    void removeFromParent();

    NodePoint *getNodePtWithNodeId(const int &id);

    int getNodeCount();

    const bool &isClosed() const;
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
    stdptr<NodePoint> mFirstPoint;

    QList<int> mNodesToRemove;
    QList<stdsptr<NodeSettings>> mNodeSettings;
    QList<stdsptr<NodePoint>> mPoints;
};

#endif // VECTORPATHANIMATOR_H
