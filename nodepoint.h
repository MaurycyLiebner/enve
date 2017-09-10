#ifndef NODEPOINT_H
#define NODEPOINT_H
#include "movablepoint.h"

class UndoRedoStack;
struct NodeSettings;
class VectorPath;
class SkCanvas;
class CtrlPoint;

class VectorPathAnimator;

enum CanvasMode : short;
enum CtrlsMode : short;

class NodePoint;
class VectorPathEdge;

struct NodePointValues {
    NodePointValues(const QPointF &startPosT,
                    const QPointF &pointPosT,
                    const QPointF &endPosT) {
        startRelPos = startPosT;
        pointRelPos = pointPosT;
        endRelPos = endPosT;
    }

    NodePointValues() {}    

    QPointF startRelPos;
    QPointF pointRelPos;
    QPointF endRelPos;

    NodePointValues &operator/=(const qreal &val) {
        qreal inv = 1.f/val;
        startRelPos *= inv;
        pointRelPos *= inv;
        endRelPos *= inv;
        return *this;
    }
    NodePointValues &operator*=(const qreal &val) {
        startRelPos *= val;
        pointRelPos *= val;
        endRelPos *= val;
        return *this;
    }
    NodePointValues &operator+=(const NodePointValues &ppv) {
        startRelPos += ppv.startRelPos;
        pointRelPos += ppv.pointRelPos;
        endRelPos += ppv.endRelPos;
        return *this;
    }
    NodePointValues &operator-=(const NodePointValues &ppv)
    {
        startRelPos -= ppv.startRelPos;
        pointRelPos -= ppv.pointRelPos;
        endRelPos -= ppv.endRelPos;
        return *this;
    }
};

NodePointValues operator+(const NodePointValues &ppv1, const NodePointValues &ppv2);
NodePointValues operator-(const NodePointValues &ppv1, const NodePointValues &ppv2);
NodePointValues operator/(const NodePointValues &ppv, const qreal &val);
NodePointValues operator*(const NodePointValues &ppv, const qreal &val);
NodePointValues operator*(const qreal &val, const NodePointValues &ppv);

class NodePoint : public NonAnimatedMovablePoint
{
public:
    NodePoint(VectorPathAnimator *parentAnimator);
    ~NodePoint();

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

    void drawSk(SkCanvas *canvas,
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
                             QList<MovablePoint *> *list);
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

    void setParentPath(VectorPathAnimator *path);

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
private:
    std::shared_ptr<VectorPathEdge> mNextEdge;

    NodeSettings *mCurrentNodeSettings = NULL;
    int mNodeId;

    VectorPathAnimator *mParentPath;

    bool mSeparateNodePoint = false;
    NodePoint *mNextPoint = NULL;
    NodePoint *mPreviousPoint = NULL;
    CtrlPoint *mStartCtrlPt = NULL;
    CtrlPoint *mEndCtrlPt = NULL;
    void ctrlPointPosChanged(CtrlPoint *pointChanged,
                             CtrlPoint *pointToUpdate);
};

#endif // NODEPOINT_H
