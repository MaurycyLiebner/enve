#ifndef SMARTNODEPOINT_H
#define SMARTNODEPOINT_H
#include "movablepoint.h"
#include "nodepointvalues.h"
#include "segment.h"
#include "Animators/SmartPath/smartpathcontainer.h"
class UndoRedoStack;
struct NodeSettings;
class VectorPath;
class SkCanvas;
class SmartCtrlPoint;
class PathPointsHandler;
class SmartPathAnimator;

enum CanvasMode : short;
enum CtrlsMode : short;

class SmartNodePoint : public NonAnimatedMovablePoint {
    friend class StdSelfRef;
    friend class NormalSegment;
public:
    void applyTransform(const QMatrix &transform);

    void startTransform();
    void finishTransform();
    void cancelTransform();

    void setRelativePos(const QPointF &relPos);

    void removeFromVectorPath();
    void removeApproximate();

    SmartNodePoint *actionAddPointRelPos(const QPointF &relPos);
    SmartNodePoint* actionAddPointAbsPos(const QPointF &absPos);
    void actionConnectToNormalPoint(SmartNodePoint * const point);
    void actionDisconnectFromNormalPoint(SmartNodePoint * const point);

    //void moveByRel(const QPointF &relTranslation);

    QPointF getC0AbsPos() const;
    QPointF getC0Value() const;
    SmartCtrlPoint *getC0Pt();

    QPointF getC2AbsPos();
    QPointF getC2Value() const;
    SmartCtrlPoint *getC2Pt();

    void drawNodePoint(SkCanvas * const canvas,
                       const CanvasMode &mode,
                       const SkScalar &invScale,
                       const bool &keyOnCurrent);

    SmartNodePoint *getNextPoint();
    SmartNodePoint *getPreviousPoint();

    bool isEndPoint();

    MovablePoint *getPointAtAbsPos(const QPointF &absPos,
                                   const CanvasMode &canvasMode,
                                   const qreal &canvasScaleInv);
    void rectPointsSelection(const QRectF &absRect,
                             QList<stdptr<MovablePoint>> &list);
    void updateC0Visibility();
    void updateC2Visibility();

    void setSeparateNodePoint(const bool &separateNodePoint);
    bool isSeparateNodePoint();

    void setCtrlsMode(const CtrlsMode &mode);
    QPointF symmetricToAbsPos(const QPointF &absPosToMirror);
    QPointF symmetricToAbsPosNewLen(const QPointF &absPosToMirror,
                                    const qreal &newLen);
    void c0PtPosChanged();
    void c2PtPosChanged();

    void moveC2ToAbsPos(const QPointF &c2);
    void moveC0ToAbsPos(const QPointF &c0);
    void moveC2ToRelPos(const QPointF &c2);
    void moveC0ToRelPos(const QPointF &c0);

    SmartPathAnimator *getTargetAnimator() const;
    SmartPath *getTargetPath() const {
        return currentPath();
    }

    void setC0Enabled(const bool &enabled);
    void setC2Enabled(const bool &enabled);
    void resetC0();
    void resetC2();

    void setNodeId(const int &idT);
    const int &getNodeId();

    NodePointValues getPointValues() const;

    bool isPrevNormalSelected() const;
    bool isNextNormalSelected() const;

    bool isNeighbourNormalSelected() const;
    //void moveByAbs(const QPointF &absTranslatione);

    SmartNodePoint *getConnectedSeparateNodePoint();

    void saveTransformPivotAbsPos(const QPointF &absPivot);
    void rotateRelativeToSavedPivot(const qreal &rot);
    void scaleRelativeToSavedPivot(const qreal &sx,
                                   const qreal &sy);

    const NormalSegment& getNextNormalSegment() {
        return mNextNormalSegment;
    }

    void setElementsPos(const QPointF &c0,
                        const QPointF &p1,
                        const QPointF &c2);

    const qreal& getT() const {
        return mNode_d->fT;
    }

    const QPointF& getC0() const {
        return mNode_d->fC0;
    }

    const QPointF& getP1() const {
        return mNode_d->fP1;
    }

    const QPointF& getC2() const {
        return mNode_d->fC2;
    }

    const bool &getC0Enabled() const {
        return mNode_d->getC0Enabled();
    }

    const bool &getC2Enabled() const {
        return mNode_d->getC2Enabled();
    }

    const CtrlsMode& getCtrlsMode() const {
        return mNode_d->getCtrlsMode();
    }

    const Node::Type& getType() const {
        return mNode_d->getType();
    }

    void c0Moved(const QPointF& c0);

    void c2Moved(const QPointF& c2);

    void updateNode() {
        if(!currentPath()) mNode_d = nullptr;
        else mNode_d = currentPath()->getNodePtr(mNodeId);
        updateFromNodeData();
    }

    void updateNextSegmentDnD() {
        mNextNormalSegment.updateDnD();
    }
    void updateFromNodeData();
    void afterNextNodeC0P1Changed() {
        mNextNormalSegment.afterChanged();
    }

    bool hasNextPoint() const;
    bool hasPrevPoint() const;
    bool hasNextNormalPoint() const;
    bool hasPrevNormalPoint() const;
protected:
    SmartNodePoint(const int& nodeId,
                   PathPointsHandler * const handler,
                   SmartPathAnimator * const parentAnimator,
                   BasicTransformAnimator * const parentTransform);

    void setNextPoint(SmartNodePoint * const nextPoint);
    void setPrevPoint(SmartNodePoint * const prevPoint);
    void setPointAsPrev(SmartNodePoint * const pointToSet);
    void setPointAsNext(SmartNodePoint * const pointToSet);

    void setNextNormalPoint(SmartNodePoint * const nextPoint);
    void setPrevNormalPoint(SmartNodePoint * const prevPoint);
    void setPointAsNextNormal(SmartNodePoint * const pointToSet);
    void setPointAsPrevNormal(SmartNodePoint * const pointToSet);
private:
    SmartPath* currentPath() const;

    bool mSeparateNodePoint = false;
    int mNodeId;
    const Node * mNode_d = nullptr;

    const stdptr<PathPointsHandler> mHandler_k;
    const qptr<SmartPathAnimator> mParentAnimator;
    NormalSegment mNextNormalSegment;

    stdsptr<SmartCtrlPoint> mC0Pt;
    stdsptr<SmartCtrlPoint> mC2Pt;

    SmartNodePoint* mPrevNormalPoint = nullptr;
    SmartNodePoint* mNextNormalPoint = nullptr;

    SmartNodePoint* mPrevPoint = nullptr;
    SmartNodePoint* mNextPoint = nullptr;

    void ctrlPointPosChanged(const SmartCtrlPoint * const pointChanged,
                             SmartCtrlPoint * const pointToUpdate);
};

#endif // SMARTNODEPOINT_H
