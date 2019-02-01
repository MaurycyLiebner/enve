#include "Boxes/boundingbox.h"
#include "canvas.h"
#include "undoredo.h"
#include "Boxes/boxesgroup.h"
#include <QDebug>
#include <QMenu>
#include "singlewidgetabstraction.h"
#include "durationrectangle.h"
#include "pointhelpers.h"
#include "skia/skqtconversions.h"
#include "global.h"
#include "MovablePoints/movablepoint.h"
#include "PropertyUpdaters/pixmapeffectupdater.h"
#include "taskscheduler.h"
#include "PixmapEffects/pixmapeffect.h"
#include "Animators/effectanimators.h"
#include "Animators/gpueffectanimators.h"
#include "Animators/transformanimator.h"
#include "GPUEffects/gpurastereffect.h"
#include "linkbox.h"

QList<qptr<BoundingBox>> BoundingBox::mLoadedBoxes;
QList<stdsptr<FunctionWaitingForBoxLoad>> BoundingBox::mFunctionsWaitingForBoxLoad;

BoundingBox::BoundingBox(const BoundingBoxType &type) :
    ComplexAnimator("box") {
    mTransformAnimator = SPtrCreate(BoxTransformAnimator)(this);
    ca_addChildAnimator(mTransformAnimator);

    mEffectsAnimators = SPtrCreate(EffectAnimators)(this);
    mEffectsAnimators->prp_setUpdater(SPtrCreate(PixmapEffectUpdater)(this));
    mEffectsAnimators->prp_blockUpdater();
    ca_addChildAnimator(mEffectsAnimators);
    mEffectsAnimators->SWT_hide();

    mGPUEffectsAnimators = SPtrCreate(GPUEffectAnimators)(this);
    mGPUEffectsAnimators->prp_setUpdater(SPtrCreate(PixmapEffectUpdater)(this));
    mGPUEffectsAnimators->prp_blockUpdater();
    ca_addChildAnimator(mGPUEffectsAnimators);
    mGPUEffectsAnimators->SWT_hide();

    mType = type;

    mTransformAnimator->reset();
}

BoundingBox::~BoundingBox() {}

void BoundingBox::prp_updateAfterChangedAbsFrameRange(const FrameRange &range) {
    Property::prp_updateAfterChangedAbsFrameRange(range);
    if(range.inRange(anim_mCurrentAbsFrame)) {
        scheduleUpdate(Animator::USER_CHANGE);
    }
}

void BoundingBox::ca_childAnimatorIsRecordingChanged() {
    ComplexAnimator::ca_childAnimatorIsRecordingChanged();
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Animated);
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_NotAnimated);
}

SingleWidgetAbstraction* BoundingBox::SWT_getAbstractionForWidget(
            const UpdateFuncs &updateFuncs,
            const int& visiblePartWidgetId) {
    Q_FOREACH(const stdsptr<SingleWidgetAbstraction> &abs,
              mSWT_allAbstractions) {
        if(abs->getParentVisiblePartWidgetId() == visiblePartWidgetId) {
            return abs.get();
        }
    }
    return SWT_createAbstraction(updateFuncs, visiblePartWidgetId);
}

qsptr<BoundingBox> BoundingBox::createLink() {
    auto linkBox = SPtrCreate(InternalLinkBox)(this);
    copyBoundingBoxDataTo(linkBox.get());
    return linkBox;
}

qsptr<BoundingBox> BoundingBox::createLinkForLinkGroup() {
    qsptr<BoundingBox> box = createLink();
    box->clearEffects();
    return box;
}

void BoundingBox::clearEffects() {
    mEffectsAnimators->ca_removeAllChildAnimators();
}

void BoundingBox::setFont(const QFont &) {}

void BoundingBox::setSelectedFontSize(const qreal &) {}

void BoundingBox::setSelectedFontFamilyAndStyle(const QString &, const QString &) {}

QPointF BoundingBox::getRelCenterPosition() {
    return mRelBoundingRect.center();
}

void BoundingBox::centerPivotPosition() {
    mTransformAnimator->setPivotWithoutChangingTransformation(
                getRelCenterPosition());
}

void BoundingBox::setPivotPosition(const QPointF &pos) {
    mTransformAnimator->setPivotWithoutChangingTransformation(pos);
}

void BoundingBox::copyBoundingBoxDataTo(BoundingBox * const targetBox) const {
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    BoundingBox::writeBoundingBoxDataForLink(&buffer);
    if(buffer.seek(sizeof(BoundingBoxType)) ) {
        targetBox->BoundingBox::readBoundingBoxDataForLink(&buffer);
    }
    buffer.close();
}

void BoundingBox::drawHoveredSk(SkCanvas *canvas, const SkScalar &invScale) {
    drawHoveredPathSk(canvas, mSkRelBoundingRectPath, invScale);
}

void BoundingBox::drawHoveredPathSk(SkCanvas *canvas,
                                    const SkPath &path,
                                    const SkScalar &invScale) {
    canvas->save();
    SkPath mappedPath = path;
    mappedPath.transform(QMatrixToSkMatrix(
                             mTransformAnimator->getCombinedTransform()));
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(2*invScale);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(mappedPath, paint);

    paint.setColor(SK_ColorRED);
    paint.setStrokeWidth(invScale);
    canvas->drawPath(mappedPath, paint);
    canvas->restore();
}

void BoundingBox::applyPaintSetting( PaintSetting *setting) {
    Q_UNUSED(setting);
}

void BoundingBox::setFillColorMode(const ColorMode &colorMode) {
    Q_UNUSED(colorMode);
}

void BoundingBox::setStrokeColorMode(const ColorMode &colorMode) {
    Q_UNUSED(colorMode);
}

bool BoundingBox::isAncestor(BoxesGroup *box) const {
    if(mParentGroup == box) return true;
    if(!mParentGroup) return false;
    return mParentGroup->isAncestor(box);
}

bool BoundingBox::isAncestor(BoundingBox *box) const {
    if(box->SWT_isBoxesGroup()) {
        return isAncestor(box);
    }
    return false;
}

Canvas *BoundingBox::getParentCanvas() {
    if(!mParentGroup) return nullptr;
    return mParentGroup->getParentCanvas();
}

void BoundingBox::reloadCacheHandler() { clearAllCache(); }

bool BoundingBox::SWT_isBoundingBox() const { return true; }

void BoundingBox::updateAllBoxes(const UpdateReason &reason) {
    scheduleUpdate(reason);
}

void BoundingBox::prp_updateInfluenceRangeAfterChanged() {
    auto visRange = getVisibleAbsFrameRange();
    prp_updateAfterChangedAbsFrameRange(visRange);
}

void BoundingBox::clearAllCache() {
    prp_updateInfluenceRangeAfterChanged();
}

void BoundingBox::drawSelectedSk(SkCanvas *canvas,
                                 const CanvasMode &currentCanvasMode,
                                 const SkScalar &invScale) {
    if(isVisibleAndInVisibleDurationRect()) {
        canvas->save();
        drawBoundingRectSk(canvas, invScale);
        if(currentCanvasMode == MOVE_PATH) {
            mTransformAnimator->getPivotMovablePoint()->
                    drawSk(canvas, invScale);
        }
        canvas->restore();
    }
}

void BoundingBox::drawPixmapSk(SkCanvas * const canvas,
                               GrContext * const grContext) {
    if(isVisibleAndInVisibleDurationRect() &&
        mTransformAnimator->getOpacity() > 0.001) {
        canvas->save();

        SkPaint paint;
        int intAlpha = qRound(mTransformAnimator->getOpacity()*2.55);
        paint.setAlpha(static_cast<U8CPU>(intAlpha));
        paint.setBlendMode(mBlendModeSk);
        //paint.setFilterQuality(kHigh_SkFilterQuality);
        drawPixmapSk(canvas, &paint, grContext);
        canvas->restore();
    }
}

void BoundingBox::drawPixmapSk(SkCanvas *canvas, SkPaint *paint,
                               GrContext* const grContext) {
    if(mTransformAnimator->getOpacity() < 0.001) { return; }
    //paint->setFilterQuality(kHigh_SkFilterQuality);
    mDrawRenderContainer->drawSk(canvas, paint, grContext);
}

void BoundingBox::setBlendModeSk(const SkBlendMode &blendMode) {
    mBlendModeSk = blendMode;
    prp_updateInfluenceRangeAfterChanged();
}

const SkBlendMode &BoundingBox::getBlendMode() {
    return mBlendModeSk;
}

void BoundingBox::resetScale() {
    mTransformAnimator->resetScale();
}

void BoundingBox::resetTranslation() {
    mTransformAnimator->resetTranslation();
}

void BoundingBox::resetRotation() {
    mTransformAnimator->resetRotation();
}

void BoundingBox::prp_setAbsFrame(const int &frame) {
    int lastRelFrame = anim_mCurrentRelFrame;
    ComplexAnimator::prp_setAbsFrame(frame);
    bool isInVisRange = isRelFrameInVisibleDurationRect(
                anim_mCurrentRelFrame);
    if(mUpdateDrawOnParentBox != isInVisRange) {
        if(mUpdateDrawOnParentBox) {
            if(mParentGroup) mParentGroup->scheduleUpdate(Animator::FRAME_CHANGE);
        } else {
            scheduleUpdate(Animator::FRAME_CHANGE);
        }
        mUpdateDrawOnParentBox = isInVisRange;
    }
    if(prp_differencesBetweenRelFrames(lastRelFrame, anim_mCurrentRelFrame)) {
        scheduleUpdate(Animator::FRAME_CHANGE);
    }
}

void BoundingBox::startAllPointsTransform() {}

void BoundingBox::finishAllPointsTransform() {}

void BoundingBox::setStrokeCapStyle(const Qt::PenCapStyle &capStyle) {
    Q_UNUSED(capStyle); }

void BoundingBox::setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle) {
    Q_UNUSED(joinStyle); }

void BoundingBox::setStrokeWidth(const qreal &strokeWidth) {
    Q_UNUSED(strokeWidth);
}

void BoundingBox::startSelectedStrokeWidthTransform() {}

void BoundingBox::startSelectedStrokeColorTransform() {}

void BoundingBox::startSelectedFillColorTransform() {}

VectorPathEdge *BoundingBox::getEdge(const QPointF &absPos, const qreal &canvasScaleInv) {
    Q_UNUSED(absPos);
    Q_UNUSED(canvasScaleInv);
    return nullptr;
}

bool BoundingBox::prp_differencesBetweenRelFramesIncludingInherited(
        const int &relFrame1, const int &relFrame2) {
    bool diffThis = prp_differencesBetweenRelFrames(relFrame1, relFrame2);
    if(!mParentGroup || diffThis) return diffThis;
    int absFrame1 = prp_relFrameToAbsFrame(relFrame1);
    int absFrame2 = prp_relFrameToAbsFrame(relFrame2);
    int parentRelFrame1 = mParentGroup->prp_absFrameToRelFrame(absFrame1);
    int parentRelFrame2 = mParentGroup->prp_absFrameToRelFrame(absFrame2);

    bool diffInherited =
            mParentGroup->prp_differencesBetweenRelFramesIncludingInheritedExcludingContainedBoxes(
                parentRelFrame1, parentRelFrame2);
    return diffThis || diffInherited;
}

void BoundingBox::setParentGroup(BoxesGroup *parent) {
    mParentGroup = parent;
    if(!mParentGroup) return;
    mParentTransform = parent->getTransformAnimator();
    mTransformAnimator->setParentTransformAnimator(mParentTransform);

    prp_setAbsFrame(mParentGroup->anim_getCurrentAbsFrame());
    mTransformAnimator->updateCombinedTransform(Animator::USER_CHANGE);
}

void BoundingBox::setParentTransform(BasicTransformAnimator *parent) {
    if(parent == mParentTransform) return;
    mParentTransform = parent;
    mTransformAnimator->setParentTransformAnimator(mParentTransform);

    mTransformAnimator->updateCombinedTransform(Animator::USER_CHANGE);
}

void BoundingBox::clearParent() {
    setParentTransform(mParentGroup->getTransformAnimator());
}

BoxesGroup *BoundingBox::getParentGroup() const {
    return mParentGroup;
}

void BoundingBox::setPivotRelPos(const QPointF &relPos,
                                 const bool &pivotAutoAdjust) {
    mPivotAutoAdjust = pivotAutoAdjust;
    mTransformAnimator->setPivotWithoutChangingTransformation(relPos);
    requestGlobalPivotUpdateIfSelected();
}

void BoundingBox::startPivotTransform() {
    mTransformAnimator->startPivotTransform();
}

void BoundingBox::finishPivotTransform() {
    mTransformAnimator->finishPivotTransform();
}

void BoundingBox::setPivotAbsPos(const QPointF &absPos,
                                 const bool &pivotChanged) {
    setPivotRelPos(mapAbsPosToRel(absPos), pivotChanged);
    //updateCombinedTransform();
}

QPointF BoundingBox::getPivotAbsPos() {
    return mTransformAnimator->getPivotAbs();
}

void BoundingBox::select() {
    mSelected = true;

    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Selected);
}

void BoundingBox::updateRelBoundingRectFromRenderData(
        BoundingBoxRenderData* renderData) {
    mRelBoundingRect = renderData->fRelBoundingRect;
    mRelBoundingRectSk = QRectFToSkRect(mRelBoundingRect);
    mSkRelBoundingRectPath = SkPath();
    mSkRelBoundingRectPath.addRect(mRelBoundingRectSk);

    if(mPivotAutoAdjust && !prp_isDescendantRecording()) {
        setPivotPosition(renderData->getCenterPosition());
    }
}

void BoundingBox::updateCurrentPreviewDataFromRenderData(
        BoundingBoxRenderData* renderData) {
    updateRelBoundingRectFromRenderData(renderData);
}

bool BoundingBox::shouldScheduleUpdate() {
    if(!mParentGroup) return false;
    if((isVisibleAndInVisibleDurationRect()) ||
            (isRelFrameInVisibleDurationRect(anim_mCurrentRelFrame))) {
        return true;
    }
    return false;
}

void BoundingBox::scheduleUpdate(const UpdateReason &reason) {
    scheduleUpdate(anim_mCurrentRelFrame, reason);
}

void BoundingBox::scheduleUpdate(const int &relFrame,
                                 const UpdateReason& reason) {
    if(!shouldScheduleUpdate()) return;
    if(reason != UpdateReason::FRAME_CHANGE) mStateId++;
    mDrawRenderContainer->setExpired(true);
    auto currentRenderData = getCurrentRenderData(relFrame);
    if(currentRenderData) {
        if(currentRenderData->fRedo) return;
        if(reason != UpdateReason::FRAME_CHANGE) return;
        currentRenderData->fRedo = currentRenderData->isQued();
        return;
    }
    currentRenderData = updateCurrentRenderData(relFrame, reason);
    auto currentReason = currentRenderData->fReason;
    if(reason == USER_CHANGE &&
            (currentReason == CHILD_USER_CHANGE ||
             currentReason == FRAME_CHANGE)) {
        currentRenderData->fReason = reason;
    } else if(reason == CHILD_USER_CHANGE &&
              currentReason == FRAME_CHANGE) {
        currentRenderData->fReason = reason;
    }
    currentRenderData->fRedo = false;
    currentRenderData->scheduleTask();

    //mUpdateDrawOnParentBox = isVisibleAndInVisibleDurationRect();

    if(mParentGroup) {
        mParentGroup->scheduleUpdate(reason == USER_CHANGE ?
                                         CHILD_USER_CHANGE : reason);
    }

    emit scheduledUpdate();
}

BoundingBoxRenderData *BoundingBox::updateCurrentRenderData(
        const int& relFrame, const UpdateReason& reason) {
    auto renderData = createRenderData();
    Q_ASSERT(renderData);
    renderData->fRelFrame = relFrame;
    renderData->fReason = reason;
    mCurrentRenderDataHandler.addItemAtRelFrame(renderData);
    return renderData.get();
}

BoundingBoxRenderData *BoundingBox::getCurrentRenderData(const int& relFrame) {
    BoundingBoxRenderData* currentRenderData =
            mCurrentRenderDataHandler.getItemAtRelFrame(relFrame);
    if(!currentRenderData) {
        if(mDrawRenderContainer->isExpired()) return nullptr;
        currentRenderData = mDrawRenderContainer->getSrcRenderData();
        if(!currentRenderData) return nullptr;
//        if(currentRenderData->fRelFrame == relFrame) {
        if(!prp_differencesBetweenRelFramesIncludingInherited(
                    currentRenderData->fRelFrame, relFrame)) {
            auto copy = currentRenderData->makeCopy();
            copy->fRelFrame = relFrame;
            mCurrentRenderDataHandler.addItemAtRelFrame(copy);
            return copy.get();
        }
        return nullptr;
    }
    return currentRenderData;
}


void BoundingBox::nullifyCurrentRenderData(const int& relFrame) {
    mCurrentRenderDataHandler.removeItemAtRelFrame(relFrame);
}

void BoundingBox::deselect() {
    mSelected = false;

    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Selected);
}

bool BoundingBox::isContainedIn(const QRectF &absRect) const {
    return absRect.contains(getCombinedTransform().mapRect(mRelBoundingRect));
}

BoundingBox *BoundingBox::getPathAtFromAllAncestors(const QPointF &absPos) {
    if(absPointInsidePath(absPos)) return this;
    return nullptr;
}

QPointF BoundingBox::mapAbsPosToRel(const QPointF &absPos) {
    return mTransformAnimator->mapAbsPosToRel(absPos);
}

PaintSettings *BoundingBox::getFillSettings() const {
    return nullptr;
}

StrokeSettings *BoundingBox::getStrokeSettings() const {
    return nullptr;
}

void BoundingBox::drawAsBoundingRectSk(SkCanvas *canvas,
                                       const SkPath &path,
                                       const SkScalar &invScale,
                                       const bool &dashes) {
    canvas->save();

    SkPaint paint;
    if(dashes) {
        SkScalar intervals[2] = {MIN_WIDGET_HEIGHT*0.25f*invScale,
                                 MIN_WIDGET_HEIGHT*0.25f*invScale};
        paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
    }
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1.5f*invScale);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    SkPath mappedPath = path;
    mappedPath.transform(QMatrixToSkMatrix(getCombinedTransform()));
    canvas->drawPath(mappedPath, paint);
    paint.setStrokeWidth(0.75f*invScale);
    paint.setColor(SK_ColorWHITE);
    canvas->drawPath(mappedPath, paint);

//    SkPaint paint;
//    SkScalar intervals[2] = {MIN_WIDGET_HEIGHT*0.25f*invScale,
//                             MIN_WIDGET_HEIGHT*0.25f*invScale};
//    paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
//    paint.setColor(SkColorSetARGB(125, 0, 0, 0));
//    paint.setStyle(SkPaint::kStroke_Style);
//    paint.setStrokeWidth(invScale);
//    SkPath mappedPath = path;
//    mappedPath.transform(QMatrixToSkMatrix(getCombinedTransform()));
//    canvas->drawPath(mappedPath, paint);

    canvas->restore();
}

void BoundingBox::drawBoundingRectSk(SkCanvas *canvas,
                                     const SkScalar &invScale) {
    drawAsBoundingRectSk(canvas, mSkRelBoundingRectPath,
                         invScale, true);
}

const SkPath &BoundingBox::getRelBoundingRectPath() {
    return mSkRelBoundingRectPath;
}

QMatrix BoundingBox::getCombinedTransform() const {
    return mTransformAnimator->getCombinedTransform();
}

QMatrix BoundingBox::getRelativeTransformAtCurrentFrame() {
    return getRelativeTransformAtRelFrame(anim_mCurrentRelFrame);
}

void BoundingBox::applyTransformation(BoxTransformAnimator *transAnimator) {
    Q_UNUSED(transAnimator);
}

void BoundingBox::applyTransformationInverted(BoxTransformAnimator *transAnimator) {
    Q_UNUSED(transAnimator);
}

void BoundingBox::scale(const qreal &scaleBy) {
    scale(scaleBy, scaleBy);
}

void BoundingBox::scale(const qreal &scaleXBy, const qreal &scaleYBy) {
    mTransformAnimator->scale(scaleXBy, scaleYBy);
}

NodePoint *BoundingBox::createNewPointOnLineNear(const QPointF &absPos,
                                                 const bool &adjust,
                                                 const qreal &canvasScaleInv) {
    Q_UNUSED(absPos);
    Q_UNUSED(adjust);
    Q_UNUSED(canvasScaleInv);
    return nullptr;
}

void BoundingBox::rotateBy(const qreal &rot) {
    mTransformAnimator->rotateRelativeToSavedValue(rot);
}

void BoundingBox::rotateRelativeToSavedPivot(const qreal &rot) {
    mTransformAnimator->rotateRelativeToSavedValue(rot,
                                                   mSavedTransformPivot);
}

void BoundingBox::scaleRelativeToSavedPivot(const qreal &scaleXBy,
                                            const qreal &scaleYBy) {
    mTransformAnimator->scaleRelativeToSavedValue(scaleXBy, scaleYBy,
                                                 mSavedTransformPivot);
}

void BoundingBox::scaleRelativeToSavedPivot(const qreal &scaleBy) {
    scaleRelativeToSavedPivot(scaleBy, scaleBy);
}

QPointF BoundingBox::mapRelPosToAbs(const QPointF &relPos) const {
    return mTransformAnimator->mapRelPosToAbs(relPos);
}

QRectF BoundingBox::getRelBoundingRect() const {
    return mRelBoundingRect;
}

QRectF BoundingBox::getRelBoundingRectAtRelFrame(const int &relFrame) {
    Q_UNUSED(relFrame);
    return getRelBoundingRect();
}

void BoundingBox::applyCurrentTransformation() {}

void BoundingBox::moveByAbs(const QPointF &trans) {
    mTransformAnimator->moveByAbs(trans);
    //    QPointF by = mParent->mapAbsPosToRel(trans) -
//                 mParent->mapAbsPosToRel(QPointF(0., 0.));
// //    QPointF by = mapAbsPosToRel(
// //                trans - mapRelativeToAbsolute(QPointF(0., 0.)));

//    moveByRel(by);
}

void BoundingBox::moveByRel(const QPointF &trans) {
    mTransformAnimator->moveRelativeToSavedValue(trans.x(), trans.y());
}

void BoundingBox::setAbsolutePos(const QPointF &pos) {
    setRelativePos(mParentTransform->mapAbsPosToRel(pos));
}

void BoundingBox::setRelativePos(const QPointF &relPos) {
    mTransformAnimator->setPosition(relPos.x(), relPos.y());
}

void BoundingBox::saveTransformPivotAbsPos(const QPointF &absPivot) {
    mSavedTransformPivot =
            mParentTransform->mapAbsPosToRel(absPivot) -
            mTransformAnimator->getPivot();
}

void BoundingBox::startPosTransform() {
    mTransformAnimator->startPosTransform();
}

void BoundingBox::startRotTransform() {
    mTransformAnimator->startRotTransform();
}

void BoundingBox::startScaleTransform() {
    mTransformAnimator->startScaleTransform();
}

void BoundingBox::startTransform() {
    mTransformAnimator->prp_startTransform();
}

void BoundingBox::finishTransform() {
    mTransformAnimator->prp_finishTransform();
    //updateCombinedTransform();
}

qreal BoundingBox::getEffectsMarginAtRelFrame(const int &relFrame) {
    return mEffectsAnimators->getEffectsMarginAtRelFrame(relFrame);
}

qreal BoundingBox::getEffectsMarginAtRelFrameF(const qreal &relFrame) {
    return mEffectsAnimators->getEffectsMarginAtRelFrameF(relFrame);
}

void BoundingBox::setupBoundingBoxRenderDataForRelFrameF(
                        const qreal &relFrame,
                        BoundingBoxRenderData* data) {
    data->fBoxStateId = mStateId;
    data->fRelFrame = qRound(relFrame);
    data->fRenderedToImage = false;
    data->fRelTransform = getRelativeTransformAtRelFrameF(relFrame);
    data->fParentTransform = mTransformAnimator->
            getParentCombinedTransformMatrixAtRelFrameF(relFrame);
    data->fTransform = data->fRelTransform*data->fParentTransform;
    data->fOpacity = mTransformAnimator->getOpacityAtRelFrameF(relFrame);
    data->fResolution = getParentCanvas()->getResolutionFraction();
    bool effectsVisible = getParentCanvas()->getRasterEffectsVisible();
    if(effectsVisible) {
        data->fEffectsMargin = getEffectsMarginAtRelFrameF(relFrame)*
                data->fResolution + 2.;
    } else {
        data->fEffectsMargin = 2.;
    }
    data->fBlendMode = getBlendMode();

    Canvas* parentCanvas = getParentCanvas();
    data->fMaxBoundsRect = parentCanvas->getMaxBoundsRect();
    if(data->fOpacity > 0.001 && effectsVisible) {
        setupEffectsF(relFrame, data);
        setupGPUEffectsF(relFrame, data);
    }
    if(data->fParentIsTarget && !data->nullifyBeforeProcessing()) {
        nullifyCurrentRenderData(data->fRelFrame);
    }
}

stdsptr<BoundingBoxRenderData> BoundingBox::createRenderData() { return nullptr; }

void BoundingBox::setupEffectsF(const qreal &relFrame,
                                BoundingBoxRenderData *data) {
    mEffectsAnimators->addEffectRenderDataToListF(relFrame, data);
}

void BoundingBox::setupGPUEffectsF(const qreal &relFrame,
                                   BoundingBoxRenderData *data) {
    mGPUEffectsAnimators->addEffectRenderDataToListF(relFrame, data);
}

void BoundingBox::addLinkingBox(BoundingBox *box) {
    mLinkingBoxes << box;
}

void BoundingBox::removeLinkingBox(BoundingBox *box) {
    mLinkingBoxes.removeOne(box);
}

const QList<qptr<BoundingBox>> &BoundingBox::getLinkingBoxes() const {
    return mLinkingBoxes;
}

EffectAnimators *BoundingBox::getEffectsAnimators() {
    return mEffectsAnimators.data();
}

void BoundingBox::incReasonsNotToApplyUglyTransform() {
    mNReasonsNotToApplyUglyTransform++;
}

void BoundingBox::decReasonsNotToApplyUglyTransform() {
    mNReasonsNotToApplyUglyTransform--;
}

bool BoundingBox::isSelected() const { return mSelected; }

bool BoundingBox::relPointInsidePath(const QPointF &relPos) const {
    return mRelBoundingRect.contains(relPos.toPoint());
}

bool BoundingBox::absPointInsidePath(const QPointF &absPoint) {
    return relPointInsidePath(mapAbsPosToRel(absPoint));
}

MovablePoint *BoundingBox::getPointAtAbsPos(const QPointF &absPtPos,
                                            const CanvasMode &currentCanvasMode,
                                            const qreal &canvasScaleInv) {
    if(currentCanvasMode == MOVE_PATH) {
        MovablePoint* pivotMovable = mTransformAnimator->getPivotMovablePoint();
        if(pivotMovable->isPointAtAbsPos(absPtPos, canvasScaleInv)) {
            return pivotMovable;
        }
    }
    return nullptr;
}

void BoundingBox::cancelTransform() {
    mTransformAnimator->prp_cancelTransform();
    //updateCombinedTransform();
}

void BoundingBox::moveUp() {
    mParentGroup->increaseContainedBoxZInList(this);
}

void BoundingBox::moveDown() {
    mParentGroup->decreaseContainedBoxZInList(this);
}

void BoundingBox::bringToFront() {
    mParentGroup->bringContainedBoxToEndList(this);
}

void BoundingBox::bringToEnd() {
    mParentGroup->bringContainedBoxToFrontList(this);
}

void BoundingBox::setZListIndex(const int &z) {
    mZListIndex = z;
}

void BoundingBox::selectAndAddContainedPointsToList(
        const QRectF &, QList<stdptr<MovablePoint>> &) {}

int BoundingBox::getZIndex() const {
    return mZListIndex;
}

QPointF BoundingBox::getAbsolutePos() const {
    return QPointF(mTransformAnimator->getCombinedTransform().dx(),
                   mTransformAnimator->getCombinedTransform().dy());
}

void BoundingBox::updateDrawRenderContainerTransform() {
    if(mNReasonsNotToApplyUglyTransform == 0) {
        mDrawRenderContainer->updatePaintTransformGivenNewCombinedTransform(
                    mTransformAnimator->getCombinedTransform());
    }

}

void BoundingBox::setPivotAutoAdjust(const bool &bT) {
    mPivotAutoAdjust = bT;
}

const BoundingBoxType &BoundingBox::getBoxType() const { return mType; }

void BoundingBox::startDurationRectPosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->startPosTransform();
    }
}

void BoundingBox::finishDurationRectPosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->finishPosTransform();
    }
}

void BoundingBox::moveDurationRect(const int &dFrame) {
    if(hasDurationRectangle()) {
        mDurationRectangle->changeFramePosBy(dFrame);
    }
}

void BoundingBox::startMinFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->startMinFramePosTransform();
    }
}

void BoundingBox::finishMinFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->finishMinFramePosTransform();
    }
}

void BoundingBox::moveMinFrame(const int &dFrame) {
    if(hasDurationRectangle()) {
        mDurationRectangle->moveMinFrame(dFrame);
    }
}

void BoundingBox::startMaxFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->startMaxFramePosTransform();
    }
}

void BoundingBox::finishMaxFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->finishMaxFramePosTransform();
    }
}

void BoundingBox::moveMaxFrame(const int &dFrame) {
    if(hasDurationRectangle()) {
        mDurationRectangle->moveMaxFrame(dFrame);
    }
}

DurationRectangle *BoundingBox::getDurationRectangle() {
    return mDurationRectangle.get();
}

void BoundingBox::requestGlobalFillStrokeUpdateIfSelected() {
    if(isSelected()) {
        emit fillStrokeSettingsChanged();
    }
}

void BoundingBox::requestGlobalPivotUpdateIfSelected() {
    if(isSelected()) {
        emit globalPivotInfluenced();
    }
}

void BoundingBox::getMotionBlurProperties(QList<Property*> &list) const {
    list.append(mTransformAnimator->getScaleAnimator());
    list.append(mTransformAnimator->getPosAnimator());
    list.append(mTransformAnimator->getPivotAnimator());
    list.append(mTransformAnimator->getRotAnimator());
}

BoxTransformAnimator *BoundingBox::getTransformAnimator() {
    return mTransformAnimator.get();
}

VectorPath *BoundingBox::objectToVectorPathBox() { return nullptr; }

VectorPath *BoundingBox::strokeToVectorPathBox() { return nullptr; }

void BoundingBox::selectionChangeTriggered(const bool &shiftPressed) {
    Canvas* parentCanvas = getParentCanvas();
    if(shiftPressed) {
        if(mSelected) {
            parentCanvas->removeBoxFromSelection(this);
        } else {
            parentCanvas->addBoxToSelection(this);
        }
    } else {
        parentCanvas->clearBoxesSelection();
        parentCanvas->addBoxToSelection(this);
    }
}

bool BoundingBox::isAnimated() const {
    return prp_isDescendantRecording();
}

void BoundingBox::addGPUEffect(const qsptr<GPURasterEffect>& rasterEffect) {
    if(mGPUEffectsAnimators->ca_getNumberOfChildren() == 0) {
        mGPUEffectsAnimators->SWT_show();
    }
    mGPUEffectsAnimators->ca_addChildAnimator(rasterEffect);
    //effect->setParentEffectAnimators(mGPUEffectsAnimators.data());

    clearAllCache();
}

void BoundingBox::addEffect(const qsptr<PixmapEffect>& effect) {
    //effect->setUpdater(SPtrCreate(PixmapEffectUpdater)(this));

    if(!mEffectsAnimators->hasChildAnimators()) {
        mEffectsAnimators->SWT_show();
    }
    mEffectsAnimators->ca_addChildAnimator(effect);
    effect->setParentEffectAnimators(mEffectsAnimators.data());

    clearAllCache();
}

void BoundingBox::removeEffect(const qsptr<PixmapEffect>& effect) {
    mEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mEffectsAnimators->hasChildAnimators()) {
        mEffectsAnimators->SWT_hide();
    }

    clearAllCache();
}

//int BoundingBox::prp_getParentFrameShift() const {
//    if(!mParentGroup) {
//        return 0;
//    } else {
//        return mParentGroup->prp_getFrameShift();
//    }
//}

bool BoundingBox::hasDurationRectangle() const {
    return mDurationRectangle != nullptr;
}

void BoundingBox::createDurationRectangle() {
    qsptr<DurationRectangle> durRect =
            SPtrCreate(DurationRectangle)(this);
    durRect->setMinFrame(0);
    Canvas* parentCanvas = getParentCanvas();
    if(parentCanvas) {
        durRect->setFramesDuration(getParentCanvas()->getFrameCount());
    }
    setDurationRectangle(durRect);
}

void BoundingBox::shiftAll(const int &shift) {
    if(hasDurationRectangle()) {
        mDurationRectangle->changeFramePosBy(shift);
    } else {
        anim_shiftAllKeys(shift);
    }
}

QMatrix BoundingBox::getRelativeTransformAtRelFrame(const int &relFrame) {
    return mTransformAnimator->getRelativeTransformAtRelFrame(relFrame);
}

QMatrix BoundingBox::getRelativeTransformAtRelFrameF(const qreal &relFrame) {
    return mTransformAnimator->getRelativeTransformAtRelFrameF(relFrame);
}

int BoundingBox::prp_getRelFrameShift() const {
    if(!mDurationRectangle) return 0;
    return mDurationRectangle->getFrameShift();
}

void BoundingBox::setDurationRectangle(
        const qsptr<DurationRectangle>& durationRect) {
    if(durationRect == mDurationRectangle) return;
    if(mDurationRectangle) {
        disconnect(mDurationRectangle.data(), nullptr, this, nullptr);
    }
    qsptr<DurationRectangle> oldDurRect = mDurationRectangle;
    mDurationRectangle = durationRect;
    updateAfterDurationRectangleShifted();
    if(!mDurationRectangle) {
        shiftAll(oldDurRect->getFrameShift());
        return;
    }

    connect(mDurationRectangle.data(), &DurationRectangle::posChangedBy,
            this, &BoundingBox::updateAfterDurationRectangleShifted);
    connect(mDurationRectangle.data(), &DurationRectangle::rangeChanged,
            this, &BoundingBox::updateAfterDurationRectangleRangeChanged);

    connect(mDurationRectangle.data(), &DurationRectangle::minFrameChangedBy,
            this, &BoundingBox::updateAfterDurationMinFrameChangedBy);
    connect(mDurationRectangle.data(), &DurationRectangle::maxFrameChangedBy,
            this, &BoundingBox::updateAfterDurationMaxFrameChangedBy);
}

void BoundingBox::updateAfterDurationRectangleShifted(const int &dFrame) {
    prp_setParentFrameShift(prp_mParentFrameShift);
    prp_setAbsFrame(anim_mCurrentAbsFrame);
    auto visRange = getVisibleAbsFrameRange();
    if(dFrame > 0) {
        visRange.fMin -= dFrame;
    } else {
        visRange.fMax -= dFrame;
    }
    prp_updateAfterChangedAbsFrameRange(visRange);
}

void BoundingBox::updateAfterDurationMinFrameChangedBy(const int &by) {
    auto visRange = getVisibleAbsFrameRange();
    if(by > 0) {
        visRange.fMin -= by;
        visRange.fMax = visRange.fMin - 1;
    } else {
        visRange.fMax = visRange.fMin - by - 1;
    }
    prp_updateAfterChangedAbsFrameRange(visRange);
}

void BoundingBox::updateAfterDurationMaxFrameChangedBy(const int &by) {
    auto visRange = getVisibleAbsFrameRange();

    if(by > 0) {
        visRange.fMin = visRange.fMax - by + 1;
    } else {
        visRange.fMin = visRange.fMax + 1;
        visRange.fMax -= by;
    }
    prp_updateAfterChangedAbsFrameRange(visRange);
}

void BoundingBox::updateAfterDurationRectangleRangeChanged() {}

DurationRectangleMovable *BoundingBox::anim_getRectangleMovableAtPos(
        const int &relX, const int &minViewedFrame,
        const qreal &pixelsPerFrame) {
    if(!mDurationRectangle) return nullptr;
    return mDurationRectangle->getMovableAt(relX, pixelsPerFrame,
                                            minViewedFrame);
}

void BoundingBox::prp_drawKeys(QPainter *p,
                               const qreal &pixelsPerFrame,
                               const qreal &drawY,
                               const int &startFrame,
                               const int &endFrame,
                               const int &rowHeight,
                               const int &keyRectSize) {
    if(mDurationRectangle) {
        p->save();
        p->translate(prp_getParentFrameShift()*pixelsPerFrame, 0);
        QRect drawRect(0, drawY, (endFrame - startFrame)*pixelsPerFrame,
                       rowHeight);
        mDurationRectangle->draw(p, drawRect, pixelsPerFrame,
                                 startFrame, endFrame);
        p->restore();
    }

    Animator::prp_drawKeys(p, pixelsPerFrame, drawY,
                           startFrame, endFrame,
                           rowHeight, keyRectSize);
}

void BoundingBox::addPathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::addFillPathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::addOutlinePathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::removePathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::removeFillPathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::removeOutlinePathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::addActionsToMenu(QMenu *) {}

bool BoundingBox::handleSelectedCanvasAction(QAction *, QWidget *) {
    return false;
}

void BoundingBox::setName(const QString &name) {
    if(name == prp_mName) return;
    prp_mName = name;

    emit nameChanged(name);
}

const QString &BoundingBox::getName() const {
    return prp_mName;
}

bool BoundingBox::isVisibleAndInVisibleDurationRect() const {
    return isRelFrameInVisibleDurationRect(anim_mCurrentRelFrame) && mVisible;
}

bool BoundingBox::isRelFrameInVisibleDurationRect(const int &relFrame) const {
    if(!mDurationRectangle) return true;
    return relFrame <= mDurationRectangle->getMaxFrameAsRelFrame() &&
           relFrame >= mDurationRectangle->getMinFrameAsRelFrame();
}

bool BoundingBox::isRelFrameFInVisibleDurationRect(const qreal &relFrame) const {
    if(!mDurationRectangle) return true;
    return relFrame <= mDurationRectangle->getMaxFrameAsRelFrame() &&
           relFrame >= mDurationRectangle->getMinFrameAsRelFrame();
}

bool BoundingBox::isRelFrameVisibleAndInVisibleDurationRect(
        const int &relFrame) const {
    return isRelFrameInVisibleDurationRect(relFrame) && mVisible;
}

bool BoundingBox::isRelFrameFVisibleAndInVisibleDurationRect(
        const qreal &relFrame) const {
    return isRelFrameFInVisibleDurationRect(relFrame) && mVisible;
}

FrameRange BoundingBox::prp_getIdenticalRelFrameRange(const int &relFrame) const {
    if(mVisible) {
        if(isRelFrameInVisibleDurationRect(relFrame)) {
            return ComplexAnimator::prp_getIdenticalRelFrameRange(relFrame);
        }
        if(relFrame > mDurationRectangle->getMaxFrameAsRelFrame()) {
            return {mDurationRectangle->getMaxFrameAsRelFrame() + 1,
                        FrameRange::EMAX};
        } else if(relFrame < mDurationRectangle->getMinFrameAsRelFrame()) {
            return {FrameRange::EMIN,
                    mDurationRectangle->getMinFrameAsRelFrame() - 1};
        }
    }
    return {FrameRange::EMIN, FrameRange::EMAX};
}


FrameRange BoundingBox::getFirstAndLastIdenticalForMotionBlur(
        const int &relFrame, const bool &takeAncestorsIntoAccount) {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    if(mVisible) {
        if(isRelFrameInVisibleDurationRect(relFrame)) {
            QList<Property*> propertiesT;
            getMotionBlurProperties(propertiesT);
            for(const auto& child : propertiesT) {
                if(range.isUnary()) break;
                auto childRange = child->prp_getIdenticalRelFrameRange(relFrame);
                range *= childRange;
            }

            range *= mDurationRectangle->getRelFrameRange();
        } else {
            if(relFrame > mDurationRectangle->getMaxFrameAsRelFrame()) {
                return mDurationRectangle->getAbsFrameRangeToTheRight();
            } else if(relFrame < mDurationRectangle->getMinFrameAsRelFrame()) {
                return mDurationRectangle->getAbsFrameRangeToTheLeft();
            }
        }
    } else {
        return {FrameRange::EMIN, FrameRange::EMAX};
    }
    if(mParentGroup == nullptr || takeAncestorsIntoAccount) return range;
    if(range.isUnary()) return range;
    int parentRel = mParentGroup->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame));
    auto parentRange = mParentGroup->BoundingBox::getFirstAndLastIdenticalForMotionBlur(parentRel);

    return range*parentRange;
}

void BoundingBox::scheduleWaitingTasks() {
    for(const auto &task : mScheduledTasks) {
        task->taskQued();
    }
}

void BoundingBox::queScheduledTasks() {
    for(const auto &task : mScheduledTasks) {
        TaskScheduler::sGetInstance()->queCPUTask(task);
    }

    mScheduledTasks.clear();
}

const int &BoundingBox::getLoadId() const {
    return mLoadId;
}

int BoundingBox::setBoxLoadId(const int &loadId) {
    mLoadId = loadId;
    return loadId + 1;
}

void BoundingBox::clearBoxLoadId() {
    mLoadId = -1;
}

BoundingBox *BoundingBox::getLoadedBoxById(const int &loadId) {
    for(const auto& box : mLoadedBoxes) {
        if(!box) return nullptr;
        if(box->getLoadId() == loadId) {
            return box;
        }
    }
    return nullptr;
}

void BoundingBox::addFunctionWaitingForBoxLoad(
        const stdsptr<FunctionWaitingForBoxLoad> &func) {
    mFunctionsWaitingForBoxLoad << func;
}

void BoundingBox::addLoadedBox(BoundingBox *box) {
    mLoadedBoxes << box;
    for(int i = 0; i < mFunctionsWaitingForBoxLoad.count(); i++) {
        auto funcT = mFunctionsWaitingForBoxLoad.at(i);
        if(funcT->loadBoxId == box->getLoadId()) {
            funcT->boxLoaded(box);
            mFunctionsWaitingForBoxLoad.removeAt(i);
            i--;
        }
    }
}

int BoundingBox::getLoadedBoxesCount() {
    return mLoadedBoxes.count();
}

void BoundingBox::clearLoadedBoxes() {
    for(const auto& box : mLoadedBoxes) {
        box->clearBoxLoadId();
    }
    mLoadedBoxes.clear();
    mFunctionsWaitingForBoxLoad.clear();
}

void BoundingBox::selectAllPoints(Canvas *canvas) {
    Q_UNUSED(canvas);
}

void BoundingBox::scheduleTask(const stdsptr<_ScheduledTask>& task) {
    mScheduledTasks << task;
}

void BoundingBox::setVisibile(const bool &visible) {
    if(mVisible == visible) return;
    if(mSelected) {
        removeFromSelection();
    }
    mVisible = visible;

    clearAllCache();

    scheduleUpdate(Animator::USER_CHANGE);

    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Visible);
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Invisible);
    for(BoundingBox* box : mLinkingBoxes) {
        if(box->isParentLinkBox()) {
            box->setVisibile(visible);
        }
    }
}

void BoundingBox::switchVisible() {
    setVisibile(!mVisible);
}

bool BoundingBox::isParentLinkBox() {
    return mParentGroup->SWT_isLinkBox();
}

void BoundingBox::switchLocked() {
    setLocked(!mLocked);
}

void BoundingBox::hide() {
    setVisibile(false);
}

void BoundingBox::show() {
    setVisibile(true);
}

bool BoundingBox::isVisibleAndUnlocked() const {
    return isVisible() && !mLocked;
}

bool BoundingBox::isVisible() const {
    return mVisible;
}

bool BoundingBox::isLocked() const {
    return mLocked;
}

void BoundingBox::lock() {
    setLocked(true);
}

void BoundingBox::unlock() {
    setLocked(false);
}

void BoundingBox::setLocked(const bool &bt) {
    if(bt == mLocked) return;
    if(mSelected) {
        getParentCanvas()->removeBoxFromSelection(this);
    }
    mLocked = bt;
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Locked);
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Unlocked);
}

bool BoundingBox::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                      const bool &parentSatisfies,
                                      const bool &parentMainTarget) const {
    const SWT_Rule &rule = rules.rule;
    bool satisfies = false;
    bool alwaysShowChildren = rules.alwaysShowChildren;
    if(rules.type == &SingleWidgetTarget::SWT_isSingleSound) return false;
    if(alwaysShowChildren) {
        if(rule == SWT_NoRule) {
            satisfies = parentSatisfies;
        } else if(rule == SWT_Selected) {
            satisfies = isSelected() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_Animated) {
            satisfies = isAnimated() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_NotAnimated) {
            satisfies = !isAnimated() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_Visible) {
            satisfies = isVisible() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_Invisible) {
            satisfies = !isVisible() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_Locked) {
            satisfies = isLocked() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_Unlocked) {
            satisfies = !isLocked() ||
                    (parentSatisfies && !parentMainTarget);
        }
    } else {
        if(rule == SWT_NoRule) {
            satisfies = parentSatisfies;
        } else if(rule == SWT_Selected) {
            satisfies = isSelected();
        } else if(rule == SWT_Animated) {
            satisfies = isAnimated();
        } else if(rule == SWT_NotAnimated) {
            satisfies = !isAnimated();
        } else if(rule == SWT_Visible) {
            satisfies = isVisible() && parentSatisfies;
        } else if(rule == SWT_Invisible) {
            satisfies = !isVisible() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_Locked) {
            satisfies = isLocked() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_Unlocked) {
            satisfies = !isLocked() && parentSatisfies;
        }
    }
    if(satisfies) {
        const QString &nameSearch = rules.searchString;
        if(!nameSearch.isEmpty()) {
            satisfies = prp_mName.contains(nameSearch);
        }
    }
    return satisfies;
}

bool BoundingBox::SWT_visibleOnlyIfParentDescendant() const {
    return false;
}

void BoundingBox::SWT_addToContextMenu(QMenu *menu) {
    menu->addAction("Apply Transformation");
    menu->addAction("Create Link");
    menu->addAction("Center Pivot");
    menu->addAction("Copy");
    menu->addAction("Cut");
    menu->addAction("Duplicate");
    menu->addAction("Group");
    menu->addAction("Ungroup");
    menu->addAction("Delete");

    QMenu *effectsMenu = menu->addMenu("Effects");
    effectsMenu->addAction("Blur");
    effectsMenu->addAction("Shadow");
//            effectsMenu->addAction("Brush");
    effectsMenu->addAction("Lines");
    effectsMenu->addAction("Circles");
    effectsMenu->addAction("Swirl");
    effectsMenu->addAction("Oil");
    effectsMenu->addAction("Implode");
    effectsMenu->addAction("Desaturate");
}

void BoundingBox::removeFromParent_k() {
    if(!mParentGroup) return;
    mParentGroup->removeContainedBox_k(ref<BoundingBox>());
}

void BoundingBox::removeFromSelection() {
    if(mSelected) {
        Canvas* parentCanvas = getParentCanvas();
        parentCanvas->removeBoxFromSelection(this);
    }
}

bool BoundingBox::SWT_handleContextMenuActionSelected(
        QAction *selectedAction) {
    if(selectedAction) {
        if(selectedAction->text() == "Delete") {
            removeFromParent_k();
        } else if(selectedAction->text() == "Apply Transformation") {
            applyCurrentTransformation();
        } else if(selectedAction->text() == "Create Link") {
            mParentGroup->addContainedBox(createLink());
        } else if(selectedAction->text() == "Group") {
            getParentCanvas()->groupSelectedBoxes();
            return true;
//        } else if(selectedAction->text() == "Ungroup") {
//            ungroupSelected();
//        } else if(selectedAction->text() == "Center Pivot") {
//            mCurrentBoxesGroup->centerPivotForSelected();
//        } else if(selectedAction->text() == "Blur") {
//            mCurrentBoxesGroup->applyBlurToSelected();
//        } else if(selectedAction->text() == "Shadow") {
//            mCurrentBoxesGroup->applyShadowToSelected();
//        } else if(selectedAction->text() == "Brush") {
//            mCurrentBoxesGroup->applyBrushEffectToSelected();
//        } else if(selectedAction->text() == "Lines") {
//            mCurrentBoxesGroup->applyLinesEffectToSelected();
//        } else if(selectedAction->text() == "Circles") {
//            mCurrentBoxesGroup->applyCirclesEffectToSelected();
//        } else if(selectedAction->text() == "Swirl") {
//            mCurrentBoxesGroup->applySwirlEffectToSelected();
//        } else if(selectedAction->text() == "Oil") {
//            mCurrentBoxesGroup->applyOilEffectToSelected();
//        } else if(selectedAction->text() == "Implode") {
//            mCurrentBoxesGroup->applyImplodeEffectToSelected();
//        } else if(selectedAction->text() == "Desaturate") {
//            mCurrentBoxesGroup->applyDesaturateEffectToSelected();
        }
    } else {

    }
    return false;
}

QMimeData *BoundingBox::SWT_createMimeData() {
    return new BoundingBoxMimeData(this);
}

void BoundingBox::renderDataFinished(BoundingBoxRenderData *renderData) {
    if(renderData->fRedo) {
        scheduleUpdate(renderData->fRelFrame, Animator::USER_CHANGE);
    }
    auto currentRenderData = mDrawRenderContainer->getSrcRenderData();
    bool newerSate = true;
    bool closerFrame = true;
    if(currentRenderData) {
        newerSate = currentRenderData->fBoxStateId <
                renderData->fBoxStateId;
        const int finishedFrameDist =
                qAbs(anim_mCurrentRelFrame - renderData->fRelFrame);
        const int oldFrameDist =
                qAbs(anim_mCurrentRelFrame - currentRenderData->fRelFrame);
        closerFrame = finishedFrameDist < oldFrameDist;
    }
    if(newerSate || closerFrame) {
        mDrawRenderContainer->setSrcRenderData(renderData);
        const bool currentState =
                renderData->fBoxStateId == mStateId;
        const bool currentFrame =
                renderData->fRelFrame == anim_mCurrentRelFrame;
        mDrawRenderContainer->setExpired(!currentState || !currentFrame);
    }
}

FrameRange BoundingBox::getVisibleAbsFrameRange() const {
    if(!mDurationRectangle) return {FrameRange::EMIN, FrameRange::EMAX};
    return mDurationRectangle->getAbsFrameRange();
}

FunctionWaitingForBoxLoad::FunctionWaitingForBoxLoad(const int &boxIdT) {
    loadBoxId = boxIdT;
}
