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
class BasicTransformAnimator;
enum CanvasMode : short;

class VectorPathAnimator : public InterpolationAnimator,
                           public PathContainer {
    Q_OBJECT
    friend class SelfRef;
protected:
    VectorPathAnimator(PathAnimator * const pathAnimator);
    VectorPathAnimator(const QList<const NodeSettings*> &settingsList,
                       PathAnimator * const pathAnimator);
    VectorPathAnimator(const QList<const NodeSettings*> &settingsList,
                       const QList<SkPoint> &posList,
                       PathAnimator * const pathAnimator);
public:
    bool SWT_isVectorPathAnimator() const { return true; }
    void prp_updateAfterChangedRelFrameRange(const FrameRange& range) {
        if(range.inRange(anim_getCurrentRelFrame())) {
            mElementsUpdateNeeded = true;
        }
        InterpolationAnimator::prp_updateAfterChangedRelFrameRange(range);
    }

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

    void anim_saveCurrentValueAsKey();
    void anim_addKeyAtRelFrame(const int &relFrame);
    stdsptr<Key> readKey(QIODevice *target);

    SkPath getPathAtRelFrame(const qreal &relFrame);

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
    void updateNodePointsFromData();
    PathAnimator *getParentPathAnimator();

    void finalizeNodesRemove();

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
    void selectAllPoints(Canvas * const canvas);
    void drawSelected(SkCanvas *canvas,
                      const CanvasMode &currentCanvasMode,
                      const SkScalar &invScale,
                      const SkMatrix &totalTransform);
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
