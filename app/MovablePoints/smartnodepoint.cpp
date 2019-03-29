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

SmartNodePoint::SmartNodePoint(PathPointsHandler * const handler,
                               SmartPathAnimator * const parentAnimator,
                               BasicTransformAnimator * const parentTransform) :
    NonAnimatedMovablePoint(parentTransform, TYPE_SMART_PATH_POINT, 6.5),
    mHandler_k(handler), mParentAnimator(parentAnimator) {
    mC0Pt = SPtrCreate(SmartCtrlPoint)(this, SmartCtrlPoint::C0);
    mC2Pt = SPtrCreate(SmartCtrlPoint)(this, SmartCtrlPoint::C2);

    mC0Pt->setOtherCtrlPt(mC2Pt.get());
    mC2Pt->setOtherCtrlPt(mC0Pt.get());
}

void SmartNodePoint::applyTransform(const QMatrix &transform) {
    mC0Pt->applyTransform(transform);
    mC2Pt->applyTransform(transform);
    NonAnimatedMovablePoint::applyTransform(transform);
}

void SmartNodePoint::startTransform() {
    NonAnimatedMovablePoint::startTransform();
    if(!mC0Pt->isSelected()) mC0Pt->NonAnimatedMovablePoint::startTransform();
    if(!mC2Pt->isSelected()) mC2Pt->NonAnimatedMovablePoint::startTransform();
    mParentAnimator->startPathChange();
}

void SmartNodePoint::saveTransformPivotAbsPos(const QPointF &absPivot) {
    NonAnimatedMovablePoint::saveTransformPivotAbsPos(absPivot);
    if(!mC0Pt->isSelected()) mC0Pt->saveTransformPivotAbsPos(absPivot);
    if(!mC2Pt->isSelected()) mC2Pt->saveTransformPivotAbsPos(absPivot);
}

void SmartNodePoint::rotateRelativeToSavedPivot(const qreal &rot) {
    NonAnimatedMovablePoint::rotateRelativeToSavedPivot(rot);
    if(!mC0Pt->isSelected()) mC0Pt->rotate(rot);
    if(!mC2Pt->isSelected()) mC2Pt->rotate(rot);
}

void SmartNodePoint::scaleRelativeToSavedPivot(const qreal &sx,
                                          const qreal &sy) {
    NonAnimatedMovablePoint::scaleRelativeToSavedPivot(sx, sy);
    if(!mC0Pt->isSelected()) mC0Pt->scale(sx, sy);
    if(!mC2Pt->isSelected()) mC2Pt->scale(sx, sy);
}

void SmartNodePoint::cancelTransform() {
    NonAnimatedMovablePoint::cancelTransform();
    if(!mC0Pt->isSelected()) mC0Pt->NonAnimatedMovablePoint::cancelTransform();
    if(!mC2Pt->isSelected()) mC2Pt->NonAnimatedMovablePoint::cancelTransform();
    mParentAnimator->cancelPathChange();
}

void SmartNodePoint::finishTransform() {
    NonAnimatedMovablePoint::finishTransform();
    if(!mC0Pt->isSelected())
        mC0Pt->NonAnimatedMovablePoint::finishTransform();
    if(!mC2Pt->isSelected())
        mC2Pt->NonAnimatedMovablePoint::finishTransform();
    mParentAnimator->finishPathChange();
}

void SmartNodePoint::setRelativePos(const QPointF &relPos) {
    if(getType() == Node::NORMAL) {
        setRelativePosVal(relPos);
        currentPath()->actionSetNormalNodeP1(getNodeId(), mCurrentPos);
        mNextNormalSegment.afterChanged();
        if(mPrevNormalPoint) mPrevNormalPoint->afterNextNodeC0P1Changed();

        const QPointF change = relPos - mSavedRelPos;
        mC0Pt->NonAnimatedMovablePoint::moveByRel(change);
        mC2Pt->NonAnimatedMovablePoint::moveByRel(change);
    } else if(getType() == Node::DISSOLVED) {
        const auto parentSeg = mPrevNormalPoint->getNextNormalSegment();
        const auto tRange = currentPath()->dissolvedTRange(getNodeId());
        auto seg = parentSeg.getAsRelSegment().tFragment(tRange.fMin,
                                                         tRange.fMax);
        const auto closest = seg.closestPosAndT(relPos);
        const qreal mappedT = gMapTFromFragment(tRange.fMin, tRange.fMax,
                                                closest.fT);
        currentPath()->actionSetDissolvedNodeT(getNodeId(), mappedT);
        setRelativePosVal(closest.fPos);
    }
    mParentAnimator->pathChanged();
}

void SmartNodePoint::removeFromVectorPath() {
    currentPath()->actionRemoveNode(getNodeId());
}

void SmartNodePoint::removeApproximate() {
    currentPath()->actionRemoveNode(getNodeId());
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
    } else if(!isEndPoint() || canvasMode != CanvasMode::ADD_SMART_POINT) {
        return nullptr;
    }
    if(isPointAtAbsPos(absPos, canvasScaleInv)) return this;
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
    mC2Pt->setRelativePos(c2);
    ctrlPointPosChanged(mC2Pt.get(), mC0Pt.get());
}

void SmartNodePoint::moveC0ToRelPos(const QPointF &c0) {
    if(!getC0Enabled()) setC0Enabled(true);
    mC0Pt->setRelativePos(c0);
    ctrlPointPosChanged(mC0Pt.get(), mC2Pt.get());
}

QPointF SmartNodePoint::getC0AbsPos() const {
    return mapRelativeToAbsolute(getC0Value());
}

QPointF SmartNodePoint::getC0Value() const {
    if(getC0Enabled()) return mC0Pt->getRelativePos();
    return getRelativePos();
}

SmartCtrlPoint *SmartNodePoint::getC0Pt() {
    return mC0Pt.get();
}

void SmartNodePoint::ctrlPointPosChanged(
        const SmartCtrlPoint * const pointChanged,
        SmartCtrlPoint * const pointToUpdate) {
    const QPointF changedPointPos = pointChanged->getAbsolutePos();
    if(getCtrlsMode() == CtrlsMode::CTRLS_SYMMETRIC) {
        pointToUpdate->NonAnimatedMovablePoint::moveToAbs(
                    symmetricToAbsPos(changedPointPos));
    } else if(getCtrlsMode() == CtrlsMode::CTRLS_SMOOTH) {
        if(!isPointZero(changedPointPos) ) {
            pointToUpdate->NonAnimatedMovablePoint::moveToAbs(
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
    if(mNode_d->getC2Enabled()) return mC2Pt->getRelativePos();
    return getRelativePos();
}

SmartCtrlPoint *SmartNodePoint::getC2Pt() {
    return mC2Pt.get();
}

void SmartNodePoint::drawNodePoint(
        SkCanvas * const canvas,
        const CanvasMode &mode,
        const SkScalar &invScale,
        const bool &keyOnCurrent) {
    canvas->save();

    const SkPoint absPos = toSkPoint(getAbsolutePos());
    if(getType() == Node::NORMAL) {
        const SkColor fillCol = mSelected ?
                    SkColorSetRGB(0, 200, 255) :
                    SkColorSetRGB(170, 240, 255);
        drawOnAbsPosSk(canvas, absPos, invScale, fillCol, keyOnCurrent);

        if((mode == CanvasMode::MOVE_POINT && isNextNormalSelected()) ||
           (mode == CanvasMode::ADD_SMART_POINT && mSelected)) {
            SkPaint paint;
            paint.setAntiAlias(true);
            if(mC2Pt->isVisible() || mode == CanvasMode::ADD_SMART_POINT) {
                const SkPoint endAbsPos = toSkPoint(mC2Pt->getAbsolutePos());
                paint.setColor(SK_ColorBLACK);
                paint.setStrokeWidth(1.5f*invScale);
                paint.setStyle(SkPaint::kStroke_Style);

                canvas->drawLine(absPos, endAbsPos, paint);
                paint.setColor(SK_ColorWHITE);
                paint.setStrokeWidth(0.75f*invScale);
                canvas->drawLine(absPos, endAbsPos, paint);
            }
            mC2Pt->drawSk(canvas, invScale);
        }
        if((mode == CanvasMode::MOVE_POINT && isPrevNormalSelected()) ||
           (mode == CanvasMode::ADD_SMART_POINT && mSelected)) {
            SkPaint paint;
            paint.setAntiAlias(true);
            if(mC0Pt->isVisible() || mode == CanvasMode::ADD_SMART_POINT) {
                const SkPoint startAbsPos = toSkPoint(mC0Pt->getAbsolutePos());
                paint.setColor(SK_ColorBLACK);
                paint.setStrokeWidth(1.5f*invScale);
                paint.setStyle(SkPaint::kStroke_Style);
                canvas->drawLine(absPos, startAbsPos, paint);

                paint.setColor(SK_ColorWHITE);
                paint.setStrokeWidth(0.75f*invScale);
                canvas->drawLine(absPos, startAbsPos, paint);
            }
            mC0Pt->drawSk(canvas, invScale);
        }
    } else if(getType() == Node::DISSOLVED) {
        const SkColor fillCol = mSelected ?
                    SkColorSetRGB(255, 0, 0) :
                    SkColorSetRGB(255, 120, 120);
        drawOnAbsPosSk(canvas, absPos, invScale, fillCol, keyOnCurrent);
    }

    if(MainWindow::isCtrlPressed()) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        paint.setStyle(SkPaint::kFill_Style);

        SkFont font;
        font.setSize(FONT_HEIGHT*invScale);
        const auto fontStyle = SkFontStyle(SkFontStyle::kBold_Weight,
                                           SkFontStyle::kNormal_Width,
                                           SkFontStyle::kUpright_Slant);
        font.setTypeface(SkTypeface::MakeFromName(nullptr, fontStyle));
        const auto nodeIdStr = QString::number(getNodeId());
        const ulong sizeT = static_cast<ulong>(nodeIdStr.size());
        const auto cStr = nodeIdStr.toStdString().c_str();
        SkRect bounds;
        font.measureText(cStr,
                         sizeT*sizeof(char),
                         SkTextEncoding::kUTF8,
                         &bounds);

        canvas->drawString(cStr,
                           absPos.x() + bounds.width()*0.5f,
                           absPos.y() + bounds.height()*0.5f,
                           font, paint);
    }
    canvas->restore();
}

SmartNodePoint* SmartNodePoint::getNextPoint() {
    return mNextPoint;
}

SmartNodePoint *SmartNodePoint::getPreviousPoint() {
    return mPrevPoint;
}

SmartNodePoint *SmartNodePoint::getConnectedSeparateNodePoint() {
    if(isSeparateNodePoint() || !mPrevNormalPoint) return this;
    return mPrevNormalPoint->getConnectedSeparateNodePoint();
}

void SmartNodePoint::updateC0Visibility() {
    if(!mPrevNormalPoint) mC0Pt->hide();
    else mC0Pt->setVisible(mNode_d->getC0Enabled() &&
                           mNode_d->isNormal());
}

void SmartNodePoint::updateC2Visibility() {
    if(!mNextNormalPoint) mC2Pt->hide();
    else mC2Pt->setVisible(mNode_d->getC2Enabled() &&
                           mNode_d->isNormal());
}

void SmartNodePoint::setC2Enabled(const bool &enabled) {
    if(enabled == getC2Enabled()) return;
    if(getC2Enabled()) setCtrlsMode(CtrlsMode::CTRLS_CORNER);
    currentPath()->actionSetNormalNodeC2Enabled(getNodeId(), enabled);
    updateC2Visibility();
    mParentAnimator->pathChanged();
}

void SmartNodePoint::setC0Enabled(const bool &enabled) {
    if(enabled == getC0Enabled()) return;
    if(mNode_d->getC0Enabled()) setCtrlsMode(CtrlsMode::CTRLS_CORNER);
    currentPath()->actionSetNormalNodeC0Enabled(getNodeId(), enabled);
    updateC0Visibility();
    mParentAnimator->pathChanged();
}

void SmartNodePoint::resetC2() {
    mC2Pt->setRelativePosStartAndFinish(getRelativePos());
}

void SmartNodePoint::resetC0() {
    mC0Pt->setRelativePosStartAndFinish(getRelativePos());
}

int SmartNodePoint::getNodeId() const {
    return mNode_d->getNodeId();
}

NodePointValues SmartNodePoint::getPointValues() const {
    return {getC0Value(), getRelativePos(), getC2Value()};
}

bool SmartNodePoint::isPrevNormalSelected() const {
    const bool prevSelected = mPrevNormalPoint ?
                mPrevNormalPoint->isSelected() : false;
    return isSelected() || prevSelected;
}

bool SmartNodePoint::isNextNormalSelected() const {
    const bool nextSelected = mNextNormalPoint ?
                mNextNormalPoint->isSelected() : false;
    return isSelected() || nextSelected;
}

bool SmartNodePoint::isNeighbourNormalSelected() const {
    const bool nextSelected = mNextNormalPoint ?
                mNextNormalPoint->isSelected() : false;
    const bool prevSelected = mPrevNormalPoint ?
                mPrevNormalPoint->isSelected() : false;
    return isSelected() || nextSelected || prevSelected;
}

SmartPathAnimator *SmartNodePoint::getTargetAnimator() const {
    return mParentAnimator;
}

void SmartNodePoint::setSeparateNodePoint(const bool &separateNodePoint) {
    mSeparateNodePoint = separateNodePoint;
}

bool SmartNodePoint::isSeparateNodePoint() {
    return mSeparateNodePoint;
}

void SmartNodePoint::setCtrlsMode(const CtrlsMode &mode) {
    if(getCtrlsMode() == mode) return;
    currentPath()->actionSetNormalNodeCtrlsMode(getNodeId(), mode);
    updateFromNodeData();
    mParentAnimator->pathChanged();
    if(mode == CtrlsMode::CTRLS_CORNER) return;
    mNextNormalSegment.afterChanged();
    if(mPrevNormalPoint) mPrevNormalPoint->afterNextNodeC0P1Changed();
}

bool SmartNodePoint::hasNextNormalPoint() const {
    return mNextNormalPoint;
}

bool SmartNodePoint::hasPrevNormalPoint() const {
    return mPrevNormalPoint;
}

const PathPointsHandler *SmartNodePoint::getHandler() {
    return mHandler_k.data();
}

void SmartNodePoint::setPrevNormalPoint(SmartNodePoint * const prevPoint) {
    if(mPrevNormalPoint == this) RuntimeThrow("Node cannot point to itself");
    mPrevNormalPoint = prevPoint;
    updateC0Visibility();
}

void SmartNodePoint::setNextNormalPoint(SmartNodePoint * const nextPoint) {
    if(mNextNormalPoint == this) RuntimeThrow("Node cannot point to itself");
    mNextNormalPoint = nextPoint;
    if(isNormal() && nextPoint) {
        mNextNormalSegment = NormalSegment(this, nextPoint, mHandler_k);
    } else mNextNormalSegment.reset();
    updateC2Visibility();
}

SmartPath *SmartNodePoint::currentPath() const {
    return mParentAnimator->getCurrentlyEditedPath();
}

bool SmartNodePoint::hasNextPoint() const {
    return mNextPoint;
}

bool SmartNodePoint::hasPrevPoint() const {
    return mPrevPoint;
}

void SmartNodePoint::setPrevPoint(SmartNodePoint * const prevPoint) {
    if(prevPoint == this) RuntimeThrow("Node cannot point to itself");
    mPrevPoint = prevPoint;
}

void SmartNodePoint::setNextPoint(SmartNodePoint * const nextPoint) {
    if(nextPoint == this) RuntimeThrow("Node cannot point to itself");
    mNextPoint = nextPoint;
}

bool SmartNodePoint::actionConnectToNormalPoint(
        SmartNodePoint * const other) {
    const bool thisHasPrev = hasPrevNormalPoint();
    const bool thisHasNext = hasNextNormalPoint();
    if(thisHasNext && thisHasPrev) return false;
    const bool otherHasPrev = other->hasPrevNormalPoint();
    const bool otherHasNext = other->hasNextNormalPoint();
    if(otherHasPrev && otherHasNext) return false;

    const auto thisAnim = getTargetAnimator();
    const auto otherAnim = other->getTargetAnimator();
    if(!thisAnim || !otherAnim) return false;
    const bool sameAnim = thisAnim == otherAnim;
    if(sameAnim) {
        if(!hasNextNormalPoint()) {
            mHandler_k->createSegment(getNodeId(), other->getNodeId());
        } else if(!hasPrevNormalPoint()) {
            mHandler_k->createSegment(other->getNodeId(), getNodeId());
        } else return false;
    } else {
        const auto thisParentAnim = thisAnim->getParent();
        const auto otherParentAnim = otherAnim->getParent();
        if(thisParentAnim != otherParentAnim) return false;
        if(!thisHasNext && !otherHasPrev) {
            otherAnim->actionPrependMoveAllFrom(thisAnim);
        } else if(!thisHasPrev && !otherHasNext) {
            otherAnim->actionAppendMoveAllFrom(thisAnim);
        } else { // one path has to be reversed to connect
            thisAnim->actionReverseAll();
            if(!thisHasPrev) otherAnim->actionPrependMoveAllFrom(thisAnim);
            else otherAnim->actionAppendMoveAllFrom(thisAnim);
        }
    }
    return true;
}

void SmartNodePoint::actionDisconnectFromNormalPoint(
        SmartNodePoint * const other) {
    if(other == mNextNormalPoint) {
        mHandler_k->removeSegment(mNextNormalSegment);
    } else if(other == mPrevNormalPoint) {
        mPrevNormalPoint->actionDisconnectFromNormalPoint(this);
    }
}

SmartNodePoint* SmartNodePoint::actionAddPointRelPos(const QPointF &relPos) {
    return mHandler_k->addNewAtEnd(relPos);
}

SmartNodePoint* SmartNodePoint::actionAddPointAbsPos(const QPointF &absPos) {
    return actionAddPointRelPos(mapAbsoluteToRelative(absPos));
}

void SmartNodePoint::setElementsPos(const QPointF &c0,
                                    const QPointF &p1,
                                    const QPointF &c2) {
    setRelativePosVal(p1);
    mC0Pt->setRelativePosVal(c0 + p1);
    mC2Pt->setRelativePosVal(c2 + p1);
}

void SmartNodePoint::c0Moved(const QPointF &c0) {
    currentPath()->actionSetNormalNodeC0(getNodeId(), c0);
    if(mPrevNormalPoint)
        mPrevNormalPoint->afterNextNodeC0P1Changed();
    mParentAnimator->pathChanged();
}

void SmartNodePoint::c2Moved(const QPointF &c2) {
    currentPath()->actionSetNormalNodeC2(getNodeId(), c2);
    mNextNormalSegment.afterChanged();
    mParentAnimator->pathChanged();
}

void SmartNodePoint::updateFromNodeDataPosOnly() {
    if(mNode_d->isNormal()) {
        mC0Pt->setRelativePosVal(mNode_d->getC0());
        setRelativePosVal(mNode_d->fP1);
        mC2Pt->setRelativePosVal(mNode_d->getC2());
    } else if(mNode_d->isDissolved()) {
        currentPath()->updateDissolvedNodePosition(getNodeId());
        setRelativePosVal(mNode_d->fP1);
    }
}

void SmartNodePoint::testNode_TEST() {
    Q_ASSERT(mNode_d);

    const int prevNodeId = currentPath()->prevNodeId(mNode_d->getNodeId());
    const auto prevNode = mHandler_k->getPointWithId(prevNodeId);
    Q_ASSERT(prevNode == mPrevPoint);

    const int nextNodeId = currentPath()->nextNodeId(mNode_d->getNodeId());
    const auto nextNode = mHandler_k->getPointWithId(nextNodeId);
    Q_ASSERT(nextNode == mNextPoint);

    const auto prevNormalNode = mHandler_k->getPrevNormalNode(getNodeId());
    Q_ASSERT(prevNormalNode == mPrevNormalPoint);

    const auto nextNormalNode = mHandler_k->getNextNormalNode(getNodeId());
    Q_ASSERT(nextNormalNode == mNextNormalPoint);
}

void SmartNodePoint::updateFromNodeData() {
    if(!mNode_d) {
        clear();
        return;
    }

    updateFromNodeDataPosOnly();

    const int prevNodeId = currentPath()->prevNodeId(mNode_d->getNodeId());
    const auto prevNode = mHandler_k->getPointWithId(prevNodeId);
    setPrevPoint(prevNode);

    const int nextNodeId = currentPath()->nextNodeId(mNode_d->getNodeId());
    const auto nextNode = mHandler_k->getPointWithId(nextNodeId);
    setNextPoint(nextNode);

    const auto prevNormalNode = mHandler_k->getPrevNormalNode(getNodeId());
    setPrevNormalPoint(prevNormalNode);

    const auto nextNormalNode = mHandler_k->getNextNormalNode(getNodeId());
    setNextNormalPoint(nextNormalNode);

    updateC0Visibility();
    updateC2Visibility();
    const auto type = getType();
    if(type == Node::NORMAL) {
        mType = TYPE_SMART_PATH_POINT;
        mRadius = 6.5;
    } else {
        mType = TYPE_CTRL_POINT;
        mRadius = (type == Node::DISSOLVED ? 5.5 : 4);
    }
}

bool SmartNodePoint::isEndPoint() {
    return !mPrevNormalPoint || !mNextNormalPoint;
}
