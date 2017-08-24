#include "nodepoint.h"
#include "Animators/PathAnimators/singlevectorpathanimator.h"
#include "undoredo.h"
#include "ctrlpoint.h"
#include <QPainter>
#include <QDebug>
#include "edge.h"
#include "Animators/pathanimator.h"
#include "canvas.h"
#include "pointhelpers.h"
#include "Animators/PathAnimators/vectorpathanimator.h"

NodePoint::NodePoint(VectorPathAnimator *parentAnimator) :
    NonAnimatedMovablePoint(parentAnimator->getParentPathAnimator()->getParentBox(),
                 MovablePointType::TYPE_PATH_POINT, 9.5) {
    mParentPath = parentAnimator;
    mStartCtrlPt = new CtrlPoint(this, true);
    mEndCtrlPt = new CtrlPoint(this, false);

    mStartCtrlPt->setOtherCtrlPt(mEndCtrlPt);
    mEndCtrlPt->setOtherCtrlPt(mStartCtrlPt);

    mStartCtrlPt->hide();
    mEndCtrlPt->hide();

    //NodePointUpdater *updater = new NodePointUpdater(vectorPath);
    //prp_setUpdater(updater);
}

void NodePoint::setParentPath(VectorPathAnimator *path) {
    mParentPath = path;
}

void NodePoint::applyTransform(const QMatrix &transform) {
    mStartCtrlPt->applyTransform(transform);
    mEndCtrlPt->applyTransform(transform);
    NonAnimatedMovablePoint::applyTransform(transform);
}

void NodePoint::startTransform() {
    NonAnimatedMovablePoint::startTransform();
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->NonAnimatedMovablePoint::startTransform();
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->NonAnimatedMovablePoint::startTransform();
    }
}

void NodePoint::saveTransformPivotAbsPos(const QPointF &absPivot) {
    NonAnimatedMovablePoint::saveTransformPivotAbsPos(absPivot);
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->saveTransformPivotAbsPos(absPivot);
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->saveTransformPivotAbsPos(absPivot);
    }
}

void NodePoint::rotateRelativeToSavedPivot(const qreal &rot) {
    NonAnimatedMovablePoint::rotateRelativeToSavedPivot(rot);
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->rotate(rot);
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->rotate(rot);
    }
}

void NodePoint::cancelTransform() {
    NonAnimatedMovablePoint::cancelTransform();
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->NonAnimatedMovablePoint::cancelTransform();
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->NonAnimatedMovablePoint::cancelTransform();
    }
}

void NodePoint::finishTransform() {
    NonAnimatedMovablePoint::finishTransform();
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->NonAnimatedMovablePoint::finishTransform();
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->NonAnimatedMovablePoint::finishTransform();
    }
}

void NodePoint::connectToPoint(NodePoint *point) {
    if(point == NULL) {
        return;
    }
    if(!hasNextPoint()) {
        setPointAsNext(point);
    } else if(!hasPreviousPoint()) {
        setPointAsPrevious(point);
    } else {
        return;
    }
}

void NodePoint::disconnectFromPoint(NodePoint *point) {
    if(point == NULL) {
        return;
    }
    if(point == mNextPoint) {
        setPointAsNext(NULL);
    } else if(point == mPreviousPoint) {
        setPointAsPrevious(NULL);
    } else {
        return;
    }
}

void NodePoint::removeFromVectorPath() {
    mParentPath->removeNodeAt(mPointId);
}

void NodePoint::removeApproximate() {
    mParentPath->removeNodeAtAndApproximate(mPointId);
}

void NodePoint::rectPointsSelection(const QRectF &absRect,
                                    QList<MovablePoint*> *list) {
    if(!isSelected()) {
        if(isContainedInRect(absRect)) {
            select();
            list->append(this);
        }
    }
    if(!mEndCtrlPt->isSelected()) {
        if(mEndCtrlPt->isContainedInRect(absRect)) {
            mEndCtrlPt->select();
            list->append(mEndCtrlPt);
        }
    }
    if(!mStartCtrlPt->isSelected()) {
        if(mStartCtrlPt->isContainedInRect(absRect)) {
            mStartCtrlPt->select();
            list->append(mStartCtrlPt);
        }
    }
}

MovablePoint *NodePoint::getPointAtAbsPos(const QPointF &absPos,
                                          const CanvasMode &canvasMode,
                                          const qreal &canvasScaleInv) {
    if(canvasMode == CanvasMode::MOVE_POINT) {
        if(mStartCtrlPt->isPointAtAbsPos(absPos,
                                         canvasScaleInv)) {
            return mStartCtrlPt;
        } else if (mEndCtrlPt->isPointAtAbsPos(absPos,
                                               canvasScaleInv)) {
            return mEndCtrlPt;
        }
    } else {
        if(!isEndPoint()) {
            return NULL;
        }
    }
    if (isPointAtAbsPos(absPos,
                        canvasScaleInv)) {
        return this;
    }
    return NULL;
}

QPointF NodePoint::symmetricToAbsPos(const QPointF &absPosToMirror) {
    return symmetricToPos(absPosToMirror, getAbsolutePos());
}

QPointF NodePoint::symmetricToAbsPosNewLen(const QPointF &absPosToMirror,
                                           const qreal &newLen) {
    return symmetricToPosNewLen(absPosToMirror, getAbsolutePos(), newLen);
}

void NodePoint::moveStartCtrlPtToAbsPos(const QPointF &startCtrlPt) {
    if(!isStartCtrlPtEnabled()) {
        setStartCtrlPtEnabled(true);
    }
    mStartCtrlPt->moveToAbs(startCtrlPt);
}

void NodePoint::moveEndCtrlPtToAbsPos(const QPointF &endCtrlPt) {
    if(!isEndCtrlPtEnabled()) {
        setEndCtrlPtEnabled(true);
    }
    mEndCtrlPt->moveToAbs(endCtrlPt);
}

void NodePoint::moveEndCtrlPtToRelPos(const QPointF &endCtrlPt) {
    if(!isEndCtrlPtEnabled()) {
        setEndCtrlPtEnabled(true);
    }
    mEndCtrlPt->setRelativePos(endCtrlPt);
}

void NodePoint::moveStartCtrlPtToRelPos(const QPointF &startCtrlPt) {
    if(!isStartCtrlPtEnabled()) {
        setStartCtrlPtEnabled(true);
    }
    mStartCtrlPt->setRelativePos(startCtrlPt);
}

QPointF NodePoint::getStartCtrlPtAbsPos() const {
    return mapRelativeToAbsolute(getStartCtrlPtValue());
}

QPointF NodePoint::getStartCtrlPtValue() const {
    if(mStartCtrlPtEnabled) {
        return mStartCtrlPt->getRelativePos();
    } else {
        return getRelativePos();
    }
}

CtrlPoint *NodePoint::getStartCtrlPt()
{
    return mStartCtrlPt;
}

void NodePoint::ctrlPointPosChanged(const bool &startPtChanged) {
    ctrlPointPosChanged((startPtChanged) ? mStartCtrlPt : mEndCtrlPt,
                        (startPtChanged) ? mEndCtrlPt : mStartCtrlPt);
}

void NodePoint::ctrlPointPosChanged(CtrlPoint *pointChanged,
                                    CtrlPoint *pointToUpdate) {
    QPointF changedPointPos = pointChanged->getAbsolutePos();
    if(mCurrentNodeSettings->ctrlsMode ==
            CtrlsMode::CTRLS_SYMMETRIC) {
        pointToUpdate->moveToAbsWithoutUpdatingTheOther(
                    symmetricToAbsPos(changedPointPos));
    } else if(mCurrentNodeSettings->ctrlsMode ==
              CtrlsMode::CTRLS_SMOOTH) {
        if(!isPointZero(changedPointPos) ) {
            pointToUpdate->moveToAbsWithoutUpdatingTheOther(
                        symmetricToAbsPosNewLen(
                            changedPointPos,
                            pointToLen(pointToUpdate->getAbsolutePos() -
                                       getAbsolutePos())) );
        }
    }
}

QPointF NodePoint::getEndCtrlPtAbsPos() {
    return mapRelativeToAbsolute(getEndCtrlPtValue());
}

QPointF NodePoint::getEndCtrlPtValue() const {
    if(mCurrentNodeSettings->endEnabled) {
        return mEndCtrlPt->getRelativePos();
    } else {
        return getRelativePos();
    }
}

CtrlPoint *NodePoint::getEndCtrlPt() {
    return mEndCtrlPt;
}

void NodePoint::drawSk(SkCanvas *canvas,
                     const CanvasMode &mode,
                     const SkScalar &invScale) {
    canvas->save();
    SkPoint absPos = QPointFToSkPoint(getAbsolutePos());
    if(mSelected) {
        drawOnAbsPosSk(canvas,
                     absPos,
                     invScale,
                     0, 200, 255);
    } else {
        drawOnAbsPosSk(canvas,
                     absPos,
                     invScale,
                     170, 240, 255);
    }

    if((mode == CanvasMode::MOVE_POINT &&
        (isNeighbourSelected() ) ) ||
            (mode == CanvasMode::ADD_POINT && mSelected)) {
        SkPaint paint;
        paint.setAntiAlias(true);
        if(mEndCtrlPt->isVisible() || mode == CanvasMode::ADD_POINT) {
            SkPoint endAbsPos = QPointFToSkPoint(
                        mEndCtrlPt->getAbsolutePos());
            paint.setColor(SK_ColorBLACK);
            paint.setStrokeWidth(1.5*invScale);
            paint.setStyle(SkPaint::kStroke_Style);

            canvas->drawLine(absPos, endAbsPos, paint);
            paint.setColor(SK_ColorWHITE);
            paint.setStrokeWidth(0.75*invScale);
            canvas->drawLine(absPos, endAbsPos, paint);
        }
        if(mStartCtrlPt->isVisible() || mode == CanvasMode::ADD_POINT) {
            SkPoint startAbsPos = QPointFToSkPoint(
                        mStartCtrlPt->getAbsolutePos());
            paint.setColor(SK_ColorBLACK);
            paint.setStrokeWidth(1.5*invScale);
            paint.setStyle(SkPaint::kStroke_Style);
            canvas->drawLine(absPos, startAbsPos, paint);

            paint.setColor(SK_ColorWHITE);
            paint.setStrokeWidth(0.75*invScale);
            canvas->drawLine(absPos, startAbsPos, paint);
        }
        mEndCtrlPt->drawSk(canvas, invScale);
        mStartCtrlPt->drawSk(canvas, invScale);
    }

    canvas->restore();
}

NodePoint* NodePoint::getNextPoint()
{
    return mNextPoint;
}

NodePoint *NodePoint::getPreviousPoint() {
    return mPreviousPoint;
}

NodePoint *NodePoint::getConnectedSeparateNodePoint() {
    if(isSeparateNodePoint() ||
       mPreviousPoint == NULL) return this;
    return mPreviousPoint->getConnectedSeparateNodePoint();
}

void NodePoint::setNextPoint(NodePoint *nextPoint) {
    mNextPoint = nextPoint;
    if(mNextPoint == NULL) {
        if(mNextEdge.get() != NULL) {
            mNextEdge.reset();
        }
    } else {
        if(mNextEdge.get() == NULL) {
            mNextEdge = (new VectorPathEdge(this, mNextPoint))->
                                ref<VectorPathEdge>();
        } else {
            mNextEdge->setPoint2(mNextPoint);
        }
    }
    updateEndCtrlPtVisibility();
    //mParentPath->schedulePathUpdate();
}

void NodePoint::updateStartCtrlPtVisibility() {
    if(mPreviousPoint == NULL) {
        mStartCtrlPt->hide();
    } else {
        mStartCtrlPt->setVisible(mStartCtrlPtEnabled);
    }
}

void NodePoint::updateEndCtrlPtVisibility() {
    if(mNextPoint == NULL) {
        mEndCtrlPt->hide();
    } else {
        mEndCtrlPt->setVisible(mCurrentNodeSettings->endEnabled);
    }
}

void NodePoint::setEndCtrlPtEnabled(const bool &enabled) {
    if(enabled == mCurrentNodeSettings->endEnabled) return;
    if(mCurrentNodeSettings->endEnabled) {
        setCtrlsMode(CtrlsMode::CTRLS_CORNER);
    }
    mCurrentNodeSettings->endEnabled = enabled;
    updateEndCtrlPtVisibility();
}

void NodePoint::setStartCtrlPtEnabled(const bool &enabled) {
    if(enabled == mCurrentNodeSettings->startEnabled) return;
    if(mCurrentNodeSettings->startEnabled) {
        setCtrlsMode(CtrlsMode::CTRLS_CORNER);
    }
    mCurrentNodeSettings->startEnabled = enabled;
    updateStartCtrlPtVisibility();
}

void NodePoint::resetEndCtrlPt() {
    mEndCtrlPt->setRelativePos(getRelativePos());
}

void NodePoint::resetStartCtrlPt() {
    mStartCtrlPt->setRelativePos(getRelativePos());
}

void NodePoint::setPointId(const int &idT) {
    mPointId = idT;
}

const int &NodePoint::getPointId() {
    return mPointId;
}

NodePointValues NodePoint::getPointValues() const {
    return NodePointValues(getStartCtrlPtValue(),
                           getRelativePos(),
                           getEndCtrlPtValue() );
}

CtrlsMode NodePoint::getCurrentCtrlsMode()
{
    return mCurrentNodeSettings->ctrlsMode;
}

bool NodePoint::isEndCtrlPtEnabled()
{
    return mEndCtrlPtEnabled;
}

bool NodePoint::isStartCtrlPtEnabled()
{
    return mStartCtrlPtEnabled;
}

void NodePoint::setCtrlPtEnabled(const bool &enabled,
                                 const bool &isStartPt) {
    if(isStartPt) {
        if(mStartCtrlPtEnabled == enabled) {
            return;
        }
        setStartCtrlPtEnabled(enabled);
    } else {
        if(mEndCtrlPtEnabled == enabled) {
            return;
        }
        setEndCtrlPtEnabled(enabled);
    }
}

bool NodePoint::isNeighbourSelected() {
    bool nextSelected = (mNextPoint == NULL) ?
                false : mNextPoint->isSelected();
    bool prevSelected = (mPreviousPoint == NULL) ?
                false : mPreviousPoint->isSelected();
    return isSelected() || nextSelected || prevSelected;
}

VectorPathAnimator *NodePoint::getParentPath() {
    return mParentPath;
}

void NodePoint::setSeparateNodePoint(const bool &separateNodePoint) {
    mSeparateNodePoint = separateNodePoint;
}

bool NodePoint::isSeparateNodePoint() {
    return mSeparateNodePoint;
}

void NodePoint::setCtrlsMode(const CtrlsMode &mode) {
    mCurrentNodeSettings->ctrlsMode = mode;
    if(mode == CtrlsMode::CTRLS_SYMMETRIC) {
        QPointF newStartPos;
        QPointF newEndPos;
        getCtrlsSymmetricPos(mEndCtrlPt->getRelativePos(),
                             mStartCtrlPt->getRelativePos(),
                             getRelativePos(),
                             &newEndPos,
                             &newStartPos);
        mStartCtrlPt->setRelativePos(newStartPos);
        mEndCtrlPt->setRelativePos(newEndPos);
    } else if(mode == CtrlsMode::CTRLS_SMOOTH) {
        QPointF newStartPos;
        QPointF newEndPos;
        getCtrlsSmoothPos(mEndCtrlPt->getRelativePos(),
                          mStartCtrlPt->getRelativePos(),
                          getRelativePos(),
                          &newEndPos,
                          &newStartPos);
        mStartCtrlPt->setRelativePos(newStartPos);
    } else {
        return;
    }
    setCtrlPtEnabled(true, true);
    setCtrlPtEnabled(true, false);

    //mParentPath->schedulePathUpdate();
}

void NodePoint::setPreviousPoint(NodePoint *previousPoint) {
    mPreviousPoint = previousPoint;
    updateStartCtrlPtVisibility();
    //mParentPath->schedulePathUpdate();
}

bool NodePoint::hasNextPoint() {
    return mNextPoint != NULL;
}

bool NodePoint::hasPreviousPoint() {
    return mPreviousPoint != NULL;
}

void NodePoint::setPointAsNext(NodePoint *pointToSet) {
    if(hasNextPoint()) {
        mNextPoint->setPreviousPoint(NULL);
    }
    setNextPoint(pointToSet);
    if(pointToSet != NULL) {
        pointToSet->setPreviousPoint(this);
    }
}

void NodePoint::setPointAsPrevious(NodePoint *pointToSet) {
    if(hasPreviousPoint()) {
        mPreviousPoint->setNextPoint(NULL);
    }
    setPreviousPoint(pointToSet);
    if(pointToSet != NULL) {
        pointToSet->setNextPoint(this);
    }
}

NodePoint *NodePoint::addPointRelPos(const QPointF &relPos) {
    int targetId;
    if(mPointId == 0) {
        targetId = 0;
    } else {
        targetId = mPointId + 1;
    }
    NodePoint *newPt =
            mParentPath->addNodeRelPos(relPos,
                                       this);
    if(mPointId == 0 && hasNextPoint()) {
        setPointAsPrevious(newPt);
    } else {
        setPointAsNext(newPt);
    }
    return newPt;
}

NodePoint *NodePoint::addPointAbsPos(const QPointF &absPos) {
    NodePoint *newPt =
            mParentPath->addNodeAbsPos(absPos,
                                       this);
    return newPt;
}

void NodePoint::setElementsPos(const QPointF &startPos,
                               const QPointF &targetPos,
                               const QPointF &endPos) {
    mStartCtrlPt->setRelativePos(startPos);
    setRelativePos(targetPos);
    mEndCtrlPt->setRelativePos(endPos);
}

bool NodePoint::isEndPoint() {
    return mNextPoint == NULL || mPreviousPoint == NULL;
}

NodePointValues operator-(const NodePointValues &ppv1,
                          const NodePointValues &ppv2) {
    return NodePointValues(ppv1.startRelPos - ppv2.startRelPos,
                           ppv1.pointRelPos - ppv2.pointRelPos,
                           ppv1.endRelPos - ppv2.endRelPos);
}

NodePointValues operator+(const NodePointValues &ppv1,
                          const NodePointValues &ppv2) {
    return NodePointValues(ppv1.startRelPos + ppv2.startRelPos,
                           ppv1.pointRelPos + ppv2.pointRelPos,
                           ppv1.endRelPos + ppv2.endRelPos);
}

NodePointValues operator/(const NodePointValues &ppv,
                          const qreal &val) {
    qreal invVal = 1.f/val;
    return NodePointValues(ppv.startRelPos * invVal,
                           ppv.pointRelPos * invVal,
                           ppv.endRelPos * invVal);
}

NodePointValues operator*(const qreal &val,
                          const NodePointValues &ppv) {
    return ppv*val;
}

NodePointValues operator*(const NodePointValues &ppv,
                          const qreal &val) {
    return NodePointValues(ppv.startRelPos * val,
                           ppv.pointRelPos * val,
                           ppv.endRelPos * val);
}
