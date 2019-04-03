#include "smartnodepoint.h"
#include "undoredo.h"
#include <QPainter>
#include <QDebug>
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
    if(!mC0Pt->isSelected()) mC0Pt->rotateRelativeToSavedPivot(rot);
    if(!mC2Pt->isSelected()) mC2Pt->rotateRelativeToSavedPivot(rot);
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
    mParentAnimator->finishPathChange();
}

int SmartNodePoint::moveToClosestSegment(const QPointF &absPos) {
    const QPointF relPos = mapAbsoluteToRelative(absPos);
    return mHandler_k->moveToClosestSegment(getNodeId(), relPos);
}

#include <QApplication>
void SmartNodePoint::setRelativePos(const QPointF &relPos) {
    if(getType() == Node::NORMAL) {
        NonAnimatedMovablePoint::setRelativePos(relPos);
        currentPath()->actionSetNormalNodeP1(getNodeId(), getRelativePos());
        mNextNormalSegment.afterChanged();
        if(mPrevNormalPoint) mPrevNormalPoint->afterNextNodeC0P1Changed();

        const QPointF change = relPos - getSavedRelPos();
        mC0Pt->moveByRel(change);
        mC2Pt->moveByRel(change);
    } else if(getType() == Node::DISSOLVED) {
        const auto parentSeg = mPrevNormalPoint->getNextNormalSegment();
        const auto tRange = currentPath()->dissolvedTRange(getNodeId());
        const auto parentRelSeg = parentSeg.getAsRelSegment();
        auto seg = parentRelSeg.tFragment(tRange.fMin, tRange.fMax);
        const auto closest = seg.closestPosAndT(relPos);
        const qreal mappedT = gMapTFromFragment(tRange.fMin, tRange.fMax,
                                                closest.fT);
        currentPath()->actionSetDissolvedNodeT(getNodeId(), mappedT);
        NonAnimatedMovablePoint::setRelativePos(closest.fPos);
    }
    mParentAnimator->pathChanged();
}

void SmartNodePoint::removeFromVectorPath() {
    currentPath()->actionRemoveNode(getNodeId());
}

#include "pointtypemenu.h"
void SmartNodePoint::canvasContextMenu(PointTypeMenu * const menu) {
    if(isNormal() && !isEndPoint()) {
        std::function<void(SmartNodePoint*)> op = [](SmartNodePoint * pt) {
            pt->actionDemoteToDissolved(false);
        };
        menu->addPlainAction("Demote to dissolved", op);
        std::function<void(SmartNodePoint*)> opApprox = [](SmartNodePoint * pt) {
            pt->actionDemoteToDissolved(true);
        };
        menu->addPlainAction("Demote to dissolved approx.", opApprox);
    } else if(isDissolved()) {
        std::function<void(SmartNodePoint*)> op = [](SmartNodePoint * pt) {
            pt->actionPromoteToNormal();
        };
        menu->addPlainAction("Promote to normal", op);
    }
}

void SmartNodePoint::rectPointsSelection(
        const QRectF &absRect, QList<stdptr<MovablePoint>> &list) {
    if(!selectionEnabled()) return;
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

void SmartNodePoint::moveC0ToAbsPos(const QPointF &c0) {
    moveC0ToRelPos(mapAbsoluteToRelative(c0));
}

void SmartNodePoint::moveC2ToAbsPos(const QPointF &c2) {
    moveC2ToRelPos(mapAbsoluteToRelative(c2));
}

void SmartNodePoint::moveC2ToRelPos(const QPointF &c2) {
    if(!getC2Enabled()) setC2Enabled(true);
    mC2Pt->setRelativePos(c2);
}

void SmartNodePoint::moveC0ToRelPos(const QPointF &c0) {
    if(!getC0Enabled()) setC0Enabled(true);
    mC0Pt->setRelativePos(c0);
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

void SmartNodePoint::updateCtrlPtPos(SmartCtrlPoint * const pointToUpdate) {
    const auto otherPt = pointToUpdate == mC0Pt.get() ? mC2Pt : mC0Pt;
    const QPointF relPos = otherPt->getRelativePos();
    QPointF newPos;
    if(getCtrlsMode() == CtrlsMode::CTRLS_SYMMETRIC) {
        newPos = symmetricToPos(relPos, getRelativePos());
    } else if(getCtrlsMode() == CtrlsMode::CTRLS_SMOOTH) {
        const qreal distFromCenter =
                pointToLen(pointToUpdate->getRelativePos() -
                           getRelativePos());
        newPos = symmetricToPosNewLen(relPos, getRelativePos(),
                                      distFromCenter);
    } else newPos = relPos;
    pointToUpdate->::NonAnimatedMovablePoint::setRelativePos(newPos);
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

void drawCtrlPtLine(SkCanvas * const canvas,
                    const QPointF& qCtrlAbsPos,
                    const QPointF& qAbsPos,
                    const SkPoint& skAbsPos,
                    const SkScalar &invScale) {
    if(pointToLen(qCtrlAbsPos - qAbsPos) > 1) {
        const SkPoint skCtrlAbsPos = toSkPoint(qCtrlAbsPos);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        paint.setStrokeWidth(1.5f*invScale);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawLine(skAbsPos, skCtrlAbsPos, paint);

        paint.setColor(SK_ColorWHITE);
        paint.setStrokeWidth(0.75f*invScale);
        canvas->drawLine(skAbsPos, skCtrlAbsPos, paint);
    }
}

void SmartNodePoint::drawNodePoint(
        SkCanvas * const canvas,
        const CanvasMode &mode,
        const SkScalar &invScale,
        const bool &keyOnCurrent) {
    canvas->save();

    const QPointF qAbsPos = getAbsolutePos();
    const SkPoint skAbsPos = toSkPoint(qAbsPos);

    if(getType() == Node::NORMAL) {
        const SkColor fillCol = isSelected() ?
                    SkColorSetRGB(0, 200, 255) :
                    SkColorSetRGB(170, 240, 255);
        drawOnAbsPosSk(canvas, skAbsPos, invScale, fillCol, keyOnCurrent);

        if((mode == CanvasMode::MOVE_POINT && isNextNormalSelected()) ||
           (mode == CanvasMode::ADD_SMART_POINT && isSelected())) {
            SkPaint paint;
            paint.setAntiAlias(true);
            if(mC2Pt->isVisible() || mode == CanvasMode::ADD_SMART_POINT) {
                drawCtrlPtLine(canvas, mC2Pt->getAbsolutePos(),
                               qAbsPos, skAbsPos, invScale);
            }
            mC2Pt->drawSk(canvas, invScale);
        }
        if((mode == CanvasMode::MOVE_POINT && isPrevNormalSelected()) ||
           (mode == CanvasMode::ADD_SMART_POINT && isSelected())) {
            SkPaint paint;
            paint.setAntiAlias(true);
            if(mC0Pt->isVisible() || mode == CanvasMode::ADD_SMART_POINT) {
                drawCtrlPtLine(canvas, mC0Pt->getAbsolutePos(),
                               qAbsPos, skAbsPos, invScale);
            }
            mC0Pt->drawSk(canvas, invScale);
        }
    } else if(getType() == Node::DISSOLVED) {
        const SkColor fillCol = isSelected() ?
                    SkColorSetRGB(255, 0, 0) :
                    SkColorSetRGB(255, 120, 120);
        drawOnAbsPosSk(canvas, skAbsPos, invScale, fillCol, keyOnCurrent);
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
                           skAbsPos.x() + bounds.width()*0.5f,
                           skAbsPos.y() + bounds.height()*0.5f,
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
    mC2Pt->setRelativePos(getRelativePos());
}

void SmartNodePoint::resetC0() {
    mC0Pt->setRelativePos(getRelativePos());
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

void SmartNodePoint::actionMergeWithNormalPoint(SmartNodePoint * const other) {
    if(other == mNextNormalPoint || other == mPrevNormalPoint) {
        mHandler_k->mergeNodes(getNodeId(), other->getNodeId());
    }
}

void SmartNodePoint::actionPromoteToNormal() {
    return mHandler_k->promoteToNormal(getNodeId());
}

void SmartNodePoint::actionDemoteToDissolved(const bool& approx) {
    return mHandler_k->demoteToDissolved(getNodeId(), approx);
}

SmartNodePoint* SmartNodePoint::actionAddPointRelPos(const QPointF &relPos) {
    return mHandler_k->addNewAtEnd(relPos);
}

SmartNodePoint* SmartNodePoint::actionAddPointAbsPos(const QPointF &absPos) {
    return actionAddPointRelPos(mapAbsoluteToRelative(absPos));
}

void SmartNodePoint::c0Moved(const QPointF &c0) {
    currentPath()->actionSetNormalNodeC0(getNodeId(), c0);
    if(getCtrlsMode() != CTRLS_CORNER) {
        updateCtrlPtPos(mC2Pt.get());
        currentPath()->actionSetNormalNodeC2(getNodeId(),
                                             mC2Pt->getRelativePos());
    }

    if(mPrevNormalPoint)
        mPrevNormalPoint->afterNextNodeC0P1Changed();
    mParentAnimator->pathChanged();
}

void SmartNodePoint::c2Moved(const QPointF &c2) {
    currentPath()->actionSetNormalNodeC2(getNodeId(), c2);
    if(getCtrlsMode() != CTRLS_CORNER) {
        updateCtrlPtPos(mC0Pt.get());
        currentPath()->actionSetNormalNodeC0(getNodeId(),
                                             mC0Pt->getRelativePos());
    }

    mNextNormalSegment.afterChanged();
    mParentAnimator->pathChanged();
}

void SmartNodePoint::updateFromNodeDataPosOnly() {
    if(mNode_d->isNormal()) {
        mC0Pt->NonAnimatedMovablePoint::setRelativePos(mNode_d->c0());
        NonAnimatedMovablePoint::setRelativePos(mNode_d->p1());
        mC2Pt->NonAnimatedMovablePoint::setRelativePos(mNode_d->c2());
    } else if(mNode_d->isDissolved()) {
        currentPath()->updateDissolvedNodePosition(getNodeId());
        NonAnimatedMovablePoint::setRelativePos(mNode_d->p1());
    }
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
    if(type == Node::NORMAL) setRadius(6.5);
    else setRadius(type == Node::DISSOLVED ? 5.5 : 4);
    setSelectionEnabled(type == Node::NORMAL);
}

bool SmartNodePoint::isEndPoint() {
    return !mPrevNormalPoint || !mNextNormalPoint;
}
