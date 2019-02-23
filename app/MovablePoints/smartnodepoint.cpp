#include "smartnodepoint.h"
#include "undoredo.h"
#include <QPainter>
#include <QDebug>
#include "Animators/pathanimator.h"
#include "canvas.h"
#include "pointhelpers.h"
#include "Animators/SmartPath/smartpathanimator.h"
#include "GUI/mainwindow.h"
#include "global.h"
#include "pathpointshandler.h"
#include "smartctrlpoint.h"

SmartNodePoint::SmartNodePoint(const int& nodeId,
                               PathPointsHandler * const handler,
                               SmartPathAnimator * const parentAnimator,
                               BasicTransformAnimator * const parentTransform) :
    NonAnimatedMovablePoint(parentTransform, TYPE_PATH_POINT, 6.5),
    mHandler_k(handler), mParentPath(parentAnimator) {
    mSegment.setPoint1(this);
    setNodeId(nodeId);
    mC0Pt = SPtrCreate(SmartCtrlPoint)(this, SmartCtrlPoint::C0);
    mC2Pt = SPtrCreate(SmartCtrlPoint)(this, SmartCtrlPoint::C2);

    mC0Pt->setOtherCtrlPt(mC2Pt.get());
    mC2Pt->setOtherCtrlPt(mC0Pt.get());

    mC0Pt->hide();
    mC2Pt->hide();
}

void SmartNodePoint::applyTransform(const QMatrix &transform) {
    mC0Pt->applyTransform(transform);
    mC2Pt->applyTransform(transform);
    NonAnimatedMovablePoint::applyTransform(transform);
}

void SmartNodePoint::startTransform() {
    NonAnimatedMovablePoint::startTransform();
    if(!mC0Pt->isSelected()) {
        mC0Pt->NonAnimatedMovablePoint::startTransform();
    }
    if(!mC2Pt->isSelected()) {
        mC2Pt->NonAnimatedMovablePoint::startTransform();
    }
    getTargetAnimator()->startPathChange();
}

void SmartNodePoint::saveTransformPivotAbsPos(const QPointF &absPivot) {
    NonAnimatedMovablePoint::saveTransformPivotAbsPos(absPivot);
    if(!mC0Pt->isSelected()) {
        mC0Pt->saveTransformPivotAbsPos(absPivot);
    }
    if(!mC2Pt->isSelected()) {
        mC2Pt->saveTransformPivotAbsPos(absPivot);
    }
}

void SmartNodePoint::rotateRelativeToSavedPivot(const qreal &rot) {
    NonAnimatedMovablePoint::rotateRelativeToSavedPivot(rot);
    if(!mC0Pt->isSelected()) {
        mC0Pt->rotate(rot);
    }
    if(!mC2Pt->isSelected()) {
        mC2Pt->rotate(rot);
    }
}

void SmartNodePoint::scaleRelativeToSavedPivot(const qreal &sx,
                                          const qreal &sy) {
    NonAnimatedMovablePoint::scaleRelativeToSavedPivot(sx, sy);
    if(!mC0Pt->isSelected()) {
        mC0Pt->scale(sx, sy);
    }
    if(!mC2Pt->isSelected()) {
        mC2Pt->scale(sx, sy);
    }
}

void SmartNodePoint::cancelTransform() {
    NonAnimatedMovablePoint::cancelTransform();
    if(!mC0Pt->isSelected()) {
        mC0Pt->NonAnimatedMovablePoint::cancelTransform();
    }
    if(!mC2Pt->isSelected()) {
        mC2Pt->NonAnimatedMovablePoint::cancelTransform();
    }
    getTargetAnimator()->cancelPathChange();
}

void SmartNodePoint::finishTransform() {
    NonAnimatedMovablePoint::finishTransform();
    if(!mC0Pt->isSelected()) {
        mC0Pt->NonAnimatedMovablePoint::finishTransform();
    }
    if(!mC2Pt->isSelected()) {
        mC2Pt->NonAnimatedMovablePoint::finishTransform();
    }
    mParentPath->finishPathChange();
}

void SmartNodePoint::setRelativePos(const QPointF &relPos) {
    setRelativePosVal(relPos);
    mTargetPath_d->actionSetNormalNodeP1(mNodeId, relPos);
}

void SmartNodePoint::connectToPoint(SmartNodePoint * const point) {
    if(!point) return;
    if(!hasNextPoint()) {
        setPointAsNext(point);
    } else if(!hasPreviousPoint()) {
        setPointAsPrevious(point);
    }
}

void SmartNodePoint::disconnectFromPoint(SmartNodePoint * const point) {
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

void SmartNodePoint::rectPointsSelection(
        const QRectF &absRect, QList<stdptr<MovablePoint>> &list) {
    if(!isSelected()) {
        if(isContainedInRect(absRect)) {
            select();
            list.append(this);
        }
    }
    if(!mC2Pt->isSelected()) {
        if(mC2Pt->isContainedInRect(absRect)) {
            mC2Pt->select();
            list.append(mC2Pt.get());
        }
    }
    if(!mC0Pt->isSelected()) {
        if(mC0Pt->isContainedInRect(absRect)) {
            mC0Pt->select();
            list.append(mC0Pt.get());
        }
    }
}

MovablePoint *SmartNodePoint::getPointAtAbsPos(const QPointF &absPos,
                                          const CanvasMode &canvasMode,
                                          const qreal &canvasScaleInv) {
    if(canvasMode == CanvasMode::MOVE_POINT) {
        if(mC0Pt->isPointAtAbsPos(absPos, canvasScaleInv)) {
            return mC0Pt.get();
        } else if(mC2Pt->isPointAtAbsPos(absPos, canvasScaleInv)) {
            return mC2Pt.get();
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

void SmartNodePoint::c0PtPosChanged() {
    ctrlPointPosChanged(mC0Pt.get(), mC2Pt.get());
}

void SmartNodePoint::c2PtPosChanged() {
    ctrlPointPosChanged(mC2Pt.get(), mC0Pt.get());
}

void SmartNodePoint::moveC0ToAbsPos(const QPointF &c0) {
    moveC0ToRelPos(mapAbsoluteToRelative(c0));
}

void SmartNodePoint::moveC2ToAbsPos(const QPointF &c2) {
    moveC2ToRelPos(mapAbsoluteToRelative(c2));
}

void SmartNodePoint::moveC2ToRelPos(const QPointF &c2) {
    if(!getC2Enabled()) setC2Enabled(true);
    mTargetPath_d->actionSetNormalNodeC2(mNodeId, c2);
    mC2Pt->setRelativePos(c2);
    ctrlPointPosChanged(mC2Pt.get(), mC0Pt.get());
}

void SmartNodePoint::moveC0ToRelPos(const QPointF &c0) {
    if(!getC0Enabled()) setC0Enabled(true);
    mTargetPath_d->actionSetNormalNodeC0(mNodeId, c0);
    mC0Pt->setRelativePos(c0);
    ctrlPointPosChanged(mC0Pt.get(), mC2Pt.get());
}

QPointF SmartNodePoint::getC0AbsPos() const {
    return mapRelativeToAbsolute(getC0Value());
}

QPointF SmartNodePoint::getC0Value() const {
    if(getC0Enabled()) {
        return mC0Pt->getRelativePos();
    } else {
        return getRelativePos();
    }
}

SmartCtrlPoint *SmartNodePoint::getC0Pt() {
    return mC0Pt.get();
}

void SmartNodePoint::ctrlPointPosChanged(
        const SmartCtrlPoint * const pointChanged,
        SmartCtrlPoint * const pointToUpdate) {
    const QPointF changedPointPos = pointChanged->getAbsolutePos();
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

QPointF SmartNodePoint::getC2AbsPos() {
    return mapRelativeToAbsolute(getC2Value());
}

QPointF SmartNodePoint::getC2Value() const {
    if(mNode_d->getC2Enabled()) {
        return mC2Pt->getRelativePos();
    } else {
        return getRelativePos();
    }
}

SmartCtrlPoint *SmartNodePoint::getC2Pt() {
    return mC2Pt.get();
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
        if(mC2Pt->isVisible() || mode == CanvasMode::ADD_POINT) {
            const SkPoint endAbsPos = qPointToSk(
                        mC2Pt->getAbsolutePos());
            paint.setColor(SK_ColorBLACK);
            paint.setStrokeWidth(1.5f*invScale);
            paint.setStyle(SkPaint::kStroke_Style);

            canvas->drawLine(absPos, endAbsPos, paint);
            paint.setColor(SK_ColorWHITE);
            paint.setStrokeWidth(0.75f*invScale);
            canvas->drawLine(absPos, endAbsPos, paint);
        }
        if(mC0Pt->isVisible() || mode == CanvasMode::ADD_POINT) {
            const SkPoint startAbsPos = qPointToSk(
                        mC0Pt->getAbsolutePos());
            paint.setColor(SK_ColorBLACK);
            paint.setStrokeWidth(1.5f*invScale);
            paint.setStyle(SkPaint::kStroke_Style);
            canvas->drawLine(absPos, startAbsPos, paint);

            paint.setColor(SK_ColorWHITE);
            paint.setStrokeWidth(0.75f*invScale);
            canvas->drawLine(absPos, startAbsPos, paint);
        }
        mC2Pt->drawSk(canvas, invScale);
        mC0Pt->drawSk(canvas, invScale);
    }
    if(MainWindow::isCtrlPressed()) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(FONT_HEIGHT*invScale);
        SkRect bounds;
        const ulong sizeT = static_cast<ulong>(
                    QString::number(mNodeId).size());
        paint.measureText(QString::number(mNodeId).toStdString().c_str(),
                          sizeT*sizeof(char),
                          &bounds);
        paint.setColor(SK_ColorBLACK);
        const SkFontStyle fontStyle =
                SkFontStyle(SkFontStyle::kBold_Weight,
                            SkFontStyle::kNormal_Width,
                            SkFontStyle::kUpright_Slant);
        const sk_sp<SkTypeface> typeFace =
                SkTypeface::MakeFromName(nullptr, fontStyle);
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
    mSegment.setPoint2(nextPoint);
    updatec2Visibility();
    //mParentPath->schedulePathUpdate();
}

void SmartNodePoint::updateC0PtVisibility() {
    if(!mPreviousPoint) mC0Pt->hide();
    else mC0Pt->setVisible(mNode_d->getC0Enabled());
}

void SmartNodePoint::updatec2Visibility() {
    if(!mNextPoint) mC2Pt->hide();
    else mC2Pt->setVisible(mNode_d->getC2Enabled());
}

void SmartNodePoint::setC2Enabled(const bool &enabled) {
    if(enabled == getC2Enabled()) return;
    if(getC2Enabled()) setCtrlsMode(CtrlsMode::CTRLS_CORNER);
    mTargetPath_d->actionSetNormalNodeC2Enabled(mNodeId, enabled);
    //mParentPath->schedulePathUpdate();
    mParentPath->prp_updateInfluenceRangeAfterChanged();
    updatec2Visibility();
}

void SmartNodePoint::setC0Enabled(const bool &enabled) {
    if(enabled == getC0Enabled()) return;
    if(mNode_d->getC0Enabled()) setCtrlsMode(CtrlsMode::CTRLS_CORNER);
    mTargetPath_d->actionSetNormalNodeC0Enabled(mNodeId, enabled);
    //mParentPath->schedulePathUpdate();
    mParentPath->prp_updateInfluenceRangeAfterChanged();
    updateC0PtVisibility();
}

void SmartNodePoint::resetC2() {
    mC2Pt->setRelativePosStartAndFinish(getRelativePos());
}

void SmartNodePoint::resetC0() {
    mC0Pt->setRelativePosStartAndFinish(getRelativePos());
}

void SmartNodePoint::setNodeId(const int &idT) {
    mNodeId = idT;
    updateNode();
}

const int &SmartNodePoint::getNodeId() {
    return mNodeId;
}

NodePointValues SmartNodePoint::getPointValues() const {
    return NodePointValues(getC0Value(),
                           getRelativePos(),
                           getC2Value());
}

bool SmartNodePoint::isNeighbourSelected() {
    const bool nextSelected = mNextPoint ?
                mNextPoint->isSelected() : false;
    const bool prevSelected = mPreviousPoint ?
                mPreviousPoint->isSelected() : false;
    return isSelected() || nextSelected || prevSelected;
}

SmartPathAnimator *SmartNodePoint::getTargetAnimator() const {
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
        gGetCtrlsSymmetricPos(mC2Pt->getRelativePos(),
                              mC0Pt->getRelativePos(),
                              getRelativePos(),
                              newEndPos,
                              newStartPos);
        mC0Pt->setRelativePosStartAndFinish(newStartPos);
        mC2Pt->setRelativePosStartAndFinish(newEndPos);
    } else if(mode == CtrlsMode::CTRLS_SMOOTH) {
        QPointF newStartPos;
        QPointF newEndPos;
        gGetCtrlsSmoothPos(mC2Pt->getRelativePos(),
                           mC0Pt->getRelativePos(),
                           getRelativePos(),
                           newEndPos,
                           newStartPos);
        mC0Pt->setRelativePosStartAndFinish(newStartPos);
    } else return;
    setC0Enabled(true);
    setC2Enabled(true);

    //mParentPath->schedulePathUpdate();
}

void SmartNodePoint::setPreviousPoint(SmartNodePoint *previousPoint) {
    mPreviousPoint = previousPoint;
    updateC0PtVisibility();
    //mParentPath->schedulePathUpdate();
}

bool SmartNodePoint::hasNextPoint() {
    return mNextPoint;
}

bool SmartNodePoint::hasPreviousPoint() {
    return mPreviousPoint;
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
    const int insertId = mTargetPath_d->actionInsertNodeBetween(
                mNodeId, mNode_d->getNextNodeId(), relPos, relPos, relPos);
    const auto newPt = mHandler_k->createNewNodePoint(insertId);
    if(hasNextPoint()) setPointAsPrevious(newPt);
    else setPointAsNext(newPt);
    return newPt;
}

SmartNodePoint *SmartNodePoint::addPointAbsPos(const QPointF &absPos) {
    return addPointRelPos(mapAbsoluteToRelative(absPos));
}

void SmartNodePoint::setElementsPos(const QPointF &c0,
                                    const QPointF &p1,
                                    const QPointF &c2) {
    setRelativePosVal(p1);
    mC0Pt->setRelativePosVal(c0 + p1);
    mC2Pt->setRelativePosVal(c2 + p1);
}

bool SmartNodePoint::isEndPoint() {
    return mNextPoint == nullptr || mPreviousPoint == nullptr;
}
