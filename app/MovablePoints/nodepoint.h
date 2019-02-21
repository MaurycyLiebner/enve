#ifndef NODEPOINT_H
#define NODEPOINT_H
#include "movablepoint.h"
#include "nodepointvalues.h"
class UndoRedoStack;
struct NodeSettings;
class VectorPath;
class SkCanvas;
class CtrlPoint;

class VectorPathAnimator;

enum CanvasMode : short;
enum CtrlsMode : short;
class VectorPathEdge;

class NodePoint : public NonAnimatedMovablePoint {
    friend class StdSelfRef;
public:
    void applyTransform(const QMatrix &transform);

    void startTransform();
    void finishTransform();

    void setRelativePos(const QPointF &relPos);

    //void moveByRel(const QPointF &relTranslation);

    QPointF getStartCtrlPtAbsPos() const;
    QPointF getStartCtrlPtValue() const;
    CtrlPoint *getStartCtrlPt();

    QPointF getEndCtrlPtAbsPos();
    QPointF getEndCtrlPtValue() const;
    CtrlPoint *getEndCtrlPt();

    void drawNodePoint(SkCanvas *canvas,
                const CanvasMode &mode,
                const SkScalar &invScale,
                const bool &keyOnCurrent);

    NodePoint *getNextPoint();
    NodePoint *getPreviousPoint();

    bool isEndPoint();

    void setPointAsPrevious(NodePoint *pointToSet);
    void setPointAsNext(NodePoint *pointToSet);
    void setNextPoint(NodePoint *mNextPoint);
    void setPreviousPoint(NodePoint *mPreviousPoint);

    bool hasNextPoint();
    bool hasPreviousPoint();

    NodePoint *addPointRelPos(const QPointF &relPos);

    void connectToPoint(NodePoint *point);
    void disconnectFromPoint(NodePoint *point);

    void removeFromVectorPath();
    void removeApproximate();

    MovablePoint *getPointAtAbsPos(const QPointF &absPos,
                                   const CanvasMode &canvasMode,
                                   const qreal &canvasScaleInv);
    void rectPointsSelection(const QRectF &absRect,
                             QList<stdptr<MovablePoint>> &list);
    void updateStartCtrlPtVisibility();
    void updateEndCtrlPtVisibility();

    void setSeparateNodePoint(const bool &separateNodePoint);
    bool isSeparateNodePoint();

    void setCtrlsMode(const CtrlsMode &mode);
    QPointF symmetricToAbsPos(const QPointF &absPosToMirror);
    QPointF symmetricToAbsPosNewLen(const QPointF &absPosToMirror,
                                    const qreal &newLen);
    void ctrlPointPosChanged(const bool &startPtChanged);
    void moveEndCtrlPtToAbsPos(const QPointF &endCtrlPt);
    void moveStartCtrlPtToAbsPos(const QPointF &startCtrlPt);
    void moveEndCtrlPtToRelPos(const QPointF &endCtrlPt);
    void moveStartCtrlPtToRelPos(const QPointF &startCtrlPt);
    void setCtrlPtEnabled(const bool &enabled,
                          const bool &isStartPt);
    VectorPathAnimator *getParentPath();

    void cancelTransform();

    void setEndCtrlPtEnabled(const bool &enabled);
    void setStartCtrlPtEnabled(const bool &enabled);

    void resetEndCtrlPt();
    void resetStartCtrlPt();

    bool isEndCtrlPtEnabled();
    bool isStartCtrlPtEnabled();

    void setNodeId(const int &idT);
    const int &getNodeId();
    int getPtId() {
        return getNodeId()*3 + 1;
    }

    CtrlsMode getCurrentCtrlsMode();

    NodePointValues getPointValues() const;

    bool isNeighbourSelected();
    //void moveByAbs(const QPointF &absTranslatione);

    NodePoint *getConnectedSeparateNodePoint();

    void saveTransformPivotAbsPos(const QPointF &absPivot);
    void rotateRelativeToSavedPivot(const qreal &rot);
    void scaleRelativeToSavedPivot(const qreal &sx,
                                   const qreal &sy);

    VectorPathEdge *getNextEdge() {
        return mNextEdge.get();
    }

    NodePoint *addPointAbsPos(const QPointF &absPos);

    void setCurrentNodeSettings(NodeSettings *settings) {
        mCurrentNodeSettings = settings;
    }
    void setElementsPos(const QPointF &startPos,
                        const QPointF &targetPos,
                        const QPointF &endPos);
protected:
    NodePoint(VectorPathAnimator *parentAnimator,
              BasicTransformAnimator* parentTransform);
private:
    bool mSeparateNodePoint = false;
    int mNodeId;

    stdsptr<VectorPathEdge> mNextEdge;
    NodeSettings *mCurrentNodeSettings = nullptr;
    const qptr<VectorPathAnimator> mParentPath;

    NodePoint *mNextPoint = nullptr;
    NodePoint *mPreviousPoint = nullptr;
    stdsptr<CtrlPoint> mStartCtrlPt;
    stdsptr<CtrlPoint> mEndCtrlPt;

    void ctrlPointPosChanged(CtrlPoint *pointChanged,
                             CtrlPoint *pointToUpdate);
};

#endif // NODEPOINT_H
