//#ifndef SMARTNODEPOINT_H
//#define SMARTNODEPOINT_H
//#include "movablepoint.h"
//#include "nodepointvalues.h"
//#include "Animators/SmartPath/smartpathcontainer.h"
//class UndoRedoStack;
//struct NodeSettings;
//class VectorPath;
//class SkCanvas;
//class CtrlPoint;

//class SmartPathAnimator;

//enum CanvasMode : short;
//enum CtrlsMode : short;

//class VectorPathEdge;

//class SmartNodePoint : public NonAnimatedMovablePoint {
//    friend class StdSelfRef;
//public:
//    void applyTransform(const QMatrix &transform);

//    void startTransform();
//    void finishTransform();

//    void setRelativePos(const QPointF &relPos);

//    //void moveByRel(const QPointF &relTranslation);

//    QPointF getStartCtrlPtAbsPos() const;
//    QPointF getStartCtrlPtValue() const;
//    CtrlPoint *getStartCtrlPt();

//    QPointF getEndCtrlPtAbsPos();
//    QPointF getEndCtrlPtValue() const;
//    CtrlPoint *getEndCtrlPt();

//    void drawNodePoint(SkCanvas *canvas,
//                const CanvasMode &mode,
//                const SkScalar &invScale,
//                const bool &keyOnCurrent);

//    SmartNodePoint *getNextPoint();
//    SmartNodePoint *getPreviousPoint();

//    bool isEndPoint();

//    void setPointAsPrevious(SmartNodePoint *pointToSet);
//    void setPointAsNext(SmartNodePoint *pointToSet);
//    void setNextPoint(SmartNodePoint *mNextPoint);
//    void setPreviousPoint(SmartNodePoint *mPreviousPoint);

//    bool hasNextPoint();
//    bool hasPreviousPoint();

//    SmartNodePoint *addPointRelPos(const QPointF &relPos);

//    void connectToPoint(SmartNodePoint *point);
//    void disconnectFromPoint(SmartNodePoint *point);

//    void removeFromVectorPath();
//    void removeApproximate();

//    MovablePoint *getPointAtAbsPos(const QPointF &absPos,
//                                   const CanvasMode &canvasMode,
//                                   const qreal &canvasScaleInv);
//    void rectPointsSelection(const QRectF &absRect,
//                             QList<stdptr<MovablePoint>> &list);
//    void updateStartCtrlPtVisibility();
//    void updateEndCtrlPtVisibility();

//    void setSeparateNodePoint(const bool &separateNodePoint);
//    bool isSeparateNodePoint();

//    void setCtrlsMode(const CtrlsMode &mode);
//    QPointF symmetricToAbsPos(const QPointF &absPosToMirror);
//    QPointF symmetricToAbsPosNewLen(const QPointF &absPosToMirror,
//                                    const qreal &newLen);
//    void ctrlPointPosChanged(const bool &startPtChanged);
//    void moveEndCtrlPtToAbsPos(const QPointF &endCtrlPt);
//    void moveStartCtrlPtToAbsPos(const QPointF &startCtrlPt);
//    void moveEndCtrlPtToRelPos(const QPointF &endCtrlPt);
//    void moveStartCtrlPtToRelPos(const QPointF &startCtrlPt);
//    void setCtrlPtEnabled(const bool &enabled,
//                          const bool &isStartPt);
//    SmartPathAnimator *getParentPath();

//    void cancelTransform();

//    void setEndCtrlPtEnabled(const bool &enabled);
//    void setStartCtrlPtEnabled(const bool &enabled);

//    void resetEndCtrlPt();
//    void resetStartCtrlPt();

//    void setNodeId(const int &idT);
//    const int &getNodeId();
//    int getPtId() {
//        return getNodeId()*3 + 1;
//    }

//    NodePointValues getPointValues() const;

//    bool isNeighbourSelected();
//    //void moveByAbs(const QPointF &absTranslatione);

//    SmartNodePoint *getConnectedSeparateNodePoint();

//    void saveTransformPivotAbsPos(const QPointF &absPivot);
//    void rotateRelativeToSavedPivot(const qreal &rot);
//    void scaleRelativeToSavedPivot(const qreal &sx,
//                                   const qreal &sy);

//    VectorPathEdge *getNextEdge() {
//        return mNextEdge.get();
//    }

//    SmartNodePoint *addPointAbsPos(const QPointF &absPos);

//    void setElementsPos(const QPointF &startPos,
//                        const QPointF &targetPos,
//                        const QPointF &endPos);

//    const QPointF& getC0() const {
//        return mNode_d->fC0;
//    }

//    const QPointF& getP1() const {
//        return mNode_d->fP1;
//    }

//    const QPointF& getC2() const {
//        return mNode_d->fC2;
//    }

//    const bool &getC0Enabled() const {
//        return mNode_d->getC0Enabled();
//    }

//    const bool &getC2Enabled() const {
//        return mNode_d->getC2Enabled();
//    }

//    const CtrlsMode& getCtrlsMode() const {
//        return mNode_d->getCtrlsMode();
//    }

//    Node::Type getType() const {
//        return mNode_d->getType();
//    }

//    void updateNode() {
//        if(!mTargetPath_d) mNode_d = nullptr;
//        else mNode_d = mTargetPath_d->getNodePtr(mNodeId);
//    }
//protected:
//    SmartNodePoint(SmartPathAnimator *parentAnimator,
//                   BasicTransformAnimator* parentTransform);
//private:
//    bool mSeparateNodePoint = false;
//    int mNodeId;
//    const Node * mNode_d = nullptr;

//    SmartPath * mTargetPath_d = nullptr;
//    stdsptr<VectorPathEdge> mNextEdge;
//    const qptr<SmartPathAnimator> mParentPath;

//    stdptr<SmartNodePoint> mNextPoint;
//    stdptr<SmartNodePoint> mPreviousPoint;
//    stdsptr<CtrlPoint> mStartCtrlPt;
//    stdsptr<CtrlPoint> mEndCtrlPt;

//    void ctrlPointPosChanged(CtrlPoint *pointChanged,
//                             CtrlPoint *pointToUpdate);
//};

//#endif // SMARTNODEPOINT_H
