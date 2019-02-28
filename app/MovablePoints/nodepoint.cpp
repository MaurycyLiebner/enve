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
#include "GUI/mainwindow.h"
#include "global.h"

NodePoint::NodePoint(VectorPathAnimator *parentAnimator,
                     BasicTransformAnimator *parentTransform) :
    NonAnimatedMovablePoint(parentTransform, TYPE_PATH_POINT, 6.5),
    mParentPath(parentAnimator) {
    mStartCtrlPt = SPtrCreate(CtrlPoint)(this, true);
    mEndCtrlPt = SPtrCreate(CtrlPoint)(this, false);

    mStartCtrlPt->setOtherCtrlPt(mEndCtrlPt.get());
    mEndCtrlPt->setOtherCtrlPt(mStartCtrlPt.get());

    mStartCtrlPt->hide();
    mEndCtrlPt->hide();

    //NodePointUpdater *updater = SPtrCreate(NodePointUpdater)(vectorPath);
    //prp_setUpdater(updater);
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
    mParentPath->finishedPathChange();
    mParentPath->setElementPos(getPtId(),  qPointToSk(getRelativePos()));
}

void NodePoint::setRelativePos(const QPointF &relPos) {
    setRelativePosVal(relPos);
    mParentPath->setElementPos(getPtId(), qPointToSk(relPos));
}

void NodePoint::connectToPoint(NodePoint *point) {
    if(!point) return;
    if(!hasNextPoint()) {
        setPointAsNext(point);
    } else if(!hasPreviousPoint()) {
        setPointAsPrevious(point);
    }
}

void NodePoint::disconnectFromPoint(NodePoint *point) {
    if(!point) return;
    if(point == mNextPoint) {
        setPointAsNext(nullptr);
    } else if(point == mPreviousPoint) {
        setPointAsPrevious(nullptr);
    }
}

void NodePoint::removeFromVectorPath() {
    mParentPath->removeNodeAt(mNodeId);
}

void NodePoint::removeApproximate() {
    mParentPath->removeNodeAtAndApproximate(mNodeId);
}

void NodePoint::rectPointsSelection(const QRectF &absRect,
                                    QList<stdptr<MovablePoint>> &list) {
    if(!isSelected()) {
        if(isContainedInRect(absRect)) {
            select();
            list.append(this);
        }
    }
    if(!mEndCtrlPt->isSelected()) {
        if(mEndCtrlPt->isContainedInRect(absRect)) {
            mEndCtrlPt->select();
            list.append(mEndCtrlPt.get());
        }
    }
    if(!mStartCtrlPt->isSelected()) {
        if(mStartCtrlPt->isContainedInRect(absRect)) {
            mStartCtrlPt->select();
            list.append(mStartCtrlPt.get());
        }
    }
}

MovablePoint *NodePoint::getPointAtAbsPos(const QPointF &absPos,
                                          const CanvasMode &canvasMode,
                                          const qreal &canvasScaleInv) {
    if(canvasMode == CanvasMode::MOVE_POINT) {
        if(mStartCtrlPt->isPointAtAbsPos(absPos, canvasScaleInv)) {
            return mStartCtrlPt.get();
        } else if(mEndCtrlPt->isPointAtAbsPos(absPos, canvasScaleInv)) {
            return mEndCtrlPt.get();
        }
    } else {
        if(!isEndPoint() || canvasMode != CanvasMode::ADD_POINT) {
            return nullptr;
        }
    }
    if(isPointAtAbsPos(absPos, canvasScaleInv)) {
        return this;
    }
    return nullptr;
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
    mParentPath->setElementPos(getPtId() + 1, qPointToSk(endCtrlPt));
    mEndCtrlPt->setRelativePos(endCtrlPt);
    ctrlPointPosChanged(mEndCtrlPt.get(), mStartCtrlPt.get());
}

void NodePoint::moveStartCtrlPtToRelPos(const QPointF &startCtrlPt) {
    if(!isStartCtrlPtEnabled()) {
        setStartCtrlPtEnabled(true);
    }
    mParentPath->setElementPos(getPtId() - 1, qPointToSk(startCtrlPt));
    mStartCtrlPt->setRelativePos(startCtrlPt);
    ctrlPointPosChanged(mStartCtrlPt.get(), mEndCtrlPt.get());
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

CtrlPoint *NodePoint::getStartCtrlPt() {
    return mStartCtrlPt.get();
}

void NodePoint::ctrlPointPosChanged(const bool &startPtChanged) {
    if(startPtChanged) {
        ctrlPointPosChanged(mStartCtrlPt.get(), mEndCtrlPt.get());
    } else {
        ctrlPointPosChanged(mEndCtrlPt.get(), mStartCtrlPt.get());
    }
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
    return mEndCtrlPt.get();
}

void NodePoint::drawNodePoint(SkCanvas *canvas,
                     const CanvasMode &mode,
                     const SkScalar &invScale,
                     const bool &keyOnCurrent) {
    canvas->save();
    SkPoint absPos = qPointToSk(getAbsolutePos());
    if(mSelected) {
        drawOnAbsPosSk(canvas, absPos, invScale,
                      0, 200, 255, keyOnCurrent);
    } else {
        drawOnAbsPosSk(canvas, absPos, invScale,
                       170, 240, 255, keyOnCurrent);
    }

    if((mode == CanvasMode::MOVE_POINT && isNeighbourSelected()) ||
       (mode == CanvasMode::ADD_POINT && mSelected)) {
        SkPaint paint;
        paint.setAntiAlias(true);
        if(mEndCtrlPt->isVisible() || mode == CanvasMode::ADD_POINT) {
            SkPoint endAbsPos = qPointToSk(
                        mEndCtrlPt->getAbsolutePos());
            paint.setColor(SK_ColorBLACK);
            paint.setStrokeWidth(1.5f*invScale);
            paint.setStyle(SkPaint::kStroke_Style);

            canvas->drawLine(absPos, endAbsPos, paint);
            paint.setColor(SK_ColorWHITE);
            paint.setStrokeWidth(0.75f*invScale);
            canvas->drawLine(absPos, endAbsPos, paint);
        }
        if(mStartCtrlPt->isVisible() || mode == CanvasMode::ADD_POINT) {
            SkPoint startAbsPos = qPointToSk(
                        mStartCtrlPt->getAbsolutePos());
            paint.setColor(SK_ColorBLACK);
            paint.setStrokeWidth(1.5f*invScale);
            paint.setStyle(SkPaint::kStroke_Style);
            canvas->drawLine(absPos, startAbsPos, paint);

            paint.setColor(SK_ColorWHITE);
            paint.setStrokeWidth(0.75f*invScale);
            canvas->drawLine(absPos, startAbsPos, paint);
        }
        mEndCtrlPt->drawSk(canvas, invScale);
        mStartCtrlPt->drawSk(canvas, invScale);
    }
    if(MainWindow::isCtrlPressed()) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(FONT_HEIGHT*invScale);
        SkRect bounds;
        ulong sizeT = static_cast<ulong>(QString::number(mNodeId).size());
        paint.measureText(QString::number(mNodeId).toStdString().c_str(),
                          sizeT*sizeof(char),
                          &bounds);
        paint.setColor(SK_ColorBLACK);
        SkFontStyle fontStyle = SkFontStyle(SkFontStyle::kBold_Weight,
                                            SkFontStyle::kNormal_Width,
                                            SkFontStyle::kUpright_Slant);
        sk_sp<SkTypeface> typeFace = SkTypeface::MakeFromName(nullptr, fontStyle);
        paint.setTypeface(typeFace);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawString(QString::number(mNodeId).toStdString().c_str(),
                           absPos.x() + bounds.width()*0.5f,
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
    if(isSeparateNodePoint() || !mPreviousPoint) return this;
    return mPreviousPoint->getConnectedSeparateNodePoint();
}

void NodePoint::setNextPoint(NodePoint *nextPoint) {
    mNextPoint = nextPoint;
    if(!mNextPoint) {
        if(mNextEdge) mNextEdge.reset();
    } else {
        if(!mNextEdge) {
            mNextEdge = SPtrCreate(VectorPathEdge)(this, mNextPoint);
        } else {
            mNextEdge->setPoint2(mNextPoint);
        }
    }
    updateEndCtrlPtVisibility();
    //mParentPath->schedulePathUpdate();
}

void NodePoint::updateStartCtrlPtVisibility() {
    if(!mPreviousPoint) {
        mStartCtrlPt->hide();
    } else {
        mStartCtrlPt->setVisible(mCurrentNodeSettings->startEnabled);
    }
}

void NodePoint::updateEndCtrlPtVisibility() {
    if(!mNextPoint) {
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
    mParentPath->schedulePathUpdate();
    mParentPath->prp_updateInfluenceRangeAfterChanged();
    updateEndCtrlPtVisibility();
}

void NodePoint::setStartCtrlPtEnabled(const bool &enabled) {
    if(enabled == mCurrentNodeSettings->startEnabled) return;
    if(mCurrentNodeSettings->startEnabled) {
        setCtrlsMode(CtrlsMode::CTRLS_CORNER);
    }
    mCurrentNodeSettings->startEnabled = enabled;
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
    return {getStartCtrlPtValue(),
            getRelativePos(),
            getEndCtrlPtValue()};
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
    bool nextSelected = (mNextPoint == nullptr) ?
                false : mNextPoint->isSelected();
    bool prevSelected = (mPreviousPoint == nullptr) ?
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
    mCurrentNodeSettings->ctrlsMode = mode;
    QPointF newStartPos;
    QPointF newEndPos;
    if(mode == CtrlsMode::CTRLS_SYMMETRIC) {
        gGetCtrlsSymmetricPos(mStartCtrlPt->getRelativePos(),
                              getRelativePos(),
                              mEndCtrlPt->getRelativePos(),
                              newEndPos,
                              newStartPos);
    } else if(mode == CtrlsMode::CTRLS_SMOOTH) {
        gGetCtrlsSmoothPos(mStartCtrlPt->getRelativePos(),
                           getRelativePos(),
                           mEndCtrlPt->getRelativePos(),
                           newEndPos,
                           newStartPos);
    } else return;
    mStartCtrlPt->setRelativePosStartAndFinish(newStartPos);
    mEndCtrlPt->setRelativePosStartAndFinish(newEndPos);
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
    return mNextPoint;
}

bool NodePoint::hasPreviousPoint() {
    return mPreviousPoint;
}

void NodePoint::setPointAsNext(NodePoint *pointToSet) {
    if(hasNextPoint()) {
        mNextPoint->setPreviousPoint(nullptr);
    }
    setNextPoint(pointToSet);
    if(pointToSet) {
        pointToSet->setPreviousPoint(this);
    }
}

void NodePoint::setPointAsPrevious(NodePoint *pointToSet) {
    if(hasPreviousPoint()) {
        mPreviousPoint->setNextPoint(nullptr);
    }
    setPreviousPoint(pointToSet);
    if(pointToSet) {
        pointToSet->setNextPoint(this);
    }
}

NodePoint *NodePoint::addPointRelPos(const QPointF &relPos) {
    const auto newPt = mParentPath->addNodeRelPos(relPos, this);
    if(mNodeId == 0 && hasNextPoint()) {
        setPointAsPrevious(newPt);
    } else {
        setPointAsNext(newPt);
    }
    return newPt;
}

NodePoint *NodePoint::addPointAbsPos(const QPointF &absPos) {
    const auto newPt = mParentPath->addNodeAbsPos(absPos, this);
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
    return mNextPoint == nullptr || mPreviousPoint == nullptr;
}
