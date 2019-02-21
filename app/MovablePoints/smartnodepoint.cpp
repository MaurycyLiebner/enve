#include "smartnodepoint.h"
#include "undoredo.h"
#include "ctrlpoint.h"
#include <QPainter>
#include <QDebug>
#include "edge.h"
#include "Animators/pathanimator.h"
#include "canvas.h"
#include "pointhelpers.h"
#include "Animators/SmartPath/smartpathanimator.h"
#include "GUI/mainwindow.h"
#include "global.h"

SmartNodePoint::SmartNodePoint(SmartPathAnimator *parentAnimator,
                               BasicTransformAnimator *parentTransform) :
    NonAnimatedMovablePoint(parentTransform, TYPE_PATH_POINT, 6.5),
    mParentPath(parentAnimator) {
    mStartCtrlPt = SPtrCreate(CtrlPoint)(this, true);
    mEndCtrlPt = SPtrCreate(CtrlPoint)(this, false);

    mStartCtrlPt->setOtherCtrlPt(mEndCtrlPt.get());
    mEndCtrlPt->setOtherCtrlPt(mStartCtrlPt.get());

    mStartCtrlPt->hide();
    mEndCtrlPt->hide();

    //SmartNodePointUpdater *updater = SPtrCreate(SmartNodePointUpdater)(vectorPath);
    //prp_setUpdater(updater);
}

void SmartNodePoint::applyTransform(const QMatrix &transform) {
    mStartCtrlPt->applyTransform(transform);
    mEndCtrlPt->applyTransform(transform);
    NonAnimatedMovablePoint::applyTransform(transform);
}

void SmartNodePoint::startTransform() {
    NonAnimatedMovablePoint::startTransform();
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->NonAnimatedMovablePoint::startTransform();
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->NonAnimatedMovablePoint::startTransform();
    }
    getParentPath()->startPathChange();
}

void SmartNodePoint::saveTransformPivotAbsPos(const QPointF &absPivot) {
    NonAnimatedMovablePoint::saveTransformPivotAbsPos(absPivot);
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->saveTransformPivotAbsPos(absPivot);
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->saveTransformPivotAbsPos(absPivot);
    }
}

void SmartNodePoint::rotateRelativeToSavedPivot(const qreal &rot) {
    NonAnimatedMovablePoint::rotateRelativeToSavedPivot(rot);
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->rotate(rot);
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->rotate(rot);
    }
}

void SmartNodePoint::scaleRelativeToSavedPivot(const qreal &sx,
                                          const qreal &sy) {
    NonAnimatedMovablePoint::scaleRelativeToSavedPivot(sx, sy);
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->scale(sx, sy);
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->scale(sx, sy);
    }
}

void SmartNodePoint::cancelTransform() {
    NonAnimatedMovablePoint::cancelTransform();
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->NonAnimatedMovablePoint::cancelTransform();
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->NonAnimatedMovablePoint::cancelTransform();
    }
    getParentPath()->cancelPathChange();
}

void SmartNodePoint::finishTransform() {
    NonAnimatedMovablePoint::finishTransform();
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->NonAnimatedMovablePoint::finishTransform();
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->NonAnimatedMovablePoint::finishTransform();
    }
    mParentPath->finishPathChange();
}

void SmartNodePoint::setRelativePos(const QPointF &relPos) {
    setRelativePosVal(relPos);
    mTargetPath_d->actionSetNormalNodeP1(mNodeId, relPos);
}

void SmartNodePoint::connectToPoint(SmartNodePoint *point) {
    if(!point) return;
    if(!hasNextPoint()) {
        setPointAsNext(point);
    } else if(!hasPreviousPoint()) {
        setPointAsPrevious(point);
    }
}

void SmartNodePoint::disconnectFromPoint(SmartNodePoint *point) {
    if(!point) return;
    if(point == mNextPoint) {
        setPointAsNext(nullptr);
    } else if(point == mPreviousPoint) {
        setPointAsPrevious(nullptr);
    }
}

void SmartNodePoint::removeFromVectorPath() {
    mTargetPath_d->actionRemoveNormalNode(mNodeId);
}

void SmartNodePoint::removeApproximate() {
    mTargetPath_d->actionRemoveNormalNode(mNodeId);
}

void SmartNodePoint::rectPointsSelection(const QRectF &absRect,
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

MovablePoint *SmartNodePoint::getPointAtAbsPos(const QPointF &absPos,
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

QPointF SmartNodePoint::symmetricToAbsPos(const QPointF &absPosToMirror) {
    return symmetricToPos(absPosToMirror, getAbsolutePos());
}

QPointF SmartNodePoint::symmetricToAbsPosNewLen(const QPointF &absPosToMirror,
                                           const qreal &newLen) {
    return symmetricToPosNewLen(absPosToMirror, getAbsolutePos(), newLen);
}

void SmartNodePoint::moveStartCtrlPtToAbsPos(const QPointF &startCtrlPt) {
    moveStartCtrlPtToRelPos(mapAbsoluteToRelative(startCtrlPt));
}

void SmartNodePoint::moveEndCtrlPtToAbsPos(const QPointF &endCtrlPt) {
    moveEndCtrlPtToRelPos(mapAbsoluteToRelative(endCtrlPt));
}

void SmartNodePoint::moveEndCtrlPtToRelPos(const QPointF &endCtrlPt) {
    if(!getC2Enabled()) setEndCtrlPtEnabled(true);
    mTargetPath_d->actionSetNormalNodeC2(mNodeId, endCtrlPt);
    mEndCtrlPt->setRelativePos(endCtrlPt);
    ctrlPointPosChanged(mEndCtrlPt.get(), mStartCtrlPt.get());
}

void SmartNodePoint::moveStartCtrlPtToRelPos(const QPointF &startCtrlPt) {
    if(!getC0Enabled()) setStartCtrlPtEnabled(true);
    mTargetPath_d->actionSetNormalNodeC0(mNodeId, startCtrlPt);
    mStartCtrlPt->setRelativePos(startCtrlPt);
    ctrlPointPosChanged(mStartCtrlPt.get(), mEndCtrlPt.get());
}

QPointF SmartNodePoint::getStartCtrlPtAbsPos() const {
    return mapRelativeToAbsolute(getStartCtrlPtValue());
}

QPointF SmartNodePoint::getStartCtrlPtValue() const {
    if(getC0Enabled()) {
        return mStartCtrlPt->getRelativePos();
    } else {
        return getRelativePos();
    }
}

CtrlPoint *SmartNodePoint::getStartCtrlPt() {
    return mStartCtrlPt.get();
}

void SmartNodePoint::ctrlPointPosChanged(const bool &startPtChanged) {
    if(startPtChanged) {
        ctrlPointPosChanged(mStartCtrlPt.get(), mEndCtrlPt.get());
    } else {
        ctrlPointPosChanged(mEndCtrlPt.get(), mStartCtrlPt.get());
    }
}

void SmartNodePoint::ctrlPointPosChanged(CtrlPoint *pointChanged,
                                         CtrlPoint *pointToUpdate) {
    QPointF changedPointPos = pointChanged->getAbsolutePos();
    if(getCtrlsMode() == CtrlsMode::CTRLS_SYMMETRIC) {
        pointToUpdate->moveToAbsWithoutUpdatingTheOther(
                    symmetricToAbsPos(changedPointPos));
    } else if(getCtrlsMode() == CtrlsMode::CTRLS_SMOOTH) {
        if(!isPointZero(changedPointPos) ) {
            pointToUpdate->moveToAbsWithoutUpdatingTheOther(
                        symmetricToAbsPosNewLen(
                            changedPointPos,
                            pointToLen(pointToUpdate->getAbsolutePos() -
                                       getAbsolutePos())) );
        }
    }
}

QPointF SmartNodePoint::getEndCtrlPtAbsPos() {
    return mapRelativeToAbsolute(getEndCtrlPtValue());
}

QPointF SmartNodePoint::getEndCtrlPtValue() const {
    if(mNode_d->getC2Enabled()) {
        return mEndCtrlPt->getRelativePos();
    } else {
        return getRelativePos();
    }
}

CtrlPoint *SmartNodePoint::getEndCtrlPt() {
    return mEndCtrlPt.get();
}

void SmartNodePoint::drawNodePoint(SkCanvas *canvas,
                     const CanvasMode &mode,
                     const SkScalar &invScale,
                     const bool &keyOnCurrent) {
    canvas->save();
    SkPoint absPos = qPointToSk(getAbsolutePos());
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

SmartNodePoint* SmartNodePoint::getNextPoint() {
    return mNextPoint;
}

SmartNodePoint *SmartNodePoint::getPreviousPoint() {
    return mPreviousPoint;
}

SmartNodePoint *SmartNodePoint::getConnectedSeparateNodePoint() {
    if(isSeparateNodePoint() || !mPreviousPoint) return this;
    return mPreviousPoint->getConnectedSeparateNodePoint();
}

void SmartNodePoint::setNextPoint(SmartNodePoint *nextPoint) {
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

void SmartNodePoint::updateStartCtrlPtVisibility() {
    if(!mPreviousPoint) mStartCtrlPt->hide();
    else mStartCtrlPt->setVisible(mNode_d->getC0Enabled());
}

void SmartNodePoint::updateEndCtrlPtVisibility() {
    if(!mNextPoint) mEndCtrlPt->hide();
    else mEndCtrlPt->setVisible(mNode_d->getC2Enabled());
}

void SmartNodePoint::setEndCtrlPtEnabled(const bool &enabled) {
    if(enabled == getC2Enabled()) return;
    if(getC2Enabled()) setCtrlsMode(CtrlsMode::CTRLS_CORNER);
    mTargetPath_d->actionSetNormalNodeC2Enabled(mNodeId, enabled);
    //mParentPath->schedulePathUpdate();
    mParentPath->prp_updateInfluenceRangeAfterChanged();
    updateEndCtrlPtVisibility();
}

void SmartNodePoint::setStartCtrlPtEnabled(const bool &enabled) {
    if(enabled == getC0Enabled()) return;
    if(mNode_d->getC0Enabled()) setCtrlsMode(CtrlsMode::CTRLS_CORNER);
    mTargetPath_d->actionSetNormalNodeC0Enabled(mNodeId, enabled);
    //mParentPath->schedulePathUpdate();
    mParentPath->prp_updateInfluenceRangeAfterChanged();
    updateStartCtrlPtVisibility();
}

void SmartNodePoint::resetEndCtrlPt() {
    mEndCtrlPt->setRelativePosStartAndFinish(getRelativePos());
}

void SmartNodePoint::resetStartCtrlPt() {
    mStartCtrlPt->setRelativePosStartAndFinish(getRelativePos());
}

void SmartNodePoint::setNodeId(const int &idT) {
    mNodeId = idT;
    updateNode();
}

const int &SmartNodePoint::getNodeId() {
    return mNodeId;
}

NodePointValues SmartNodePoint::getPointValues() const {
    return NodePointValues(getStartCtrlPtValue(),
                           getRelativePos(),
                           getEndCtrlPtValue());
}

void SmartNodePoint::setCtrlPtEnabled(const bool &enabled,
                                 const bool &isStartPt) {
    if(isStartPt) {
        if(getC0Enabled() == enabled) return;
        setStartCtrlPtEnabled(enabled);
    } else {
        if(getC2Enabled() == enabled) return;
        setEndCtrlPtEnabled(enabled);
    }
}

bool SmartNodePoint::isNeighbourSelected() {
    const bool nextSelected = mNextPoint ?
                mNextPoint->isSelected() : false;
    const bool prevSelected = mPreviousPoint ?
                mPreviousPoint->isSelected() : false;
    return isSelected() || nextSelected || prevSelected;
}

SmartPathAnimator *SmartNodePoint::getParentPath() {
    return mParentPath;
}

void SmartNodePoint::setSeparateNodePoint(const bool &SeparateNodePoint) {
    mSeparateNodePoint = SeparateNodePoint;
}

bool SmartNodePoint::isSeparateNodePoint() {
    return mSeparateNodePoint;
}

void SmartNodePoint::setCtrlsMode(const CtrlsMode &mode) {
    mTargetPath_d->actionSetNormalNodeCtrlsMode(mNodeId, mode);
    if(mode == CtrlsMode::CTRLS_SYMMETRIC) {
        QPointF newStartPos;
        QPointF newEndPos;
        gGetCtrlsSymmetricPos(mEndCtrlPt->getRelativePos(),
                              mStartCtrlPt->getRelativePos(),
                              getRelativePos(),
                              newEndPos,
                              newStartPos);
        mStartCtrlPt->setRelativePosStartAndFinish(newStartPos);
        mEndCtrlPt->setRelativePosStartAndFinish(newEndPos);
    } else if(mode == CtrlsMode::CTRLS_SMOOTH) {
        QPointF newStartPos;
        QPointF newEndPos;
        gGetCtrlsSmoothPos(mEndCtrlPt->getRelativePos(),
                           mStartCtrlPt->getRelativePos(),
                           getRelativePos(),
                           newEndPos,
                           newStartPos);
        mStartCtrlPt->setRelativePosStartAndFinish(newStartPos);
    } else return;
    setCtrlPtEnabled(true, true);
    setCtrlPtEnabled(true, false);

    //mParentPath->schedulePathUpdate();
}

void SmartNodePoint::setPreviousPoint(SmartNodePoint *previousPoint) {
    mPreviousPoint = previousPoint;
    updateStartCtrlPtVisibility();
    //mParentPath->schedulePathUpdate();
}

bool SmartNodePoint::hasNextPoint() {
    return mNextPoint != nullptr;
}

bool SmartNodePoint::hasPreviousPoint() {
    return mPreviousPoint != nullptr;
}

void SmartNodePoint::setPointAsNext(SmartNodePoint *pointToSet) {
    if(hasNextPoint()) mNextPoint->setPreviousPoint(nullptr);
    setNextPoint(pointToSet);
    if(pointToSet) pointToSet->setPreviousPoint(this);
}

void SmartNodePoint::setPointAsPrevious(SmartNodePoint *pointToSet) {
    if(hasPreviousPoint()) mPreviousPoint->setNextPoint(nullptr);
    setPreviousPoint(pointToSet);
    if(pointToSet) pointToSet->setNextPoint(this);
}

SmartNodePoint *SmartNodePoint::addPointRelPos(const QPointF &relPos) {
    mTargetPath_d->actionInsertNodeBetween(mNodeId, mNode_d->getNextNodeId(),
                                           relPos, relPos, relPos);
    const auto newPt = mParentPath->createNewNodePoint(mNode_d->getNextNodeId());
    if(mNodeId == 0 && hasNextPoint()) setPointAsPrevious(newPt);
    else setPointAsNext(newPt);
    return newPt;
}

SmartNodePoint *SmartNodePoint::addPointAbsPos(const QPointF &absPos) {
    return addPointRelPos(mapAbsoluteToRelative(absPos));
}

void SmartNodePoint::setElementsPos(const QPointF &startPos,
                               const QPointF &targetPos,
                               const QPointF &endPos) {
    setRelativePosVal(targetPos);
    mStartCtrlPt->setRelativePosVal(startPos + targetPos);
    mEndCtrlPt->setRelativePosVal(endPos + targetPos);
}

bool SmartNodePoint::isEndPoint() {
    return mNextPoint == nullptr || mPreviousPoint == nullptr;
}
