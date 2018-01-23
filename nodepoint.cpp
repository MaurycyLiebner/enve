#include "nodepoint.h"
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
    NonAnimatedMovablePoint(parentAnimator->getParentPathAnimator()->
                            getParentBox()->getTransformAnimator(),
                 MovablePointType::TYPE_PATH_POINT, 6.5) {
    mParentPath = parentAnimator;
    mStartCtrlPt = new CtrlPoint(this, true);
    mEndCtrlPt = new CtrlPoint(this, false);

    mStartCtrlPt->setOtherCtrlPt(mEndCtrlPt);
    mEndCtrlPt->setOtherCtrlPt(mStartCtrlPt);

    mStartCtrlPt->hide();
    mEndCtrlPt->hide();

    parentAnimator->appendToPointsList(this);
    //NodePointUpdater *updater = new NodePointUpdater(vectorPath);
    //prp_setUpdater(updater);
}

NodePoint::~NodePoint() {
    delete mStartCtrlPt;
    delete mEndCtrlPt;
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
    getParentPath()->startPathChange();
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

void NodePoint::scaleRelativeToSavedPivot(const qreal &sx,
                                          const qreal &sy) {
    NonAnimatedMovablePoint::scaleRelativeToSavedPivot(sx, sy);
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->scale(sx, sy);
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->scale(sx, sy);
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
    getParentPath()->cancelPathChange();
}

void NodePoint::finishTransform() {
    NonAnimatedMovablePoint::finishTransform();
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->NonAnimatedMovablePoint::finishTransform();
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->NonAnimatedMovablePoint::finishTransform();
    }
    getParentPath()->finishedPathChange();
    getParentPath()->setElementPos(getPtId(),
                                   QPointFToSkPoint(getRelativePos()));
}

void NodePoint::setRelativePos(const QPointF &relPos) {
    setRelativePosVal(relPos);
    getParentPath()->setElementPos(getPtId(), QPointFToSkPoint(relPos));
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
    mParentPath->removeNodeAt(mNodeId);
}

void NodePoint::removeApproximate() {
    mParentPath->removeNodeAtAndApproximate(mNodeId);
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
        if(!isEndPoint() || canvasMode != CanvasMode::ADD_POINT) {
            return NULL;
        }
    }
    if(isPointAtAbsPos(absPos,
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
    moveStartCtrlPtToRelPos(mapAbsoluteToRelative(startCtrlPt));
}

void NodePoint::moveEndCtrlPtToAbsPos(const QPointF &endCtrlPt) {
    moveEndCtrlPtToRelPos(mapAbsoluteToRelative(endCtrlPt));
}

void NodePoint::moveEndCtrlPtToRelPos(const QPointF &endCtrlPt) {
    if(!isEndCtrlPtEnabled()) {
        setEndCtrlPtEnabled(true);
    }
    mParentPath->setElementPos(getPtId() + 1, QPointFToSkPoint(endCtrlPt));
    mEndCtrlPt->setRelativePos(endCtrlPt);
    ctrlPointPosChanged(mEndCtrlPt, mStartCtrlPt);
}

void NodePoint::moveStartCtrlPtToRelPos(const QPointF &startCtrlPt) {
    if(!isStartCtrlPtEnabled()) {
        setStartCtrlPtEnabled(true);
    }
    mParentPath->setElementPos(getPtId() - 1, QPointFToSkPoint(startCtrlPt));
    mStartCtrlPt->setRelativePos(startCtrlPt);
    ctrlPointPosChanged(mStartCtrlPt, mEndCtrlPt);
}

QPointF NodePoint::getStartCtrlPtAbsPos() const {
    return mapRelativeToAbsolute(getStartCtrlPtValue());
}

QPointF NodePoint::getStartCtrlPtValue() const {
    if(mCurrentNodeSettings->startEnabled) {
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
#include "mainwindow.h"
#include "global.h"
void NodePoint::drawSk(SkCanvas *canvas,
                     const CanvasMode &mode,
                     const SkScalar &invScale,
                     const bool &keyOnCurrent) {
    canvas->save();
    SkPoint absPos = QPointFToSkPoint(getAbsolutePos());
    if(mSelected) {
        drawOnAbsPosSk(canvas,
                     absPos,
                     invScale,
                     0, 200, 255,
                     keyOnCurrent);
    } else {
        drawOnAbsPosSk(canvas,
                     absPos,
                     invScale,
                     170, 240, 255,
                     keyOnCurrent);
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
    if(MainWindow::isCtrlPressed()) {
        SkPaint paint;
        paint.setTextAlign(SkPaint::kCenter_Align);
        paint.setAntiAlias(true);
        paint.setTextSize(FONT_HEIGHT*invScale);
        SkRect bounds;
        paint.measureText(QString::number(mNodeId).toStdString().c_str(),
                          QString::number(mNodeId).size()*sizeof(char),
                          &bounds);
        paint.setColor(SK_ColorBLACK);
        paint.setTypeface(SkTypeface::MakeDefault(SkTypeface::kBold));
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawString(QString::number(mNodeId).toStdString().c_str(),
                           absPos.x(),
                           absPos.y() + bounds.height()*0.5f,
                           paint);
    }
    canvas->restore();
}

NodePoint* NodePoint::getNextPoint() {
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
        mStartCtrlPt->setVisible(mCurrentNodeSettings->startEnabled);
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
    NodeSettings newSettings = *mCurrentNodeSettings;
    if(newSettings.endEnabled) {
        setCtrlsMode(CtrlsMode::CTRLS_CORNER);
    }
    newSettings.endEnabled = enabled;
    mParentPath->replaceNodeSettingsForPtId(getPtId(),
                                            newSettings);
    mParentPath->schedulePathUpdate();
    mParentPath->prp_updateInfluenceRangeAfterChanged();
    updateEndCtrlPtVisibility();
}

void NodePoint::setStartCtrlPtEnabled(const bool &enabled) {
    if(enabled == mCurrentNodeSettings->startEnabled) return;
    NodeSettings newSettings = *mCurrentNodeSettings;
    if(newSettings.startEnabled) {
        setCtrlsMode(CtrlsMode::CTRLS_CORNER);
    }
    newSettings.startEnabled = enabled;
    mParentPath->replaceNodeSettingsForPtId(getPtId(),
                                            newSettings);
    mParentPath->schedulePathUpdate();
    mParentPath->prp_updateInfluenceRangeAfterChanged();
    updateStartCtrlPtVisibility();
}

void NodePoint::resetEndCtrlPt() {
    mEndCtrlPt->setRelativePosStartAndFinish(getRelativePos());
}

void NodePoint::resetStartCtrlPt() {
    mStartCtrlPt->setRelativePosStartAndFinish(getRelativePos());
}

void NodePoint::setNodeId(const int &idT) {
    mNodeId = idT;
}

const int &NodePoint::getNodeId() {
    return mNodeId;
}

NodePointValues NodePoint::getPointValues() const {
    return NodePointValues(getStartCtrlPtValue(),
                           getRelativePos(),
                           getEndCtrlPtValue() );
}

CtrlsMode NodePoint::getCurrentCtrlsMode() {
    return mCurrentNodeSettings->ctrlsMode;
}

bool NodePoint::isEndCtrlPtEnabled() {
    return mCurrentNodeSettings->endEnabled;
}

bool NodePoint::isStartCtrlPtEnabled() {
    return mCurrentNodeSettings->startEnabled;
}

void NodePoint::setCtrlPtEnabled(const bool &enabled,
                                 const bool &isStartPt) {
    if(isStartPt) {
        if(mCurrentNodeSettings->startEnabled == enabled) {
            return;
        }
        setStartCtrlPtEnabled(enabled);
    } else {
        if(mCurrentNodeSettings->endEnabled == enabled) {
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
    NodeSettings newSettings = *mCurrentNodeSettings;
    newSettings.ctrlsMode = mode;
    mParentPath->replaceNodeSettingsForPtId(getPtId(),
                                            newSettings);
    if(mode == CtrlsMode::CTRLS_SYMMETRIC) {
        QPointF newStartPos;
        QPointF newEndPos;
        getCtrlsSymmetricPos(mEndCtrlPt->getRelativePos(),
                             mStartCtrlPt->getRelativePos(),
                             getRelativePos(),
                             &newEndPos,
                             &newStartPos);
        mStartCtrlPt->setRelativePosStartAndFinish(newStartPos);
        mEndCtrlPt->setRelativePosStartAndFinish(newEndPos);
    } else if(mode == CtrlsMode::CTRLS_SMOOTH) {
        QPointF newStartPos;
        QPointF newEndPos;
        getCtrlsSmoothPos(mEndCtrlPt->getRelativePos(),
                          mStartCtrlPt->getRelativePos(),
                          getRelativePos(),
                          &newEndPos,
                          &newStartPos);
        mStartCtrlPt->setRelativePosStartAndFinish(newStartPos);
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
    NodePoint *newPt =
            mParentPath->addNodeRelPos(relPos,
                                       this);
    if(mNodeId == 0 && hasNextPoint()) {
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
    setRelativePosVal(targetPos);
    mStartCtrlPt->setRelativePosVal(startPos + targetPos);
    mEndCtrlPt->setRelativePosVal(endPos + targetPos);
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
