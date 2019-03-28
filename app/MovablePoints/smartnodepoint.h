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

    void saveTransformPivotAbsPos(const QPointF &absPivot);
    void rotateRelativeToSavedPivot(const qreal &rot);
    void scaleRelativeToSavedPivot(const qreal &sx, const qreal &sy);

    void setRelativePos(const QPointF &relPos);

    void removeFromVectorPath();
    void removeApproximate();

    SmartNodePoint *actionAddPointRelPos(const QPointF &relPos);
    SmartNodePoint* actionAddPointAbsPos(const QPointF &absPos);
    bool actionConnectToNormalPoint(SmartNodePoint * const other);
    void actionDisconnectFromNormalPoint(SmartNodePoint * const other);

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
    void setOutdated() {
        mOutdated = true;
        setPrevPoint(nullptr);
        setNextPoint(nullptr);
        setPrevNormalPoint(nullptr);
        setNextNormalPoint(nullptr);
    }

    const bool& isOutdated() const {
        return mOutdated;
    }
    const int &getNodeId();

    NodePointValues getPointValues() const;

    bool isPrevNormalSelected() const;
    bool isNextNormalSelected() const;

    bool isNeighbourNormalSelected() const;
    //void moveByAbs(const QPointF &absTranslatione);

    SmartNodePoint *getConnectedSeparateNodePoint();

    NormalSegment getNextNormalSegment() {
        return mNextNormalSegment;
    }

    void setElementsPos(const QPointF &c0,
                        const QPointF &p1,
                        const QPointF &c2);

    qreal getT() const {
        return mNode_d->fT;
    }

    QPointF getC0() const {
        return mNode_d->getC0();
    }

    QPointF getP1() const {
        return mNode_d->fP1;
    }

    QPointF getC2() const {
        return mNode_d->getC2();
    }

    bool getC0Enabled() const {
        return mNode_d->getC0Enabled();
    }

    bool getC2Enabled() const {
        return mNode_d->getC2Enabled();
    }

    CtrlsMode getCtrlsMode() const {
        return mNode_d->getCtrlsMode();
    }

    Node::Type getType() const {
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
    void updateFromNodeDataPosOnly();

    void afterNextNodeC0P1Changed() {
        mNextNormalSegment.afterChanged();
    }

    bool hasNextPoint() const;
    bool hasPrevPoint() const;
    bool hasNextNormalPoint() const;
    bool hasPrevNormalPoint() const;
    void afterAllNodesUpdated() {
        updatePrevNormalNode();
        updateNextNormalNode();
    }

    bool isNormal() const {
        return getType() == Node::NORMAL;
    }

    bool isDissolved() const {
        return getType() == Node::DISSOLVED;
    }

    const Node* getTargetNode() const {
        return mNode_d;
    }

    const PathPointsHandler * getHandler();
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

    void updatePrevNormalNode();
    void updateNextNormalNode();
private:
    SmartPath* currentPath() const;

    bool mOutdated = false;
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
